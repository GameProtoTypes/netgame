
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <variant>
#include <thread>
#include <chrono>
#include <filesystem>
#include <format>

#include "cista/serialization.h"
#include "cista/containers/vector.h"
#include "cista/reflection/to_tuple.h"


 
#include "GameCompute.h"
#include "GameNetworking.h"
#include "GameGraphics.h"

#include "implot.h"

import Game;
namespace Game {

void game_init_single(ALL_CORE_PARAMS);
void game_init_multi(ALL_CORE_PARAMS);
void game_init_multi2(ALL_CORE_PARAMS);
void game_init_single2(ALL_CORE_PARAMS);


void game_apply_actions(ALL_CORE_PARAMS);
void game_hover_gui(ALL_CORE_PARAMS);
void game_preupdate_1(ALL_CORE_PARAMS);
void game_preupdate_2(ALL_CORE_PARAMS);
void game_updatepre1(ALL_CORE_PARAMS);
void game_update(ALL_CORE_PARAMS);
void game_update2(ALL_CORE_PARAMS);
void game_post_update(ALL_CORE_PARAMS);

void dumbfunc(ALL_CORE_PARAMS_TYPES
){}
}





using namespace Game;



GameCompute::GameCompute()
{
}

GameCompute::~GameCompute()
{
}

void GameCompute::RunGameKernel(std::function<void(ALL_CORE_PARAMS_TYPES)> kernelFunc, 
    int funcId,
    int numThreads)
{

    //run game tick
    std::vector<std::thread> threads;
    std::vector<std::chrono::steady_clock::time_point> startTimes;
    std::vector<std::chrono::steady_clock::time_point> endTimes;

    Game::StaticData data;
    for(int t = 0; t < numThreads; t++)
    {
        startTimes.push_back(std::chrono::steady_clock::now());

        threads.push_back(std::thread(kernelFunc,
        &data,
        gameState.get(),
        gameStateActions.get(),
        &gameState.get()->guiStyle,
        &peepVBOBuffer[0],
        &particleVBOBuffer[0],
        &mapTile1VBO[0],
        &mapTile1AttrVBO[0],
        &mapTile1OtherAttrVBO[0],
        &mapTile2VBO[0],
        &mapTile2AttrVBO[0],
        &mapTile2OtherAttrVBO[0],
        &guiVBO[0],
        &linesVBO[0],
        t, 
        numThreads
        ));

    }





    for(int t=0; t < numThreads; t++)
    {
        threads[t].join();
        endTimes.push_back(std::chrono::steady_clock::now());
        profileTimes[funcId][t][gameStateActions->tickIdx%500] = (endTimes[t] - startTimes[t]).count()/1000000.0f;
    }

}

void GameCompute::ImGuiProfiler()
{
    if (ImPlot::BeginPlot("Times")) {

        ImPlot::SetupAxes("Tick", "Time");
        ImPlot::SetupAxesLimits(0, 500, 0, networking->targetTickTimeMs * 2);


        ImPlot::PlotLine("actionEvent",     profileTimes[4][0], 500);
        ImPlot::PlotLine("guiEvent",        profileTimes[5][0], 500);
        ImPlot::PlotLine("preUpdateEvent1", profileTimes[6][0], 500);
        ImPlot::PlotLine("preUpdateEvent2", profileTimes[7][0], 500);
        ImPlot::PlotLine("updatepre1Event", profileTimes[8][0], 500);
        ImPlot::PlotLine("updateEvent",     profileTimes[9][0], 500);
        ImPlot::PlotLine("update2Event",    profileTimes[10][0], 500);
        ImPlot::PlotLine("postupdateEvent", profileTimes[11][0], 500);
        ImPlot::PlotLine("FrameTime", frameTimes, 500);


        ImPlot::PlotInfLines("Limit", &(networking->targetTickTimeMs), 1, ImPlotInfLinesFlags_Horizontal);
        int d = gameStateActions->tickIdx % 501;
        ImPlot::PlotInfLines("Tick", &d, 1);


        ImPlot::EndPlot();
    }
    auto now = std::chrono::steady_clock::now();
    frameTimes[gameStateActions->tickIdx % 500] = (now - lastFrameTimePoint).count() / 1000000.0f;
    lastFrameTimePoint = now;
}

void GameCompute::GameInit()
{
    std::function<void(ALL_CORE_PARAMS_TYPES)> kernel = Game::game_init_single;
    RunGameKernel(kernel,0, 1);


    kernel = game_init_multi;
    RunGameKernel(kernel, 1);

    kernel = game_init_multi2;
    RunGameKernel(kernel,2);

    
    kernel = game_init_single2;
    RunGameKernel(kernel,3,1);

}

