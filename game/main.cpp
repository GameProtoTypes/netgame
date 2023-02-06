
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>


#include "glew.h"
#include "glfw3native.h"

#include <SDL.h>
#include "SDL_opengl.h"

#include "implot.h"

#include <angelscript.h>
#include <scriptstdstring/scriptstdstring.h>
#include <scriptbuilder/scriptbuilder.h>

#include "glm.hpp"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform2.hpp>
#include <gtx/string_cast.hpp>
#include "glmHelpers.h"


#include "GameNetworking.h"
#include "GameGraphics.h"
#include "GameCompute.h"


#include "GEShader.h"
#include "GEShaderProgram.h"

import GE.Basic;
import GE.ImGui;
import Game;


using namespace GE;

void ActionTrackingInit(ActionTracking* actionTracking)
{
    actionTracking->clientId = 0;
    actionTracking->ticksLate = 0;
    actionTracking->clientApplied = false;
}

void ActionWrapInit(ActionWrap* actionWrap)
{
    ActionTrackingInit(&actionWrap->tracking);
}





//#define GSCSTESTS
#ifdef GSCSTESTS
#define GSCS(ID) { std::cout << #ID << ": GSCS: " << gameNetworking.CheckSumGameState(gameState.get()) << " TickIdx: " << gameState.get()->tickIdx << std::endl;}
#else
#define GSCS(ID) {}
#endif

int32_t random(int32_t min, int32_t max) { return rand() % (max - min + 1) + min; }

void WaitTickTime(uint64_t timerStartMs, float targetTimeMs, float* frameTimeMS)
{
    *frameTimeMS = SDL_GetTicks64() - timerStartMs;
    float sleepTime = glm::clamp((targetTimeMs - *frameTimeMS), 0.0f, targetTimeMs);
    std::this_thread::sleep_for(std::chrono::microseconds((long long)(sleepTime*1000.0f)));
}

const char* AngelScriptPrintPrefix = "ANGEL SCRIPT: ";


// Implement a simple message callback function
void MessageCallback(const asSMessageInfo *msg, void *param)
{
  const char *type = "ERR ";
  if( msg->type == asMSGTYPE_WARNING ) 
    type = "WARN";
  else if( msg->type == asMSGTYPE_INFORMATION ) 
    type = "INFO";
  printf("%s %s (%d, %d) : %s : %s\n", AngelScriptPrintPrefix, msg->section, msg->row, msg->col, type, msg->message);
}
 




// Print the script string to the standard output stream
void print(std::string &msg)
{
  printf("AS: %s", msg.c_str());
}




int32_t main(int32_t argc, char* args[]) 
{


    asIScriptEngine *engine = asCreateScriptEngine();
    int r = engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL); assert( r >= 0 );
    std::cout << "AS ENUM COUNT: " << engine->GetEnumCount();
    RegisterStdString(engine);
    r = engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_CDECL); assert( r >= 0 );


    // The CScriptBuilder helper is an add-on that loads the file,
    // performs a pre-processing pass if necessary, and then tells
    // the engine to build a script module.
    CScriptBuilder builder;
    r = builder.StartNewModule(engine, "MainGameModule"); 
    if( r < 0 ) 
    {
        // If the code fails here it is usually because there
        // is no more memory to allocate the module
        printf("%s Unrecoverable error while starting a new module.\n", AngelScriptPrintPrefix);
        return 1;
    }
    r = builder.AddSectionFromFile("angelScript/game.as");
    if( r < 0 )
    {
        // The builder wasn't able to load the file. Maybe the file
        // has been removed, or the wrong name was given, or some
        // preprocessing commands are incorrectly written.
        printf("%s Please correct the errors in the script and restart the game\n", AngelScriptPrintPrefix);
        return 1;
    }
    r = builder.BuildModule();
    if( r < 0 )
    {
        // An error occurred. Instruct the script writer to fix the 
        // compilation errors that were listed in the output stream.
        printf("%s Please correct the errors in the script and restart the game\n", AngelScriptPrintPrefix);
        return 1;
    }




    // Find the function that is to be called. 
    asIScriptModule *mod = engine->GetModule("MainGameModule");
    asIScriptFunction *func = mod->GetFunctionByDecl("void main()");
    if( func == 0 )
    {
        // The function couldn't be found. Instruct the script writer
        // to include the expected function in the script.
        printf("%s The script must have the function 'void main()'. Please add it and restart the game\n", AngelScriptPrintPrefix);
        return 1;
    }
    
    // Create our context, prepare it, and then execute
    asIScriptContext *ctx = engine->CreateContext();
    ctx->Prepare(func);
    r = ctx->Execute();
    if( r != asEXECUTION_FINISHED )
    {
        // The execution didn't complete as expected. Determine what happened.
        if( r == asEXECUTION_EXCEPTION )
        {
            // An exception occurred, let the script writer know what happened so it can be corrected.
            printf("%s An exception '%s' occurred. Please correct the code and try again.\n", AngelScriptPrintPrefix, ctx->GetExceptionString());
        }
    }







    // Clean up
    ctx->Release();
    engine->ShutDownAndRelease();







    GameCompute gameCompute;

