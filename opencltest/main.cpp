
#include <stdio.h>
#include <stdlib.h>
#include <string>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif


#include <SDL.h>

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
        else
        {
            //Create renderer for window
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
            if (gRenderer == NULL)
            {
                printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                //Initialize renderer color
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

               
            }
        }
    }

    return success;
}



void close()
{
    //Destroy window	
    SDL_DestroyRenderer(gRenderer);
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
        // Create the OpenCL kernel
        cl_kernel kernel = clCreateKernel(program, "game_update", &ret);

        // Set the arguments of the kernel
        ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&gamestate_mem_obj);
        // Execute the OpenCL kernel on the list
        size_t WorkItems[] = { SQRT_SECTORS_PER_MAP ,SQRT_SECTORS_PER_MAP };
        size_t WorkItemsPerWorkGroup[] = { 16, 16 };
        

        //Main loop flag
        bool quit = false;

        //Event handler
        SDL_Event e;

        int mousex, mousey;

        gameState->screenHeight = SCREEN_HEIGHT;
        gameState->screenWidth = SCREEN_WIDTH;
        gameState->frameIdx = 0;

        



        while (!quit)
        {


            //printf("Simulating..\n");
            ret = clEnqueueWriteBuffer(command_queue, gamestate_mem_obj, CL_TRUE, 0,
                sizeof(GameState), gameState, 0, NULL, NULL);
            CL_ERROR_CHECK(ret)

            printf("Starting kernal\n");

            ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL,
                WorkItems, WorkItemsPerWorkGroup, 0, NULL, NULL);
            CL_ERROR_CHECK(ret)

            ret = clFinish(command_queue);
            CL_ERROR_CHECK(ret)
            printf("kernal done.\n");

            // Read the memory buffer C on the device to the local variable C
            ret = clEnqueueReadBuffer(command_queue, gamestate_mem_obj, CL_TRUE, 0,
                sizeof(GameState), gameState, 0, NULL, NULL);
            CL_ERROR_CHECK(ret)

            ret = clFinish(command_queue);
            CL_ERROR_CHECK(ret)


            //Handle events on queue
            while (SDL_PollEvent(&e) != 0)
            {
                //User requests quit
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }
            }
            SDL_GetMouseState(&mousex, &mousey);
            gameState->mousex = mousex;
            gameState->mousey = mousey;
            

            //Clear screen
            SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xFF);
            SDL_RenderClear(gRenderer);


            for (int sx = 0; sx < SQRT_SECTORS_PER_MAP; sx++)
            {
                for (int sy = 0; sy < SQRT_SECTORS_PER_MAP; sy++)
                {
                    SDL_Rect rect;
                    rect.x = sx * SQRT_CELLS_PER_MAP_SECTOR;
                    rect.y = sy * SQRT_CELLS_PER_MAP_SECTOR;
                    rect.w = SQRT_CELLS_PER_MAP_SECTOR;
                    rect.h = SQRT_CELLS_PER_MAP_SECTOR;
                    SDL_SetRenderDrawColor(gRenderer, sx+50, sy+50, 50, 0xFF);
                    SDL_RenderDrawRect(gRenderer, &rect);
                }
            }

            for (int pi = 0; pi < MAX_PEEPS; pi++) {
                Peep* p = &gameState->peeps[pi];
                if (p->valid)
                {
                    SDL_RenderDrawPoint(gRenderer, p->map_xi, p->map_yi);
                }

            }

                
           

            //Update screen
            SDL_RenderPresent(gRenderer);

            gameState->frameIdx++;
            printf("CPU frameIdx: %d\n", gameState->frameIdx);
            if (gameState->frameIdx == 1000)
            {
                while (true) {}
            }
            SDL_Delay(10);
            
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