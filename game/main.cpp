
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "glew.h"
#include "glfw3native.h"

#include <SDL.h>
#include "SDL_opengl.h"


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
#include "GameGPUCompute.h"


#include "GEShader.h"
#include "GEShaderProgram.h"

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


void ProfileEvent(cl_event event, std::string label, cl_ulong* startNanoSec, bool isFirst = false)
{
    cl_ulong time_start;
    cl_ulong time_end;

    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
    double nanoSeconds = static_cast<double>(time_end - time_start);

    cl_ulong nanoStartLocal = time_start;
    cl_ulong nanoEndLocal = time_end;
    if(isFirst == true)
    {
        time_end = time_end - time_start;
        nanoEndLocal = time_end;
        nanoStartLocal = 0;
        *startNanoSec = time_start;
    }
    else
    {
        nanoStartLocal -= *startNanoSec;
        nanoEndLocal -= *startNanoSec;
    }

    ImGui::Text("%s Execution time is: %0.3f milliseconds, ST: %f, ET: %f", label.data(), 
    nanoSeconds / 1000000.0,
    nanoStartLocal / 1000000.0,
    nanoEndLocal/ 1000000.0);


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







    GameGPUCompute gameCompute;
    GameGraphics gameGraphics(&gameCompute);      
    gameGraphics.Init();  
    gameCompute.graphics = &gameGraphics;    

    gameCompute.RunInitCompute0();
    gameCompute.BuildKernelRunSizes();
    
    gameCompute.AddCompileDefinition("PEEP_VBO_INSTANCE_SIZE", gameGraphics.peepInstanceSIZE);
    gameCompute.AddCompileDefinition("PARTICLE_VBO_INSTANCE_SIZE", gameGraphics.particleInstanceSIZE);
    gameCompute.AddCompileDefinition("MAX_PEEPS", gameCompute.maxPeeps);
    gameCompute.AddCompileDefinition("MAX_PARTICLES", gameCompute.maxParticles);
    gameCompute.AddCompileDefinition("MAPDIM", gameCompute.mapDim);
    gameCompute.AddCompileDefinition("MAPDEPTH", gameCompute.mapDepth);
    gameCompute.AddCompileDefinition("WARPSIZE", gameCompute.warpSize);
    gameCompute.AddCompileDefinition("GAME_UPDATE_WORKITEMS", gameCompute.GameUpdateWorkItems);
    gameCompute.AddCompileDefinition("MAX_CLIENTS", MAX_CLIENTS);
    gameCompute.AddCompileDefinition("MAP_TILE_SIZE", gameCompute.mapTileSize);
    gameCompute.AddCompileDefinition("MAX_GUI_RECTS", gameCompute.maxGuiRects);
    gameCompute.AddCompileDefinition("MAX_LINES", gameCompute.maxLines);
    gameCompute.RunInitCompute1();
    
    std::shared_ptr<GameState_Pointer> gameState = std::make_shared<GameState_Pointer>(gameCompute.structSizes.gameStateStructureSize);
    std::shared_ptr<GameStateActions> gameStateActions = std::make_shared<GameStateActions>();

    gameCompute.gameState = gameState;
    gameCompute.gameStateActions = gameStateActions;

    gameCompute.RunInitCompute2();

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

    std::cout << "GameState Size (bytes): " << gameCompute.structSizes.gameStateStructureSize << std::endl;

    uint64_t timerStartMs = SDL_GetTicks64();


    #ifdef LOCAL_AUTO_CONNECT
       

        gameNetworking.StartServer(GAMESERVERPORT);
        gameNetworking.ConnectToHost(SLNet::SystemAddress("localhost", GAMESERVERPORT));
        
    #endif


    std::vector<ActionWrap> clientActions;

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

        gameCompute.WriteGameStateB();
        clientActions.clear();


        gameCompute.Stage1_Begin();
        gameCompute.Stage1_End();
        
        GSCS(A)

        ImGui::Begin("Profiling");
                
        // clGetEventProfilingInfo(gameCompute.actionEvent, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
        // clGetEventProfilingInfo(gameCompute.actionEvent, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
        // double nanoSeconds = static_cast<double>(time_end - time_start);
        // ImGui::Text("actionEvent Execution time is: %0.3f milliseconds", nanoSeconds / 1000000.0);
        cl_ulong nanoSec;
        cl_ulong nanoSec2;
        ProfileEvent(gameCompute.actionEvent, "actionEvent", &nanoSec, true);
        ProfileEvent(gameCompute.guiEvent, "guiEvent", &nanoSec2, true);

        ProfileEvent(gameCompute.preUpdateEvent1, "preUpdateEvent1", &nanoSec);
        ProfileEvent(gameCompute.preUpdateEvent2, "preUpdateEvent2", &nanoSec);
        ProfileEvent(gameCompute.updatepre1Event, "updatepre1Event", &nanoSec);
        ProfileEvent(gameCompute.updateEvent, "updateEvent", &nanoSec);
        ProfileEvent(gameCompute.update2Event, "update2Event", &nanoSec);
        ProfileEvent(gameCompute.postupdateEvent, "postupdateEvent", &nanoSec);

        //ProfileEvent(gameCompute.writeEvent, "writeEvent (CPU->GPU)");
        //ProfileEvent(gameCompute.readEvent, "readEvent (GPU->CPU)");







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
            else if (e.type == SDL_MOUSEWHEEL)
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
            else if (e.type == SDL_MOUSEBUTTONDOWN)
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
            else if (e.type == SDL_MOUSEBUTTONUP)
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
            actionWrap.action.intParameters[CAC_MouseStateChange_Param_GUI_X] = int(GUI_PXPERSCREEN_F*(float(mousex) / gameGraphics.SCREEN_WIDTH));
            actionWrap.action.intParameters[CAC_MouseStateChange_Param_GUI_Y] = int(GUI_PXPERSCREEN_F*(float(mousey) / gameGraphics.SCREEN_HEIGHT));
            actionWrap.action.intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16] = int(worldMouseEnd.x*(1<<16));
            actionWrap.action.intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16] = int(worldMouseEnd.y*(1<<16));
            actionWrap.action.intParameters[CAC_MouseStateChange_Param_BUTTON_BITS] = buttonBits;

            clientActions.push_back(actionWrap);

            gameNetworking.actionStateDirty = true;
        }




        gameStateActions->mouseLocx = (float(rclientst->mousex)/gameGraphics.SCREEN_WIDTH)*GUI_PXPERSCREEN_F;
        gameStateActions->mouseLocy = (float(rclientst->mousey)/gameGraphics.SCREEN_HEIGHT)*GUI_PXPERSCREEN_F;

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
                gameCompute.Stage1_End();
                gameStateActions->pauseState = 1;
                if(gameNetworking.serverRunning)
                    gameNetworking.HOST_SendPauseAll_ToClients();
            }

        }
        else
        {
            if (ImGui::Button("RESUME"))
            {
                gameCompute.Stage1_End();
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
                gameCompute.Stage1_End();
                gameNetworking.StartServer(port);
            }
        
        if(gameNetworking.serverRunning)
            if(ImGui::Button("Stop Server"))
            {
                gameCompute.Stage1_End();
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
                gameCompute.Stage1_End();
                gameNetworking.CLIENT_HardDisconnect();
            }
            if (ImGui::Button("CLIENT_SoftDisconnect"))
            {
                gameCompute.Stage1_End();
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
            gameCompute.Stage1_End();
            gameCompute.ReadFullGameState();
            std::ofstream myfile;
            myfile.open("gamestate.bin", std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
            if (!myfile.is_open())
                std::cout << "Error Saving File!" << std::endl;
            else
            {
                myfile.write(reinterpret_cast<char*>(gameNetworking.gameState.get()->data), gameCompute.structSizes.gameStateStructureSize);
            }
            
            myfile.close();
        }
        if (ImGui::Button("Load GameState From File"))
        {
            gameCompute.Stage1_End();
            std::ifstream myfile;
            myfile.open("gamestate.bin", std::ifstream::binary);
            if (!myfile.is_open())
                std::cout << "Error Reading File!" << std::endl;
            else {
                std::cout << "Reading " << gameCompute.structSizes.gameStateStructureSize << " bytes" << std::endl;
                myfile.read(reinterpret_cast<char*>(gameCompute.gameState.get()->data), gameCompute.structSizes.gameStateStructureSize);
            
                if (myfile)
                    std::cout << "all characters read successfully.";
                else
                    std::cout << "error: only " << myfile.gcount() << " could be read";
            
            }
            myfile.close();

            gameCompute.WriteFullGameState();
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
        gameCompute.Stage1_End();





        
        glActiveTexture(GL_TEXTURE0); // Texture unit 0
        glBindTexture(GL_TEXTURE_2D, gameGraphics.mapTileTexId);

        //draw map
        gameGraphics.pMapTileShadProgram->Use();
        gameGraphics.pMapTileShadProgram->SetUniform_Mat4("projection", view);
        glm::mat4 mapTransform(1.0f);
        mapTransform = glm::scale(mapTransform, glm::vec3(gameCompute.mapTileSize, gameCompute.mapTileSize, 1));
        mapTransform = glm::translate(mapTransform, glm::vec3(-gameCompute.mapDim * 0.5f, -gameCompute.mapDim * 0.5f, 0));
        
        gameGraphics.pMapTileShadProgram->SetUniform_Mat4("localTransform", mapTransform);
        glm::ivec2 mapSize = { gameCompute.mapDim , gameCompute.mapDim };
        gameGraphics.pMapTileShadProgram->SetUniform_IVec2("mapSize", mapSize);

        glBindVertexArray(gameGraphics.mapTile1VAO);
        glDrawArrays(GL_POINTS, 0, gameCompute.mapDim* gameCompute.mapDim);
        glBindVertexArray(0);


        //draw map shadows
        gameGraphics.pMapTileShadProgram->Use();
        glBindVertexArray(gameGraphics.mapTile2VAO);
        glDrawArrays(GL_POINTS, 0, gameCompute.mapDim * gameCompute.mapDim);
        glBindVertexArray(0);

        //draw all peeps
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        gameGraphics.pPeepShadProgram->Use();
        gameGraphics.pPeepShadProgram->SetUniform_Mat4("WorldToScreenTransform", view);

        glBindVertexArray(gameGraphics.peepVAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, gameCompute.maxPeeps);
        glBindVertexArray(0);


        //draw all particles
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        gameGraphics.pParticleShadProgram->Use();
        gameGraphics.pParticleShadProgram->SetUniform_Mat4("WorldToScreenTransform", view);

        glBindVertexArray(gameGraphics.particleVAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, gameCompute.maxParticles);
        glBindVertexArray(0);

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

        //draw lines 
        glBindVertexArray(gameGraphics.linesVAO);
        glDrawArrays(GL_LINES, 0, gameCompute.maxLines);
        glBindVertexArray(0);


        //draw gui
        glActiveTexture(GL_TEXTURE0); // Texture unit 0
        glBindTexture(GL_TEXTURE_2D, gameGraphics.lettersTileTexId);
        glActiveTexture(GL_TEXTURE1); // Texture unit 0
        glBindTexture(GL_TEXTURE_2D, gameGraphics.mapTileTexId);

        gameGraphics.pGuiShadProgram->Use();
        glm::mat4 identity(1.0);
        glm::vec3 c(1.0f, 1.0f, 1.0f);
        gameGraphics.pGuiShadProgram->SetUniform_Mat4("WorldToScreenTransform", identity);
        gameGraphics.pGuiShadProgram->SetUniform_Vec3("OverallColor", c);
        gameGraphics.pGuiShadProgram->SetUniform_Mat4("LocalTransform", identity);
        gameGraphics.pGuiShadProgram->SetUniform_Int("texture0", 0);
        gameGraphics.pGuiShadProgram->SetUniform_Int("texture1", 1);




        glBindVertexArray(gameGraphics.guiRectVAO);
        glDrawArrays(GL_TRIANGLES, 0, gameCompute.maxGuiRects*6);
        glBindVertexArray(0);






        GSCS(D)







        ImGui::Text("TickIdx: %d", gameStateActions->tickIdx);
        ImGui::End();


        gameGraphics.Swap();
        WaitTickTime(timerStartMs, gameNetworking.targetTickTimeMs, &gameNetworking.lastFrameTimeMs);

        if (gameCompute.errorState)
            quit = true;
    }



    // Clean up
        
    return 0;


}



