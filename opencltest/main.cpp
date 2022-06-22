
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
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

#include "glm.hpp"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform2.hpp>
#include <gtx/string_cast.hpp>

#include "slikenet/peerinterface.h"


#include "GEShader.h"
#include "GEShaderProgram.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#define MAX_SOURCE_SIZE (0x100000)

#define CL_ERROR_CHECK(ret) if (ret != 0) {printf("ret at %d is %d\n", __LINE__, ret); fflush(stdout); return 1; }

//Screen dimension constants
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;
//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Loads individual image as texture
SDL_Texture* loadTexture(std::string path);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;


ImGuiContext* imguicontext;
bool init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {        
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        //Set texture filtering to linear
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
        {
            printf("Warning: Linear texture filtering not enabled!");
        }

        //Create window
        gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
        if (gWindow == NULL)
        {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        }


        SDL_GL_CreateContext(gWindow);
        //swap buffer at the monitors rate
        SDL_GL_SetSwapInterval(1);



        //GLEW is an OpenGL Loading Library used to reach GL functions
        //Sets all functions available
        
        glewInit();
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    imguicontext = ImGui::CreateContext();
 
    // Setup Dear ImGui style
    ImGui::StyleColorsLight();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImFont* font1 = io.Fonts->AddFontDefault();
    // Setup Platform/Renderer bindings
    // window is the SDL_Window*
    // context is the SDL_GLContext
    ImGui_ImplSDL2_InitForOpenGL(gWindow, imguicontext);
    ImGui_ImplOpenGL3_Init();



    return success;
}



void close()
{
    //Destroy window	
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gRenderer = NULL;

    //Quit SDL subsystems
    SDL_Quit();
}

int random(int min, int max) { return rand() % (max - min + 1) + min; }