// std::cout << "Running OverlapTest" << std::endl;
//     gameCompute.OverlapTest();
// std::cout << "Overlap Test Finished" << std::endl;

    // return 0;
    GameGraphics gameGraphics(&gameCompute);      
    gameGraphics.Init();  
    gameCompute.graphics = &gameGraphics;    


    
    std::shared_ptr<GameState> gameState = std::make_shared<GameState>();
    std::shared_ptr<GameStateActions> gameStateActions = std::make_shared<GameStateActions>();

    gameCompute.gameState = gameState;
    gameCompute.gameStateActions = gameStateActions;


    //at this point gamestate is at baseline for the game options.
    gameCompute.SaveGameStateBase();



    GameNetworking gameNetworking(gameState, gameStateActions, &gameCompute);
        

    gameNetworking.Init();
    gameNetworking.Update();

    //Main loop flag
    bool quit = false;

    //Event handler
    SDL_Event e;

    int32_t mousex, mousey;


    gameStateActions->tickIdx = 0;

    std::cout << "GameState Size (bytes): " << sizeof(GameState) << std::endl;

    uint64_t timerStartMs = SDL_GetTicks64();


    #ifdef LOCAL_AUTO_CONNECT
       

        gameNetworking.StartServer(GAMESERVERPORT);
        gameNetworking.ConnectToHost(SLNet::SystemAddress("localhost", GAMESERVERPORT));
        
    #endif


    std::vector<ActionWrap> clientActions;



    std::vector<float> t;
    std::vector<float> profiles[9];

    for(int i = 0; i < 9; i++)
    {
        profiles[i].reserve(500);
    }

    while (!quit)
    {
        timerStartMs = SDL_GetTicks64();

        gameGraphics.BeginDraw();
        
        


        if (gameNetworking.fullyConnectedToHost)
        {
            gameNetworking.CLIENT_SendActionUpdate_ToHost(clientActions);
        }
        gameNetworking.Update();


        if(gameStateActions->pauseState==0)
        {   
            gameStateActions->tickIdx++;
        }


        clientActions.clear();


        gameCompute.Stage1_Begin();
        // gameCompute.Stage1_End();
        
        GSCS(A)


        GameGraphics::RenderClientState* rclientst = &gameGraphics.renderClientState;

        rclientst->mousePrimaryPressed = 0;
        rclientst->mousePrimaryReleased = 0;
        rclientst->mouseSecondaryPressed = 0;
        rclientst->mouseSecondaryReleased = 0;

        //Handle events on queue
        while (SDL_PollEvent(&e) != 0)//build sdl from source and fix window dragging freezing the host....  or have clients sense it?
        {

            ImGui_ImplSDL2_ProcessEvent(&e);

            //User requests quit
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (e.type == SDL_MOUSEWHEEL && !ImGui::GetIO().WantCaptureMouse)
            {
                if (e.wheel.y > 0) // scroll up
                {
                    rclientst->mousescroll_1 = rclientst->mousescroll;
                    rclientst->mousescroll++;
                        
                }
                else if (e.wheel.y < 0) // scroll down
                {
                    rclientst->mousescroll_1 = rclientst->mousescroll;
                    rclientst->mousescroll--;
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && !ImGui::GetIO().WantCaptureMouse)
            {
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    rclientst->mousePrimaryDown = 1;
                    rclientst->mousePrimaryPressed = 1;

                }
                else
                {
                    rclientst->mouseSecondaryDown = 1;
                    rclientst->mouseSecondaryPressed = 1;
                }
            }
            else if (e.type == SDL_MOUSEBUTTONUP && !ImGui::GetIO().WantCaptureMouse)
            {
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    rclientst->mousePrimaryDown = 0;
                    rclientst->mousePrimaryReleased = 1;
                }
                else
                {
                    rclientst->mouseSecondaryDown = 0;
                    rclientst->mouseSecondaryReleased = 1;
                }
            }
            else if (SDL_WINDOWEVENT)
            {

                switch (e.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED:
                    int32_t windowWidth = e.window.data1;
                    int32_t windowHeight = e.window.data2;
                    glViewport(0, 0, windowWidth, windowHeight);
                    break;
                }


            }
        }
        SDL_GetMouseState(&mousex, &mousey);
        rclientst->mousex = mousex;
        rclientst->mousey = mousey;
        rclientst->viewFrameDelta.x = 0;
        rclientst->viewFrameDelta.y = 0;
        if (rclientst->mouseSecondaryPressed)
        {
            rclientst->mouse_dragBeginx = mousex;
            rclientst->mouse_dragBeginy = mousey;
            rclientst->view_beginX = rclientst->viewX;
            rclientst->view_beginY = rclientst->viewY;
        }
        if (rclientst->mousePrimaryPressed)
        {
            rclientst->mouse_dragBeginx = mousex;
            rclientst->mouse_dragBeginy = mousey;
        }

        if (rclientst->mouseSecondaryDown)
        {
            rclientst->viewFrameDelta.x = float(rclientst->mousex - rclientst->mouse_dragBeginx);
            rclientst->viewFrameDelta.y = float(rclientst->mousey - rclientst->mouse_dragBeginy);
            rclientst->viewX = rclientst->view_beginX + rclientst->viewFrameDelta.x;
            rclientst->viewY = rclientst->view_beginY + rclientst->viewFrameDelta.y;
        }
        

        //render
        static float viewScale = 0.002f;
        
        if (rclientst->mousescroll != rclientst->mousescroll_1)
        {
            viewScale -= (rclientst->mousescroll - rclientst->mousescroll_1) * 0.02f;
            viewScale = glm::clamp(viewScale, 0.01f, 0.2f);
            rclientst->mousescroll_1 = rclientst->mousescroll;
        }


        gameGraphics.viewScaleInterp += (viewScale - gameGraphics.viewScaleInterp) * 0.2f;
        //ImGui::SliderFloat("Zoom", &viewScale, 0.001f, 0.2f);
        const float aspectCorrection = float(gameGraphics.SCREEN_HEIGHT) / gameGraphics.SCREEN_WIDTH;
        glm::vec3 scaleVec = glm::vec3(gameGraphics.viewScaleInterp, gameGraphics.viewScaleInterp*0.8f, 1.0f);
        
           
        
        glm::vec3 position = glm::vec3((2.0f* rclientst->viewX) / gameGraphics.SCREEN_WIDTH, ( - 2.0f * rclientst->viewY) / gameGraphics.SCREEN_HEIGHT, 0.0f);
        glm::mat4 view = glm::mat4(1.0f);   
        const float a = 2000.0f;

        rclientst->worldCameraPos.x += ( 1000.0f * rclientst->viewFrameDelta.x* scaleVec.x) / gameGraphics.SCREEN_WIDTH;
        rclientst->worldCameraPos.y += ( 1000.0f * rclientst->viewFrameDelta.y * scaleVec.x) / gameGraphics.SCREEN_HEIGHT;
        view = glm::ortho(rclientst->worldCameraPos.x - scaleVec.x * a,
            rclientst->worldCameraPos.x + scaleVec.x * a,
            rclientst->worldCameraPos.y + scaleVec.y * a,
            rclientst->worldCameraPos.y - scaleVec.y * a);
        


        //glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), position/scaleVec);
        //glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scaleVec);
        //view = scaleMat*translationMat;

        
        glm::mat4 mouseWorldPos(1.0f);
        glm::mat4 mouseWorldPosEnd(1.0f);
        glm::vec4 mouseScreenCoords = glm::vec4(2.0f * (float(mousex) / gameGraphics.SCREEN_WIDTH) - 1.0, -2.0f * (float(mousey) / gameGraphics.SCREEN_HEIGHT) + 1.0, 0.0f, 1.0f);
        glm::vec4 mouseBeginScreenCoords = glm::vec4(2.0f * (float(rclientst->mouse_dragBeginx) / gameGraphics.SCREEN_WIDTH) - 1.0, -2.0f * (float(rclientst->mouse_dragBeginy) / gameGraphics.SCREEN_HEIGHT) + 1.0, 0.0f, 1.0f);
        glm::vec4 worldMouseEnd = glm::inverse(view) * mouseScreenCoords;
        glm::vec4 worldMouseBegin = glm::inverse(view) * mouseBeginScreenCoords;

        int buttonBits = (rclientst->mouseSecondaryReleased << MouseButtonBits_SecondaryReleased) | 
            (rclientst->mousePrimaryReleased << MouseButtonBits_PrimaryReleased)  |
             (rclientst->mouseSecondaryPressed <<MouseButtonBits_SecondaryPressed) | 
             (rclientst->mousePrimaryPressed << MouseButtonBits_PrimaryPressed) |
             (rclientst->mousePrimaryDown << MouseButtonBits_PrimaryDown) |
             (rclientst->mouseSecondaryDown << MouseButtonBits_SecondaryDown);


        if (rclientst->mousePrimaryReleased || rclientst->mousePrimaryPressed || rclientst->mouseSecondaryReleased || rclientst->mouseSecondaryPressed)
        {
            ActionWrap actionWrap;
            ActionWrapInit(&actionWrap);
            actionWrap.tracking.clientId = gameNetworking.clientId;
            actionWrap.tracking.sentTickIdx = gameStateActions->tickIdx;

            actionWrap.action.actionCode = ClientActionCode_MouseStateChange;
            actionWrap.action.intParameters[CAC_MouseStateChange_Param_GUI_X] = int(GE::GUI_PXPERSCREEN_F*(float(mousex) / gameGraphics.SCREEN_WIDTH));
            actionWrap.action.intParameters[CAC_MouseStateChange_Param_GUI_Y] = int(GE::GUI_PXPERSCREEN_F*(float(mousey) / gameGraphics.SCREEN_HEIGHT));
            actionWrap.action.intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16] = int(worldMouseEnd.x*(1<<16));
            actionWrap.action.intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16] = int(worldMouseEnd.y*(1<<16));
            actionWrap.action.intParameters[CAC_MouseStateChange_Param_BUTTON_BITS] = buttonBits;

            clientActions.push_back(actionWrap);

            gameNetworking.actionStateDirty = true;
        }




        gameStateActions->mouseLocx = (float(rclientst->mousex)/gameGraphics.SCREEN_WIDTH)*GE::GUI_PXPERSCREEN_F;
        gameStateActions->mouseLocy = (float(rclientst->mousey)/gameGraphics.SCREEN_HEIGHT)*GE::GUI_PXPERSCREEN_F;

        gameStateActions->mouseLocWorldx_Q16 = int(worldMouseEnd.x*(1<<16));
        gameStateActions->mouseLocWorldy_Q16 = int(worldMouseEnd.y*(1<<16));
        gameStateActions->mouseState = buttonBits;
        
        for(int x = 0; x < 4; x++)
            for(int y = 0; y < 4; y++)
                gameStateActions->viewMatrix[x][y] = view[y][x];

        for(int x = 0; x < 4; x++)
            for(int y = 0; y < 4; y++)
                gameStateActions->viewMatrix_Inv[x][y] = glm::inverse(view)[y][x];


        if (gameStateActions->pauseState == 0)
        {
            if (ImGui::Button("PAUSE"))
            {
                gameStateActions->pauseState = 1;
                if(gameNetworking.serverRunning)
                    gameNetworking.HOST_SendPauseAll_ToClients();
            }

        }
        else
        {
            if (ImGui::Button("RESUME"))
            {
                gameStateActions->pauseState = 0;
                if(gameNetworking.serverRunning)
                    gameNetworking.HOST_SendResumeAll_ToClients();
            }
        }
        ImGui::Begin("Network");
        static int32_t port = 50010;
        ImGui::InputInt("Port", &port, 1, 1);
        ImGui::Text("Server Running: %d", gameNetworking.serverRunning);
        ImGui::Text("Client Running: %d", gameNetworking.connectedToHost);
        ImGui::InputInt("CLI ID", &gameNetworking.prefferedClientID);

        if(gameNetworking.prefferedClientID < 0) 
            gameNetworking.prefferedClientID = 0;
        
        ImGui::Text("Assigned GameState Client Idx: %d", gameNetworking.clientId);
        

        ImGui::Text("Num Connections: %d", gameNetworking.clients.size());
        if(!gameNetworking.serverRunning && !gameNetworking.connectedToHost)
            if (ImGui::Button("Start Server"))
            {
                gameNetworking.StartServer(port);
            }
        
        if(gameNetworking.serverRunning)
            if(ImGui::Button("Stop Server"))
            {
                gameNetworking.StopServer();
            }

        static char connectIPString[256] = "localhost";



        if (!gameNetworking.connectedToHost)
        {
            ImGui::InputText("IP", connectIPString, 256);
            if (ImGui::Button("Connect To Local Server"))
            {
                gameNetworking.ConnectToHost(SLNet::SystemAddress(&connectIPString[0], port));
            }

        }
        if (gameNetworking.connectedToHost)
        {
            if (ImGui::Button("CLIENT_HardDisconnect"))
            {
                gameNetworking.CLIENT_HardDisconnect();
            }
            if (ImGui::Button("CLIENT_SoftDisconnect"))
            {
                gameNetworking.CLIENT_SoftDisconnect();
            }
        }



        if (ImGui::Button("Send Message"))
        {
            char buffer[256];
            sprintf_s(buffer, "msg %d", gameStateActions->tickIdx);
            gameNetworking.SendMessage(buffer);
        }
        ImGui::Text("FrameTime: %f, TargetTickTime: %f, PID Error %f", gameNetworking.lastFrameTimeMs , gameNetworking.targetTickTimeMs, gameNetworking.tickPIDError);
                
        if (gameNetworking.gameStateTransferPercent > 0.0f)
        {
                   
            ImGui::Text("Downloading:");
            ImGui::SameLine();
            ImGui::ProgressBar(gameNetworking.gameStateTransferPercent);
        }

        if (ImGui::Button("Print CHECKSUM"))
        {
            printf("todo - never.\n");
        }

        if(ImGui::Button("Show Action Ticks To Spare"))
        {
            for(auto t : gameNetworking.clientTicksToSpare)
                printf("%d\n", t);
        }


        ImGui::Text("CLIENT_actionList Size: %d", gameNetworking.CLIENT_actionList.size());

        for (auto client : gameNetworking.clients)
        {
            ImGui::Text("CliId: %d, HostTickOffset: %d, Ping: %d, GUID %u, RakGUID: %u", client.cliId,  client.hostTickOffset, client.avgHostPing, client.clientGUID,
                SLNet::RakNetGUID::ToUint32(client.rakGuid));
        }
                
        if (ImGui::Button("Save GameState To File"))
        {   
            std::ofstream myfile;
            myfile.open("gamestate.bin", std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
            if (!myfile.is_open())
                std::cout << "Error Saving File!" << std::endl;
            else
            {
                myfile.write(reinterpret_cast<char*>(gameNetworking.gameState.get()), sizeof(GameState));
            }
            
            myfile.close();
        }
        if (ImGui::Button("Load GameState From File"))
        {

            std::ifstream myfile;
            myfile.open("gamestate.bin", std::ifstream::binary);
            if (!myfile.is_open())
                std::cout << "Error Reading File!" << std::endl;
            else {
                std::cout << "Reading " <<  sizeof(GameState) << " bytes" << std::endl;
                myfile.read(reinterpret_cast<char*>(gameCompute.gameState.get()),  sizeof(GameState));
            
                if (myfile)
                    std::cout << "all characters read successfully.";
                else
                    std::cout << "error: only " << myfile.gcount() << " could be read";
            
            }
            myfile.close();
        }
        if(ImGui::Button("Save Diff"))
        {
            std::vector<char> data;
            gameCompute.Stage1_End();
            gameCompute.SaveGameStateDiff(&data, false);
        }
        static int loadtickIdx = 0;
        ImGui::InputInt("Load Tick",&loadtickIdx);
        if(ImGui::Button("Load Diff ^"))
        {
            gameCompute.Stage1_End();
            gameCompute.LoadGameStateFromDiff(loadtickIdx);
        }

        ImGui::End();
        glm::vec4 worldMouseCoords = glm::inverse(view) * mouseScreenCoords;
        ImGui::Text("X,y: %f,%f", worldMouseCoords.x, worldMouseCoords.y);
        

        //-----------------------------------------------------------------------------------





        
        glActiveTexture(GL_TEXTURE0); // Texture unit 0
        glBindTexture(GL_TEXTURE_2D, gameGraphics.mapTileTexId);

        
        //draw mouse
        {

            gameGraphics.pBasicShadProgram->Use();

            glm::mat4 I(1.0f);
            gameGraphics.pBasicShadProgram->SetUniform_Mat4("WorldToScreenTransform", I);


            glm::mat4 drawingTransform(1.0f);
            gameGraphics.pBasicShadProgram->SetUniform_Mat4("LocalTransform", drawingTransform);
            glm::vec3 c(1.0f, 1.0f, 1.0f);
            gameGraphics.pBasicShadProgram->SetUniform_Vec3("OverallColor", c);


            float mouseSelectVerts[] = {
                //positions    
                mouseBeginScreenCoords.x,  mouseBeginScreenCoords.y,
                mouseScreenCoords.x, mouseBeginScreenCoords.y,
                mouseScreenCoords.x , mouseScreenCoords.y ,
                mouseBeginScreenCoords.x, mouseScreenCoords.y
            };

            uint32_t mouseVAO, mouseVBO;
            glGenVertexArrays(1, &mouseVAO);
            glGenBuffers(1, &mouseVBO);
            glBindVertexArray(mouseVAO);
            glBindBuffer(GL_ARRAY_BUFFER, mouseVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(mouseSelectVerts), mouseSelectVerts, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

            if (rclientst->mousePrimaryDown)
                glDrawArrays(GL_LINE_LOOP, 0, 4);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            glDeleteVertexArrays(1, &mouseVAO);
            glDeleteBuffers(1, &mouseVBO);
        }





        GSCS(D)
        gameCompute.Stage1_End();


        ImGui::Begin("Profiling");



        ImGui::Text("TickIdx: %d", gameStateActions->tickIdx);

                
        if (ImPlot::BeginPlot("Times")) {

            ImPlot::SetupAxes("Tick","Time");
            ImPlot::SetupAxesLimits(0,500,0,gameNetworking.targetTickTimeMs*2);

            
            ImPlot::PlotLine("actionEvent", profiles[0].data(),      500);
            ImPlot::PlotLine("guiEvent", profiles[1].data(),         500);
            ImPlot::PlotLine("preUpdateEvent1", profiles[2].data(),  500);
            ImPlot::PlotLine("preUpdateEvent2", profiles[3].data(),  500);
            ImPlot::PlotLine("updatepre1Event", profiles[4].data(),  500);
            ImPlot::PlotLine("updateEvent", profiles[5].data(),      500);
            ImPlot::PlotLine("update2Event", profiles[6].data(),     500);
            ImPlot::PlotLine("postupdateEvent", profiles[7].data(),  500);
            ImPlot::PlotLine("FrameTime", profiles[8].data(),  500);

            
            ImPlot::PlotInfLines("Limit",&gameNetworking.targetTickTimeMs,1,ImPlotInfLinesFlags_Horizontal);
            int d = gameStateActions->tickIdx%501;
            ImPlot::PlotInfLines("Tick",&d,1);

            ImPlot::EndPlot();
        }


        ImGui::End();


        gameGraphics.Swap();
        WaitTickTime(timerStartMs, gameNetworking.targetTickTimeMs, &gameNetworking.lastFrameTimeMs);
    }



    // Clean up
        
    return 0;


}