void GameCompute::Stage1_Begin()
{
    if (gameStateActions->pauseState != 0)
        return;




    //run game tick
    std::function<void(ALL_CORE_PARAMS_TYPES)> kernel = Game::game_apply_actions;
    RunGameKernel(kernel,4, 1);

    kernel = game_hover_gui;
    RunGameKernel(kernel,5, 1);

    kernel = game_preupdate_1;
    RunGameKernel(kernel, 6);

    kernel = game_preupdate_2;
    RunGameKernel(kernel, 7);

    kernel = game_updatepre1;
    RunGameKernel(kernel,8 );

    kernel = game_update;
    RunGameKernel(kernel,9);

    kernel = game_update2;
    RunGameKernel(kernel,10);

    kernel = game_post_update;
    RunGameKernel(kernel,11);


    stage1_Running = true;
}
void GameCompute::Stage1_End()
{
    if(!stage1_Running)
        return;

    //end game tick
    gameStateActions->tickIdx++;
    stage1_Running = false;
}


void GameCompute::SaveGameStateBase()
{
    std::ofstream myfile;
    myfile.open("gamestatebase.bin", std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
    if (!myfile.is_open())
        std::cout << "Error Saving File!" << std::endl;
    else
    {
        

        myfile.write(reinterpret_cast<char*>(gameState.get()), sizeof(GameState));
    }
    
    myfile.close();
}


std::string GameCompute::GameStateString(int tickIdx)
{
    return std::format("gamestate_tick_{}_.bin",gameStateActions->tickIdx);
}
void GameCompute::SaveGameStateDiff(std::vector<char>* data, bool deleteFile)
{
    std::ofstream myfile;

    myfile.open(GameStateString(gameStateActions->tickIdx), std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
    if (!myfile.is_open())
        std::cout << "Error Saving File!" << std::endl;
    else
    {
        myfile.write(reinterpret_cast<char*>(gameState.get()), sizeof(GameState));
    }
    
    myfile.close();
    std::string diffFileName = std::format("diff_tick_{}.diff", gameStateActions->tickIdx);
    std::string command = std::format(".\\windows_binaries\\hdiffz.exe -f -s-512 gamestatebase.bin {} {}", GameStateString(gameStateActions->tickIdx), diffFileName);
    std::system(command.c_str());


    std::filesystem::remove(GameStateString(gameStateActions->tickIdx));


    std::ifstream diffFile;
    diffFile.open(diffFileName, std::ifstream::binary);
    diffFile.seekg(0, diffFile.end);
    unsigned int fileSize = (unsigned int)diffFile.tellg();
    diffFile.seekg(0, diffFile.beg);
    
    data->resize(fileSize);
    diffFile.read(data->data(),fileSize);
    diffFile.close();
    std::filesystem::remove(diffFileName);


}


void GameCompute::LoadGameStateFromDiff(std::string diffFileName, std::string resultGameStateFileName)
{

    std::string command = std::format(".\\windows_binaries\\hpatchz.exe -f gamestatebase.bin {} {}", diffFileName, resultGameStateFileName);
    std::system(command.c_str());

    std::ifstream myfile;
    myfile.open(resultGameStateFileName, std::ifstream::binary);
    if (!myfile.is_open())
        std::cout << "Error Reading File!" << std::endl;
    else {
        
        std::cout << "Reading " << sizeof(GameState) << " bytes" << std::endl;
        myfile.read(reinterpret_cast<char*>(gameState.get()), sizeof(GameState));
    
        if (myfile)
            std::cout << "all characters read successfully.";
        else
            std::cout << "error: only " << myfile.gcount() << " could be read";
    
    }
    myfile.close();

    std::filesystem::remove(resultGameStateFileName);

}
void GameCompute::LoadGameStateFromDiff(int tickidx)
{
    LoadGameStateFromDiff(std::format("diff_tick_{}.diff", tickidx), GameStateString(tickidx));
}

void GameCompute::LoadGameStateFromDiff(std::vector<char>* diffdata, int id)
{
    std::ofstream diffFile;

    std::string diffFileName = std::format("netgame_{}.gamediff", id);
    diffFile.open(diffFileName, std::ofstream::binary | std::ofstream::trunc | std::ofstream::ate);
    diffFile.write(diffdata->data(), diffdata->size());
    diffFile.close();

    LoadGameStateFromDiff(diffFileName, std::format("netgame_{}.bin", id));

    std::filesystem::remove(diffFileName);



}


