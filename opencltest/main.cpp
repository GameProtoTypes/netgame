
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
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


#include "peep.h"
#include "PeepFuncs.h"


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

//#define GSCSTESTS
#ifdef GSCSTESTS
#define GSCS(ID) { std::cout << #ID << ": GSCS: " << gameNetworking.CheckSumGameState(gameState.get()) << " TickIdx: " << gameState.get()->tickIdx << std::endl;}
#else
#define GSCS(ID) {}
#endif

int32_t random(int32_t min, int32_t max) { return rand() % (max - min + 1) + min; }

void WaitMinTickTime(uint64_t timerStartMs, int32_t targetTimeMs)
{
    int64_t frameTimeMS = SDL_GetTicks64() - timerStartMs;
    int32_t sleepTime = glm::clamp(int32_t(targetTimeMs - frameTimeMS), 0, targetTimeMs);
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
}

int32_t main(int32_t argc, char* args[]) 
{
    GameGraphics gameGraphics;

    std::shared_ptr<GameState> gameState = std::make_shared<GameState>();

    GameGPUCompute gameCompute(gameState);

    GameNetworking gameNetworking(gameState);
        
    gameNetworking.Init();
    gameNetworking.Update();

    //Main loop flag
    bool quit = false;

    //Event handler
    SDL_Event e;

    int32_t mousex, mousey;

    gameState->mapHeight = 2000;
    gameState->mapWidth = 2000;
    gameState->tickIdx = 0;

    std::cout << "GameState Size (bytes): " << sizeof(GameState) << std::endl;

    uint64_t timerStartMs = SDL_GetTicks64();


    std::shared_ptr<ClientSideClientState> client = std::make_shared<ClientSideClientState>();

    while (!quit)
    {
        timerStartMs = SDL_GetTicks64();

        gameGraphics.BeginDraw();
            
        gameCompute.Stage1();

        
        GSCS(A)
        cl_ulong time_start;
        cl_ulong time_end;

        clGetEventProfilingInfo(gameCompute.updateEvent, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
        clGetEventProfilingInfo(gameCompute.updateEvent, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);

        ImGui::Begin("Profiling");
        double nanoSeconds = time_end - time_start;
        ImGui::Text("update_kernel Execution time is: %0.3f milliseconds", nanoSeconds / 1000000.0);
        clGetEventProfilingInfo(gameCompute.preUpdateEvent1, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
        clGetEventProfilingInfo(gameCompute.preUpdateEvent2, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);

        nanoSeconds = time_end - time_start;
        ImGui::Text("preupdate_kernel Execution time is: %0.3f milliseconds", nanoSeconds / 1000000.0);
            
        clGetEventProfilingInfo(gameCompute.readEvent, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
        clGetEventProfilingInfo(gameCompute.readEvent, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
        nanoSeconds = time_end - time_start;
        ImGui::Text("GPU->CPU Transfer time is: %0.3f milliseconds", nanoSeconds / 1000000.0);

        client->mousePrimaryPressed = 0;
        client->mousePrimaryReleased = 0;
        client->mouseSecondaryPressed = 0;
        client->mouseSecondaryReleased = 0;
        //Handle events on queue
        while (SDL_PollEvent(&e) != 0)
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
                    //if (gameState->viewScale >= 1 && gameState->viewScale < 15)
                    client->mousescroll++;
                        
                }
                else if (e.wheel.y < 0) // scroll down
                {
                    // if (gameState->viewScale > 1)
                    client->mousescroll--;
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    client->mousePrimaryDown = 1;
                    client->mousePrimaryPressed = 1;

                }
                else
                {
                    client->mouseSecondaryDown = 1;
                    client->mouseSecondaryPressed = 1;
                }
            }
            else if (e.type == SDL_MOUSEBUTTONUP)
            {
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    client->mousePrimaryDown = 0;
                    client->mousePrimaryReleased = 1;
                }
                else
                {
                    client->mouseSecondaryDown = 0;
                    client->mouseSecondaryReleased = 1;
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
        client->mousex = mousex;
        client->mousey = mousey;
        client->viewFrameDelta.x = 0;
        client->viewFrameDelta.y = 0;
        if (client->mouseSecondaryPressed)
        {
            client->mouse_dragBeginx = mousex;
            client->mouse_dragBeginy = mousey;
            client->view_beginX = client->viewX;
            client->view_beginY = client->viewY;
        }
        if (client->mousePrimaryPressed)
        {
            client->mouse_dragBeginx = mousex;
            client->mouse_dragBeginy = mousey;
        }

        if (client->mouseSecondaryDown)
        {
            client->viewFrameDelta.x = (client->mousex - client->mouse_dragBeginx);
            client->viewFrameDelta.y = (client->mousey - client->mouse_dragBeginy);
            client->viewX = client->view_beginX + client->viewFrameDelta.x;
            client->viewY = client->view_beginY + client->viewFrameDelta.y;
            
        }
        
          


        //render
        static float viewScale = 0.002f;
        viewScale = client->mousescroll* client->mousescroll*0.01f;
        viewScale = glm::clamp(viewScale, 0.001f, 0.2f);
        //ImGui::SliderFloat("Zoom", &viewScale, 0.001f, 0.2f);
        glm::vec3 scaleVec = glm::vec3(viewScale, viewScale , 1.0f);
           
        
        glm::vec3 position = glm::vec3((2.0f* client->viewX) / gameGraphics.SCREEN_WIDTH, ( - 2.0f * client->viewY) / gameGraphics.SCREEN_HEIGHT, 0.0f);
        glm::mat4 view = glm::mat4(1.0f);   
        const float a = 2000.0f;

        client->worldCameraPos.x += ( 1000.0f * client->viewFrameDelta.x* scaleVec.x) / gameGraphics.SCREEN_WIDTH;
        client->worldCameraPos.y += ( 1000.0f * client->viewFrameDelta.y * scaleVec.x) / gameGraphics.SCREEN_HEIGHT;
        view = glm::ortho(client->worldCameraPos.x - scaleVec.x * a,
            client->worldCameraPos.x + scaleVec.x * a,
            client->worldCameraPos.y + scaleVec.y * a,
            client->worldCameraPos.y - scaleVec.y * a);
        


        //glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), position/scaleVec);
        //glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scaleVec);

        //view = scaleMat*translationMat;

        
        glm::mat4 mouseWorldPos(1.0f);
        glm::mat4 mouseWorldPosEnd(1.0f);
        glm::vec4 mouseScreenCoords = glm::vec4(2.0f * (float(mousex) / gameGraphics.SCREEN_WIDTH) - 1.0, -2.0f * (float(mousey) / gameGraphics.SCREEN_HEIGHT) + 1.0, 0.0f, 1.0f);
        glm::vec4 mouseBeginScreenCoords = glm::vec4(2.0f * (float(client->mouse_dragBeginx) / gameGraphics.SCREEN_WIDTH) - 1.0, -2.0f * (float(client->mouse_dragBeginy) / gameGraphics.SCREEN_HEIGHT) + 1.0, 0.0f, 1.0f);
        glm::vec4 worldMouseEnd = glm::inverse(view) * mouseScreenCoords;
        glm::vec4 worldMouseBegin = glm::inverse(view) * mouseBeginScreenCoords;



        std::vector<ActionWrap> clientActions;
        if (client->mousePrimaryReleased)
        {
            ActionWrap actionWrap;
            ActionWrapInit(&actionWrap);
            actionWrap.tracking.clientId = gameNetworking.clientId;
            float endx   = glm::max(worldMouseBegin.x, worldMouseEnd.x);
            float endy   = glm::min(worldMouseBegin.y, worldMouseEnd.y);
            float startx = glm::min(worldMouseBegin.x, worldMouseEnd.x);
            float starty = glm::max(worldMouseBegin.y, worldMouseEnd.y);

            actionWrap.action.action_DoSelect = 1;
            actionWrap.action.params_DoSelect_StartX_Q16 = cl_int(startx * (1 << 16));
            actionWrap.action.params_DoSelect_StartY_Q16 = cl_int(starty * (1 << 16));
            actionWrap.action.params_DoSelect_EndX_Q16   = cl_int(endx   * (1 << 16));
            actionWrap.action.params_DoSelect_EndY_Q16   = cl_int(endy   * (1 << 16));


                
            clientActions.push_back(actionWrap);

            gameNetworking.actionStateDirty = true;
        }

            
        if (client->mouseSecondaryReleased)
        {

            ActionWrap actionWrap;
            ActionWrapInit(&actionWrap);
            actionWrap.tracking.clientId = gameNetworking.clientId;
            actionWrap.action.action_CommandToLocation = 1;
            actionWrap.action.params_CommandToLocation_X_Q16 = cl_int(worldMouseEnd.x * (1 << 16)) ;
            actionWrap.action.params_CommandToLocation_Y_Q16 = cl_int(worldMouseEnd.y * (1 << 16));



            clientActions.push_back(actionWrap);

            gameNetworking.actionStateDirty = true;

        }
        GSCS(B)
        if (gameNetworking.fullyConnectedToHost)
        {
            gameNetworking.CLIENT_SendActionUpdate_ToHost(clientActions);
        }
        gameNetworking.Update();

        GSCS(C)
            
        //apply turns
        for (int32_t a = 0; a < gameState->numActions; a++)
        {
            ClientAction* clientAction = &gameState->clientActions[a].action;
            ActionTracking* actionTracking = &gameState->clientActions[a].tracking;
            cl_uchar cliId = actionTracking->clientId;
            SynchronizedClientState* client = &gameState->clientStates[cliId];

            if (clientAction->action_DoSelect)
            {
                client->selectedPeepsLastIdx = OFFSET_NULL;
                for (cl_uint pi = 0; pi < MAX_PEEPS; pi++)
                {
                    Peep* p = &gameState->peeps[pi];

                    if (p->faction == actionTracking->clientId)
                        if ((p->map_x_Q15_16 > clientAction->params_DoSelect_StartX_Q16)
                            && (p->map_x_Q15_16 < clientAction->params_DoSelect_EndX_Q16))
                        {

                            if ((p->map_y_Q15_16 < clientAction->params_DoSelect_StartY_Q16)
                                && (p->map_y_Q15_16 > clientAction->params_DoSelect_EndY_Q16))
                            {

                                if (client->selectedPeepsLastIdx != OFFSET_NULL)
                                {
                                    gameState->peeps[client->selectedPeepsLastIdx].nextSelectionPeepIdx[cliId] = pi;
                                    p->prevSelectionPeepIdx[cliId] = client->selectedPeepsLastIdx;
                                    p->nextSelectionPeepIdx[cliId] = OFFSET_NULL;
                                }
                                else
                                {
                                    p->prevSelectionPeepIdx[cliId] = OFFSET_NULL;
                                    p->nextSelectionPeepIdx[cliId] = OFFSET_NULL;
                                }
                                client->selectedPeepsLastIdx = pi;
                            }
                        }
                }

            }
            else if (clientAction->action_CommandToLocation)
            {
                cl_uint curPeepIdx = client->selectedPeepsLastIdx;
                while (curPeepIdx != OFFSET_NULL)
                {
                    Peep* curPeep = &gameState->peeps[curPeepIdx];
                    curPeep->target_x_Q16 = clientAction->params_CommandToLocation_X_Q16;
                    curPeep->target_y_Q16 = clientAction->params_CommandToLocation_Y_Q16;

                    curPeepIdx = curPeep->prevSelectionPeepIdx[cliId];
                }


            }
        }

        gameState->numActions = 0;
                






        ImGui::Begin("Company");
        ImGui::Button("Assets");
        ImGui::Button("Profit/Loss");
        ImGui::End();

        ImGui::Begin("Build");
        ImGui::Button("Tracks");
        ImGui::Button("Machines");
        ImGui::Button("Bulldoze");

        ImGui::End();

        ImGui::Begin("Routines");

        ImGui::End();

        ImGui::Begin("Miner Bots");
        ImGui::Button("New Miner");
        ImGui::Button("Destroy Miner");
        ImGui::End();

        if (gameState->pauseState == 0)
        {
            if (ImGui::Button("PAUSE"))
                gameState->pauseState = 1;
        }
        else
        {
            if (ImGui::Button("RESUME"))
                gameState->pauseState = 0;
        }
        ImGui::Begin("Network");
        static int32_t port = 50010;
        ImGui::InputInt("Port", &port, 1, 1);
        ImGui::Text("Server Running: %d", gameNetworking.serverRunning);
        ImGui::Text("Client Running: %d", gameNetworking.connectedToHost);
        ImGui::Text("GameState Client Idx: %d", gameNetworking.clientId);
                
        ImGui::Text("Num Connections: %d", gameNetworking.clients.size());
        if(!gameNetworking.serverRunning && !gameNetworking.connectedToHost)
            if (ImGui::Button("Start Server"))
            {
                gameNetworking.StartServer(port);
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
            if (ImGui::Button("CLIENT_Disconnect"))
            {
                gameNetworking.CLIENT_Disconnect();
            }
        }



        if (ImGui::Button("Send Message"))
        {
            char buffer[256];
            sprintf(buffer, "HELLOOOO %d", gameState->tickIdx);
            gameNetworking.SendMessage(buffer);
        }
        ImGui::Text("TargetTickTime: %d, PID Error %f", gameNetworking.targetTickTimeMs, gameNetworking.tickPIDError);
                
        if (gameNetworking.gameStateTransferPercent > 0.0f)
        {
                   
            ImGui::Text("Downloading:");
            ImGui::SameLine();
            ImGui::ProgressBar(gameNetworking.gameStateTransferPercent);
        }



        for (auto client : gameNetworking.clients)
        {
            ImGui::Text("CliId: %d, GUID %u, RakGUID: %u, HostTickOffset: %d, Ping: %d", client.cliId, client.clientGUID,
                SLNet::RakNetGUID::ToUint32(client.rakGuid), client.hostTickOffset, client.avgHostPing);
        }
                



        ImGui::End();






        gameGraphics.pPeepShadProgram->Use();
        
        gameGraphics.pPeepShadProgram->SetUniform_Mat4("WorldToScreenTransform", view);



        cl_uint curPeepIdx = gameState->clientStates[gameNetworking.clientId].selectedPeepsLastIdx;
        PeepRenderSupport peepRenderSupport[MAX_PEEPS];
        while (curPeepIdx != OFFSET_NULL)
        {
            Peep* p = &gameState->peeps[curPeepIdx];

            peepRenderSupport[curPeepIdx].render_selectedByClient = 1;

            curPeepIdx = p->prevSelectionPeepIdx[gameNetworking.clientId];
        }




        for (int32_t pi = 0; pi < MAX_PEEPS; pi++)
        {
            Peep* p = &gameState->peeps[pi];


            float brightFactor = 0.6f;
            if (p->faction == 1)
            {
                gameGraphics.colors[pi].r = 0.0f;
                gameGraphics.colors[pi].g = 1.0f;
                gameGraphics.colors[pi].b = 1.0f;
            }
            else
            {
                gameGraphics.colors[pi].r = 1.0f;
                gameGraphics.colors[pi].g = 0.0f;
                gameGraphics.colors[pi].b = 1.0f;
            }

            if (peepRenderSupport[pi].render_selectedByClient)
            {
                brightFactor = 1.0f;
                peepRenderSupport[pi].render_selectedByClient = 0;
            }
            if (p->deathState == 1)
            {
                brightFactor = 0.6f;
                gameGraphics.colors[pi].r = 0.5f;
                gameGraphics.colors[pi].g = 0.5f;
                gameGraphics.colors[pi].b = 0.5f;
            }
            if (p->attackState == 1)
            {
                brightFactor = 1.0f;
                gameGraphics.colors[pi].r = 1.0f;
                gameGraphics.colors[pi].g = 1.0f;
                gameGraphics.colors[pi].b = 1.0f;
            }




            gameGraphics.colors[pi] = gameGraphics.colors[pi] * brightFactor;

            float x = float(p->map_x_Q15_16) / float(1 << 16);
            float y = float(p->map_y_Q15_16) / float(1 << 16);

            float xv = p->xv_Q15_16 / float(1 << 16);
            float yv = p->yv_Q15_16 / float(1 << 16);

            float angle = atan2f(yv, xv);


            gameGraphics.worldPositions[pi] = glm::vec2(x, y);


            glm::mat4 localMatrix = glm::mat4(1.0f);
            localMatrix = glm::translate(localMatrix, glm::vec3(x, y, 0));
            localMatrix = glm::rotate(localMatrix, angle * (180.0f / 3.1415f) - 90.0f, glm::vec3(0, 0, 1));

            glm::vec2 location2D = glm::vec2(x, y);
            glBindBuffer(GL_ARRAY_BUFFER, gameGraphics.instanceVBO);
            int32_t stride = (sizeof(glm::vec2) + sizeof(glm::vec3));
            glBufferSubData(GL_ARRAY_BUFFER, pi * stride, sizeof(glm::vec2), &location2D.x);
            glBufferSubData(GL_ARRAY_BUFFER, pi * stride + sizeof(glm::vec2), sizeof(glm::vec3), &gameGraphics.colors[pi].r);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

        }


        //draw all peeps
        glBindVertexArray(gameGraphics.quadVAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, MAX_PEEPS);
        glBindVertexArray(0);

        //draw mouse
        {

            gameGraphics.pBasicShadProgram->Use();


            gameGraphics.pBasicShadProgram->SetUniform_Mat4("WorldToScreenTransform", glm::mat4(1.0f));


            glm::mat4 drawingTransform(1.0f);
            gameGraphics.pBasicShadProgram->SetUniform_Mat4("LocalTransform", drawingTransform);
            gameGraphics.pBasicShadProgram->SetUniform_Vec3("OverallColor", glm::vec3(1.0f, 1.0f, 1.0f));


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

            if (client->mousePrimaryDown)
                glDrawArrays(GL_LINE_LOOP, 0, 4);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }

        if(gameState->pauseState==0)
            gameState->tickIdx++;

        GSCS(D)
        gameCompute.WriteGameState();



 
        clGetEventProfilingInfo(gameCompute.writeEvent, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
        clGetEventProfilingInfo(gameCompute.writeEvent, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
        nanoSeconds = time_end - time_start;
        ImGui::Text("CPU->GPU Transfer time is: %0.3f milliseconds", nanoSeconds / 1000000.0);


        ImGui::Text("TickIdx: %d", gameState->tickIdx);
        ImGui::End();


        gameGraphics.Swap();
        WaitMinTickTime(timerStartMs, gameNetworking.targetTickTimeMs);

        if (gameCompute.errorState)
            quit = true;
    }



    // Clean up
        
    return 0;


}