int main(int argc, char* args[]) 
{

    //Start up SDL and create window
    if (!init())
    {
        printf("Failed to initialize!\n");
    }
    else
    {


        printf("started running\n");





        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLubyte* vendor = glGetString(GL_VENDOR);
        const GLubyte* version = glGetString(GL_VERSION);
        const GLubyte* glslVersion =
            glGetString(GL_SHADING_LANGUAGE_VERSION);

        GLint major, minor;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);

        printf("GL Vendor    : %s\n", vendor);
        printf("GL Renderer  : %s\n", renderer);
        printf("GL Version (string)  : %s\n", version);
        printf("GL Version (integer) : %d.%d\n", major, minor);
        printf("GLSL Version : %s\n", glslVersion);

        std::vector<GEShaderProgram*> shaderProgramList;
        std::vector<GEShader*> shaderList;

        GEShader* pVertShad = new GEShader(GL_VERTEX_SHADER, "vertPeep.shad");
        GEShader* pFragShad = new GEShader(GL_FRAGMENT_SHADER, "fragPeep.shad");
        shaderList.push_back(pVertShad);
        shaderList.push_back(pFragShad);

        //create programs to use those shaders.
        GEShaderProgram* pPeepShadProgram = new GEShaderProgram();
        pPeepShadProgram->AttachShader(pVertShad);
        pPeepShadProgram->AttachShader(pFragShad);

        shaderProgramList.push_back(pPeepShadProgram);


        pVertShad = new GEShader(GL_VERTEX_SHADER, "vertShader.shad");
        pFragShad = new GEShader(GL_FRAGMENT_SHADER, "fragShader.shad");
        shaderList.push_back(pVertShad);
        shaderList.push_back(pFragShad);

        //create programs to use those shaders.
        GEShaderProgram* pBasicShadProgram = new GEShaderProgram();
        pBasicShadProgram->AttachShader(pVertShad);
        pBasicShadProgram->AttachShader(pFragShad);

        shaderProgramList.push_back(pBasicShadProgram);


        glBindAttribLocation(pPeepShadProgram->ProgramID(), 0, "VertexPosition");
        glBindAttribLocation(pPeepShadProgram->ProgramID(), 1, "VertexColor");

        glBindAttribLocation(pBasicShadProgram->ProgramID(), 0, "VertexPosition");
        glBindAttribLocation(pBasicShadProgram->ProgramID(), 1, "VertexColor");







        pPeepShadProgram->Link();
        pBasicShadProgram->Link();


        // store instance data in an array buffer
        // --------------------------------------
        unsigned int instanceVBO;
        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, (sizeof(glm::vec2) + sizeof(glm::vec3)) * MAX_PEEPS, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float quadVertices[] = {
            // positions    
            -1.0f,  1.0f, 
             1.0f, -1.0f,  
            -1.0f, -1.0f,

            -1.0f,  1.0f, 
             1.0f, -1.0f,  
             1.0f,  1.0f
        };
        unsigned int quadVAO, quadVBO;
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        // also set instance data
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from a different vertex buffer
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) + sizeof(glm::vec3), (void*)0);//position
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) + sizeof(glm::vec3), (void*)sizeof(glm::vec2));//color
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.
        glVertexAttribDivisor(3, 1); // tell OpenGL this is an instanced vertex attribute.






        GameState* gameState = new GameState();
        for (int secx = 0; secx < SQRT_MAXSECTORS; secx++)
        {
            for (int secy = 0; secy < SQRT_MAXSECTORS; secy++)
            {
                gameState->sectors[secx][secy].xidx = secx;
                gameState->sectors[secx][secy].yidx = secy;
                gameState->sectors[secx][secy].lastPeep = NULL;
                gameState->sectors[secx][secy].lock = 0;
            }
        }
        for (int p = 0; p < MAX_PEEPS; p++)
        {
            gameState->peeps[p].map_x_Q15_16 = random(-1000, 1000)  << 16;
            gameState->peeps[p].map_y_Q15_16 = random(-1000, 1000)  << 16;

            gameState->peeps[p].xv_Q15_16 = random(-4, 4) << 16;
            gameState->peeps[p].yv_Q15_16 = random(-4, 4) << 16;
            gameState->peeps[p].netForcex_Q16 = 0;
            gameState->peeps[p].netForcey_Q16 = 0;
            gameState->peeps[p].minDistPeep = NULL;
            gameState->peeps[p].minDistPeep_Q16 = (1 << 31);
            gameState->peeps[p].mapSector = NULL;
            gameState->peeps[p].mapSector_pending = NULL;
            gameState->peeps[p].nextSectorPeep = NULL;
            gameState->peeps[p].prevSectorPeep = NULL;
            gameState->peeps[p].target_x_Q16 = random(-1000, 1000) << 16;
            gameState->peeps[p].target_y_Q16 = random(-1000, 1000) << 16;
            if (gameState->peeps[p].map_x_Q15_16>>16 < 0)
            {
                gameState->peeps[p].faction = 0;
            }
            else
            {
                gameState->peeps[p].faction = 1;
            }
        }



        // Load the update_kernel source code into the array source_str
        FILE* fp;
        char* source_str;
        size_t source_size;

        fp = fopen("vector_add_kernel.c", "r");
        if (!fp) {
            fprintf(stderr, "Failed to load update_kernel.\n");
            exit(1);
        }
        source_str = (char*)malloc(MAX_SOURCE_SIZE);
        source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
        fclose(fp);
        printf("update_kernel loading done\n");
        // Get platform and device information
        cl_device_id device_id = NULL;
        cl_uint ret_num_devices;
        cl_uint ret_num_platforms;


        cl_int ret = clGetPlatformIDs(0, NULL, &ret_num_platforms);
        printf("num platforms: %d", ret_num_platforms);


        cl_platform_id* platforms = NULL;
        platforms = (cl_platform_id*)malloc(ret_num_platforms * sizeof(cl_platform_id));

        ret = clGetPlatformIDs(ret_num_platforms, platforms, NULL);
        CL_ERROR_CHECK(ret)

        ret = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);
        CL_ERROR_CHECK(ret)

        // Create an OpenCL context
        cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
        CL_ERROR_CHECK(ret)




        // Create a command queue
        cl_command_queue command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret);
        CL_ERROR_CHECK(ret)

        // Create memory buffers on the device for each vector 
        //cl_mem peeps_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(Peep), nullptr, &ret);
        cl_mem gamestate_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(GameState), nullptr, &ret);
        CL_ERROR_CHECK(ret)


        printf("before building\n");
        // Create a program from the update_kernel source
        cl_program program = clCreateProgramWithSource(context, 1,
            (const char**)&source_str, (const size_t*)&source_size, &ret);
        CL_ERROR_CHECK(ret)

        // Build the program
        ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
        


        if (ret == CL_BUILD_PROGRAM_FAILURE) {
            // Determine the size of the log
            size_t log_size;
            clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

            // Allocate memory for the log
            char* log = (char*)malloc(log_size);

            // Get the log
            clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

            // Print the log
            printf("%s\n", log);
            return 1;
        }
        CL_ERROR_CHECK(ret)

        printf("program built\n");


        cl_ulong localMemSize;
        clGetDeviceInfo(device_id,
            CL_DEVICE_LOCAL_MEM_SIZE,
            sizeof(localMemSize),
            &localMemSize,
            NULL);

        printf("CL_DEVICE_LOCAL_MEM_SIZE: %u\n", localMemSize);


        cl_ulong globalMemSize;
        clGetDeviceInfo(device_id,
            CL_DEVICE_GLOBAL_MEM_SIZE,
            sizeof(globalMemSize),
            &globalMemSize,
            NULL);

        printf("CL_DEVICE_GLOBAL_MEM_SIZE: %u\n", globalMemSize);




        // Create the OpenCL update_kernel
        cl_kernel preupdate_kernel = clCreateKernel(program, "game_preupdate_1", &ret); CL_ERROR_CHECK(ret)
        cl_kernel preupdate_kernel_2 = clCreateKernel(program, "game_preupdate_2", &ret); CL_ERROR_CHECK(ret)
        cl_kernel update_kernel = clCreateKernel(program, "game_update", &ret); CL_ERROR_CHECK(ret)
        cl_kernel init_kernel = clCreateKernel(program, "game_init_single", &ret); CL_ERROR_CHECK(ret)

            
        // Set the arguments of the kernels
        ret = clSetKernelArg(init_kernel, 0, sizeof(cl_mem), (void*)&gamestate_mem_obj); CL_ERROR_CHECK(ret)
        ret = clSetKernelArg(preupdate_kernel, 0, sizeof(cl_mem), (void*)&gamestate_mem_obj); CL_ERROR_CHECK(ret)
        ret = clSetKernelArg(preupdate_kernel_2, 0, sizeof(cl_mem), (void*)&gamestate_mem_obj); CL_ERROR_CHECK(ret)
        ret = clSetKernelArg(update_kernel, 0, sizeof(cl_mem), (void*)&gamestate_mem_obj); CL_ERROR_CHECK(ret)


        //get stats
        cl_ulong usedLocalMemSize;
        clGetKernelWorkGroupInfo(update_kernel,
            device_id,
            CL_KERNEL_LOCAL_MEM_SIZE,
            sizeof(cl_ulong),
            &usedLocalMemSize,
            NULL);
        printf("CL_KERNEL_LOCAL_MEM_SIZE: %u\n", usedLocalMemSize);

        // Execute the OpenCL update_kernel on the list
        size_t SingleKernelWorkItems[] = { WORKGROUPSIZE };
        size_t SingleKernelWorkItemsPerWorkGroup[] = { WORKGROUPSIZE };


        size_t WorkItems[] = { TOTALWORKITEMS };
        size_t WorkItemsPerWorkGroup[] = { WORKGROUPSIZE};
        
        
        //Main loop flag
        bool quit = false;

        //Event handler
        SDL_Event e;

        int mousex, mousey;

        gameState->mapHeight = SCREEN_HEIGHT;
        gameState->mapWidth = SCREEN_WIDTH;
        gameState->frameIdx = 0;
        gameState->mousescroll = 0;


        ret = clEnqueueWriteBuffer(command_queue, gamestate_mem_obj, CL_TRUE, 0,
            sizeof(GameState), gameState, 0, NULL, NULL);
        CL_ERROR_CHECK(ret)


        cl_event initEvent;
        ret = clEnqueueNDRangeKernel(command_queue, init_kernel, 1, NULL,
            SingleKernelWorkItems, NULL, 0, NULL, &initEvent);
        CL_ERROR_CHECK(ret)
        
        clWaitForEvents(1, &initEvent);

        cl_event preUpdateEvent1;
        cl_event preUpdateEvent2;

        glm::vec2* worldPositions = new glm::vec2[MAX_PEEPS];
        glm::vec3* colors = new glm::vec3[MAX_PEEPS];
        while (!quit)
        {
            //Clear screen
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame(gWindow);
            ImGui::NewFrame();

            
            
            ret = clEnqueueNDRangeKernel(command_queue, preupdate_kernel, 1, NULL,
                SingleKernelWorkItems, NULL, 0, NULL, &preUpdateEvent1);
            CL_ERROR_CHECK(ret)

            ret = clEnqueueNDRangeKernel(command_queue, preupdate_kernel_2, 1, NULL,
                SingleKernelWorkItems, NULL, 1, &preUpdateEvent1, &preUpdateEvent2);
            CL_ERROR_CHECK(ret)


            clWaitForEvents(1, &preUpdateEvent2);
            cl_uint waitListCnt = 1;
            cl_event updateEvent;
            ret = clEnqueueNDRangeKernel(command_queue, update_kernel, 1, NULL,
                WorkItems, NULL, waitListCnt, &preUpdateEvent2, &updateEvent);
            CL_ERROR_CHECK(ret)



            cl_ulong time_start;
            cl_ulong time_end;

            clGetEventProfilingInfo(updateEvent, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
            clGetEventProfilingInfo(updateEvent, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);

            ImGui::Begin("Profiling");
            double nanoSeconds = time_end - time_start;
            ImGui::Text("update_kernel Execution time is: %0.3f milliseconds", nanoSeconds / 1000000.0);
            clGetEventProfilingInfo(preUpdateEvent1, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
            clGetEventProfilingInfo(preUpdateEvent2, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);

            nanoSeconds = time_end - time_start;
            ImGui::Text("preupdate_kernel Execution time is: %0.3f milliseconds", nanoSeconds / 1000000.0);
            


            ret = clFinish(command_queue);
            CL_ERROR_CHECK(ret)



            // Read the memory buffer C on the device to the local variable C
            cl_event readEvent;
            ret = clEnqueueReadBuffer(command_queue, gamestate_mem_obj, CL_TRUE, 0,
                sizeof(GameState), gameState, 0, NULL, &readEvent);
            CL_ERROR_CHECK(ret)

            ret = clFinish(command_queue);
            CL_ERROR_CHECK(ret)

            clGetEventProfilingInfo(readEvent, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
            clGetEventProfilingInfo(readEvent, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
            nanoSeconds = time_end - time_start;
            ImGui::Text("GPU->CPU Transfer time is: %0.3f milliseconds", nanoSeconds / 1000000.0);
            


            gameState->mousePrimaryPressed = 0;
            gameState->mousePrimaryReleased = 0;
            gameState->mouseSecondaryPressed = 0;
            gameState->mouseSecondaryReleased = 0;
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
                            gameState->mousescroll++;
                        
                    }
                    else if (e.wheel.y < 0) // scroll down
                    {
                       // if (gameState->viewScale > 1)
                            gameState->mousescroll--;
                    }
                }
                else if (e.type == SDL_MOUSEBUTTONDOWN)
                {
                    if (e.button.button == SDL_BUTTON_LEFT)
                    {
                        gameState->mousePrimaryDown = 1;
                        gameState->mousePrimaryPressed = 1;

                    }
                    else
                    {
                        gameState->mouseSecondaryDown = 1;
                        gameState->mouseSecondaryPressed = 1;
                    }
                }
                else if (e.type == SDL_MOUSEBUTTONUP)
                {
                    if (e.button.button == SDL_BUTTON_LEFT)
                    {
                        gameState->mousePrimaryDown = 0;
                        gameState->mousePrimaryReleased = 1;
                    }
                    else
                    {
                        gameState->mouseSecondaryDown = 0;
                        gameState->mouseSecondaryReleased = 1;
                    }
                }
                else if (SDL_WINDOWEVENT)
                {

                    switch (e.window.event)
                    {
                    case SDL_WINDOWEVENT_RESIZED:
                        int windowWidth = e.window.data1;
                        int windowHeight = e.window.data2;
                        glViewport(0, 0, windowWidth, windowHeight);
                        break;
                    }


                }
            }
            SDL_GetMouseState(&mousex, &mousey);
            gameState->mousex = mousex;
            gameState->mousey = mousey;
            if (gameState->mouseSecondaryPressed)
            {
                gameState->mouse_dragBeginx = mousex;
                gameState->mouse_dragBeginy = mousey;
                gameState->view_beginX = gameState->viewX;
                gameState->view_beginY = gameState->viewY;
            }
            if (gameState->mousePrimaryPressed)
            {
                gameState->mouse_dragBeginx = mousex;
                gameState->mouse_dragBeginy = mousey;
            }

            if (gameState->mouseSecondaryDown)
            {
                gameState->viewX = gameState->view_beginX + (gameState->mousex - gameState->mouse_dragBeginx);
                gameState->viewY = gameState->view_beginY + (gameState->mousey - gameState->mouse_dragBeginy);
            }
            gameState->frameIdx++;
            


            //render
            static float viewScale = 0.02f;
            
            ImGui::SliderFloat("Zoom", &viewScale, 0.001f,0.02f);

            glm::vec3 position = glm::vec3((2.0f*gameState->viewX) / SCREEN_WIDTH, ( - 2.0f * gameState->viewY) / SCREEN_HEIGHT, 0.0f);
            glm::mat4 view = glm::mat4(1.0f);   
            glm::vec3 scaleVec = glm::vec3(viewScale, viewScale , 1.0f);
            

            view = glm::scale(view, scaleVec);
            view = glm::translate(view,position/ viewScale);
            
            glm::mat4 mouseWorldPos(1.0f);
            glm::mat4 mouseWorldPosEnd(1.0f);
            glm::vec4 mouseScreenCoords = glm::vec4(2.0f * (float(mousex) / SCREEN_WIDTH) - 1.0, -2.0f * (float(mousey) / SCREEN_HEIGHT) + 1.0, 0.0f, 1.0f);
            glm::vec4 mouseBeginScreenCoords = glm::vec4(2.0f * (float(gameState->mouse_dragBeginx) / SCREEN_WIDTH) - 1.0, -2.0f * (float(gameState->mouse_dragBeginy) / SCREEN_HEIGHT) + 1.0, 0.0f, 1.0f);
            glm::vec4 worldMouseEnd = glm::inverse(view) * mouseScreenCoords;
            glm::vec4 worldMouseBegin = glm::inverse(view) * mouseBeginScreenCoords;


            if (gameState->mousePrimaryReleased)
            {

                float endx   = glm::max(worldMouseBegin.x, worldMouseEnd.x);
                float endy   = glm::min(worldMouseBegin.y, worldMouseEnd.y);
                float startx = glm::min(worldMouseBegin.x, worldMouseEnd.x);
                float starty = glm::max(worldMouseBegin.y, worldMouseEnd.y);

                gameState->action_DoSelect = 1;
                gameState->params_DoSelect_StartX_Q16 = cl_int(startx*(1<<16));
                gameState->params_DoSelect_StartY_Q16 = cl_int(starty * (1 << 16));
                gameState->params_DoSelect_EndX_Q16   = cl_int(endx * (1 << 16));
                gameState->params_DoSelect_EndY_Q16   = cl_int(endy * (1 << 16));
            }

            
            if (gameState->mouseSecondaryReleased)
            {
                gameState->action_CommandToLocation = 1;
                gameState->params_CommandToLocation_X_Q16 = cl_int(worldMouseEnd.x*(1<<16)) ;
                gameState->params_CommandToLocation_Y_Q16 = cl_int(worldMouseEnd.y * (1 << 16));
            }



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



            ImGui::Begin("Network");

            ImGui::Button("Start Server");

            ImGui::Button("Connect To Local Server");

            ImGui::End();


            




            pPeepShadProgram->Use();
            pPeepShadProgram->SetUniform_Mat4("WorldToScreenTransform", view);


                    
            for (int pi = 0; pi < MAX_PEEPS; pi++)
            {
                Peep* p = &gameState->peeps[pi];


                float factor = 0.6f;
                if (p->faction == 1)
                {
                    colors[pi].r = 0.0f;
                    colors[pi].g = 1.0f;
                    colors[pi].b = 1.0f;
                }
                else
                {
                    colors[pi].r = 1.0f;
                    colors[pi].g = 0.0f;
                    colors[pi].b = 1.0f;
                }

                if (p->selectedByClient)
                {
                    factor = 1.0f;
                }
                colors[pi] = colors[pi]*factor;
                
                float x = float(p->map_x_Q15_16) / float(1 << 16);
                float y = float(p->map_y_Q15_16) / float(1 << 16);

                float xv = p->xv_Q15_16 / float(1 << 16);
                float yv = p->yv_Q15_16 / float(1 << 16);

                float angle = atan2f(yv, xv) ;


                worldPositions[pi] = glm::vec2(x, y);


                glm::mat4 localMatrix = glm::mat4(1.0f);
                localMatrix = glm::translate(localMatrix, glm::vec3(x, y, 0));
                localMatrix = glm::rotate(localMatrix, angle * (180.0f / 3.1415f) - 90.0f, glm::vec3(0, 0, 1));

                glm::vec2 location2D = glm::vec2(x, y);
                glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
                int stride = (sizeof(glm::vec2) + sizeof(glm::vec3));
                glBufferSubData(GL_ARRAY_BUFFER, pi * stride, sizeof(glm::vec2), &location2D.x);
                glBufferSubData(GL_ARRAY_BUFFER, pi * stride + sizeof(glm::vec2), sizeof(glm::vec3), &colors[pi].r);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

            }


            //draw all peeps
            glBindVertexArray(quadVAO);
            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, MAX_PEEPS);
            glBindVertexArray(0);

            //draw mouse
            {
               
                pBasicShadProgram->Use();


                pBasicShadProgram->SetUniform_Mat4("WorldToScreenTransform", glm::mat4(1.0f));


                glm::mat4 drawingTransform(1.0f);
                pBasicShadProgram->SetUniform_Mat4("LocalTransform", drawingTransform);
                pBasicShadProgram->SetUniform_Vec3("OverallColor", glm::vec3(1.0f, 1.0f, 1.0f));


                float mouseSelectVerts[] = {
                    //positions    
                   mouseBeginScreenCoords.x,  mouseBeginScreenCoords.y,
                   mouseScreenCoords.x, mouseBeginScreenCoords.y,
                   mouseScreenCoords.x , mouseScreenCoords.y ,
                   mouseBeginScreenCoords.x, mouseScreenCoords.y
                };

                unsigned int mouseVAO, mouseVBO;
                glGenVertexArrays(1, &mouseVAO);
                glGenBuffers(1, &mouseVBO);
                glBindVertexArray(mouseVAO);
                glBindBuffer(GL_ARRAY_BUFFER, mouseVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(mouseSelectVerts), mouseSelectVerts, GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

                if (gameState->mousePrimaryDown)
                    glDrawArrays(GL_LINE_LOOP, 0, 4);

                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
            }











            cl_event writeEvent;
            ret = clEnqueueWriteBuffer(command_queue, gamestate_mem_obj, CL_TRUE, 0,
                sizeof(GameState), gameState, 0, NULL, &writeEvent);
            CL_ERROR_CHECK(ret)

            clGetEventProfilingInfo(writeEvent, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
            clGetEventProfilingInfo(writeEvent, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
            nanoSeconds = time_end - time_start;
            ImGui::Text("CPU->GPU Transfer time is: %0.3f milliseconds", nanoSeconds / 1000000.0);


            ImGui::Text("FrameIdx: %d", gameState->frameIdx);
            ImGui::End();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            //Update screen
            SDL_GL_SwapWindow(gWindow);

            if (gameState->frameIdx == 500)
            {
              //  while (true) {}
            }
        }

        //cleanup
        for (auto* s : shaderProgramList)
            delete s;
        for (auto* s : shaderList)
            delete s;

        delete[] worldPositions;
        delete[] colors;


        // Clean up
        ret = clFlush(command_queue);
        ret = clFinish(command_queue);
        ret = clReleaseKernel(update_kernel);
        ret = clReleaseProgram(program);
        ret = clReleaseMemObject(gamestate_mem_obj);
        ret = clReleaseCommandQueue(command_queue);
        ret = clReleaseContext(context);
        free(gameState);

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        return 0;
    }



}



