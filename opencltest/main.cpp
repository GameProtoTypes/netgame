
#include <stdio.h>
#include <stdlib.h>
#include <string>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif


#include <SDL.h>
#include "glew.h"
#include "peep.h"

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

        // Create the two input vectors
        int i;


        GameState* gameState = new GameState();
        for (int secx = 0; secx < SQRT_MAXSECTORS; secx++)
        {
            for (int secy = 0; secy < SQRT_MAXSECTORS; secy++)
            {
                gameState->sectors[secx][secy].xidx = secx;
                gameState->sectors[secx][secy].yidx = secy;
                gameState->sectors[secx][secy].lastPeep = NULL;
                gameState->sectors[secx][secy].nextPeepIdx = 0;
            }
        }
        for (int p = 0; p < MAX_PEEPS; p++)
        {
            gameState->peeps[p].map_x_Q15_16 = random(-1000, 1000)  << 16;
            gameState->peeps[p].map_y_Q15_16 = random(-1000, 1000)  << 16;

            gameState->peeps[p].xv_Q15_16 = random(-4, 4)  << 16;
            gameState->peeps[p].yv_Q15_16 = random(-4, 4)  << 16;
            gameState->peeps[p].netForcex_Q16 = 0;
            gameState->peeps[p].netForcey_Q16 = 0;
            gameState->peeps[p].mapSector = NULL;
            gameState->peeps[p].mapSectorListIdx = 0;
            gameState->peeps[p].nextSectorPeep = NULL;
            gameState->peeps[p].prevSectorPeep = NULL;

            if (gameState->peeps[p].map_x_Q15_16>>16 < SCREEN_WIDTH / 2)
            {
                gameState->peeps[p].faction = 0;
            }
            else
            {
                gameState->peeps[p].faction = 1;
            }

        }



        // Load the kernel source code into the array source_str
        FILE* fp;
        char* source_str;
        size_t source_size;

        fp = fopen("vector_add_kernel.c", "r");
        if (!fp) {
            fprintf(stderr, "Failed to load kernel.\n");
            exit(1);
        }
        source_str = (char*)malloc(MAX_SOURCE_SIZE);
        source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
        fclose(fp);
        printf("kernel loading done\n");
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

        ret = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 1,
            &device_id, &ret_num_devices);
        CL_ERROR_CHECK(ret)
        // Create an OpenCL context
        cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
        CL_ERROR_CHECK(ret)







        // Create a command queue
        cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
        CL_ERROR_CHECK(ret)

        // Create memory buffers on the device for each vector 
        //cl_mem peeps_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(Peep), nullptr, &ret);
        cl_mem gamestate_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(GameState), nullptr, &ret);
        CL_ERROR_CHECK(ret)


        printf("before building\n");
        // Create a program from the kernel source
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




        // Create the OpenCL kernel
        cl_kernel cellularize_kernel = clCreateKernel(program, "game_preupdate_single", &ret); CL_ERROR_CHECK(ret)
        cl_kernel kernel = clCreateKernel(program, "game_update", &ret); CL_ERROR_CHECK(ret)

            
        // Set the arguments of the kernels
        ret = clSetKernelArg(cellularize_kernel, 0, sizeof(cl_mem), (void*)&gamestate_mem_obj); CL_ERROR_CHECK(ret)
        ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&gamestate_mem_obj); CL_ERROR_CHECK(ret)
       // ret = clSetKernelArg(kernel, 1, SQRT_SECTORS_PER_MAP * SQRT_SECTORS_PER_MAP *sizeof(MapSector), NULL); CL_ERROR_CHECK(ret)

        //get stats
        cl_ulong usedLocalMemSize;
        clGetKernelWorkGroupInfo(kernel,
            device_id,
            CL_KERNEL_LOCAL_MEM_SIZE,
            sizeof(cl_ulong),
            &usedLocalMemSize,
            NULL);
        printf("CL_KERNEL_LOCAL_MEM_SIZE: %u\n", usedLocalMemSize);

        // Execute the OpenCL kernel on the list
        size_t InitWorkItems[] = { 1 };
        size_t InitWorkItemsPerWorkGroup[] = { 1 };


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

        cl_event initEvent;
        while (!quit)
        {

            
            ret = clEnqueueWriteBuffer(command_queue, gamestate_mem_obj, CL_TRUE, 0,
                sizeof(GameState), gameState, 0, NULL, NULL);
            CL_ERROR_CHECK(ret)

            
   
            ret = clEnqueueNDRangeKernel(command_queue, cellularize_kernel, 1, NULL,
                InitWorkItems, InitWorkItemsPerWorkGroup, 0, NULL, &initEvent);
            CL_ERROR_CHECK(ret)

    
            cl_uint waitListCnt = 1;
            ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
                WorkItems, NULL, waitListCnt, &initEvent, NULL);
            CL_ERROR_CHECK(ret)

            ret = clFinish(command_queue);
            CL_ERROR_CHECK(ret)
            //printf("kernal done.\n");

            // Read the memory buffer C on the device to the local variable C
            ret = clEnqueueReadBuffer(command_queue, gamestate_mem_obj, CL_TRUE, 0,
                sizeof(GameState), gameState, 0, NULL, NULL);
            CL_ERROR_CHECK(ret)

            ret = clFinish(command_queue);
            CL_ERROR_CHECK(ret)


            
            gameState->mousePrimaryPressed = 0;
            gameState->mousePrimaryReleased = 0;
            gameState->mouseSecondaryPressed = 0;
            gameState->mouseSecondaryReleased = 0;
            //Handle events on queue
            while (SDL_PollEvent(&e) != 0)
            {
                //User requests quit
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }
                else if (e.type == SDL_MOUSEWHEEL)
                {
                    if (e.wheel.y > 0) // scroll up
                    {
                        if (gameState->viewScale >= 1 && gameState->viewScale < 15)
                            gameState->mousescroll++;
                        
                    }
                    else if (e.wheel.y < 0) // scroll down
                    {
                        if (gameState->viewScale > 1)
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
            
            if (gameState->mouseSecondaryDown)
            {
                gameState->viewX = gameState->view_beginX + (gameState->mousex - gameState->mouse_dragBeginx)/ gameState->viewScale;
                gameState->viewY = gameState->view_beginY + (gameState->mousey - gameState->mouse_dragBeginy)/ gameState->viewScale;
            }
            
            

            //Clear screen
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            glPushMatrix();

            gameState->viewScale = 20.0f + gameState->mousescroll;
            if (gameState->viewScale < 1)
                gameState->viewScale = 1;

             glScalef(gameState->viewScale,
                 gameState->viewScale, 1.0f);
                       glTranslatef(2*gameState->viewX /float(SCREEN_WIDTH) , -2*gameState->viewY / float(SCREEN_HEIGHT) , 0.0f);
   
            
            //for (int sx = 0; sx < SQRT_SECTORS_PER_MAP; sx++)
            //{
            //    for (int sy = 0; sy < SQRT_SECTORS_PER_MAP; sy++)
            //    {
            //        SDL_Rect rect;
            //        rect.x = sx * SQRT_CELLS_PER_MAP_SECTOR;
            //        rect.y = sy * SQRT_CELLS_PER_MAP_SECTOR;
            //        rect.w = SQRT_CELLS_PER_MAP_SECTOR;
            //        rect.h = SQRT_CELLS_PER_MAP_SECTOR;
            //        SDL_SetRenderDrawColor(gRenderer, sx+50, sy+50, 50, 0xFF);
            //        SDL_RenderDrawRect(gRenderer, &rect);
            //    }
            //}
            
            for (int pi = 0; pi < MAX_PEEPS; pi++) 
            {
                Peep* p = &gameState->peeps[pi];


                float vertices[] = {
                    -0.001f, -0.001f,
                    0.001f, -0.001f,
                    0.001f,  0.001f
                };

                float colors[] = {
                     0.0f, 1.0f, 1.0f, // red
                     0.0f, 1.0f, 1.0f,
                     0.0f, 1.0f, 1.0f,
                };

                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(2, GL_FLOAT, 0, vertices);

                glEnableClientState(GL_COLOR_ARRAY);
                glColorPointer(3, GL_FLOAT, 0, colors);

                glPushMatrix();
                float x = float(p->map_x_Q15_16) / float(1 << 16);
                float y = float(p->map_y_Q15_16) / float(1 << 16);

                glTranslatef(x/float(SCREEN_WIDTH)-0.5, y/float(SCREEN_HEIGHT)-0.5, 0);
                glDrawArrays(GL_TRIANGLES, 0, 3);
                glPopMatrix();


            }
   
            glPopMatrix();

            //Update screen
            SDL_GL_SwapWindow(gWindow);

            gameState->frameIdx++;
            printf("CPU frameIdx: %d\n", gameState->frameIdx);
            if (gameState->frameIdx == 500)
            {
               // while (true) {}
            }
        }



        // Clean up
        ret = clFlush(command_queue);
        ret = clFinish(command_queue);
        ret = clReleaseKernel(kernel);
        ret = clReleaseProgram(program);
        ret = clReleaseMemObject(gamestate_mem_obj);
        ret = clReleaseCommandQueue(command_queue);
        ret = clReleaseContext(context);
        free(gameState);
        return 0;
    }



}