#include "GameGPUCompute.h"


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>
#include <typeinfo>
#include <variant>

#include "glew.h"

#include "GameGraphics.h"





GameGPUCompute::GameGPUCompute(std::shared_ptr<GameState> gameState, std::shared_ptr<GameStateB> gameStateB, GameGraphics* graphics)
{

    this->gameState = gameState;
    this->gameStateB = gameStateB;
    this->graphics = graphics;


}

GameGPUCompute::~GameGPUCompute()
{




    cl_int ret = clFlush(command_queue); CL_HOST_ERROR_CHECK(ret)
    ret = clFinish(command_queue); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseKernel(update_kernel); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseKernel(post_update_single_kernel); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseKernel(init_kernel); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseKernel(preupdate_kernel); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseKernel(preupdate_kernel_2); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseKernel(action_kernel); CL_HOST_ERROR_CHECK(ret)

    ret = clReleaseProgram(program); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseMemObject(gamestate_mem_obj); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseMemObject(gamestateB_mem_obj); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseMemObject(graphics_peeps_mem_obj); CL_HOST_ERROR_CHECK(ret)


    ret = clReleaseCommandQueue(command_queue); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseContext(context); CL_HOST_ERROR_CHECK(ret)
}


void GameGPUCompute::AddCompileDefinition(std::string name, GPUCompileVariant val)
{
    compileDefinitions.push_back({ name, val });
}

void GameGPUCompute::RunInitCompute()
{

    // Load the update_kernel source code into the array source_str
    FILE* fp;
    char* source_str;
    size_t source_size;

    fp = fopen("clGame.c", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load update_kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(GAMECOMPUTE_MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, GAMECOMPUTE_MAX_SOURCE_SIZE, fp);
    fclose(fp);
    printf("update_kernel loading done\n");
    // Get platform and device information
    cl_device_id device_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;


    ret = clGetPlatformIDs(0, NULL, &ret_num_platforms);
    printf("num cl platforms: %d\n", ret_num_platforms);


    cl_platform_id* platforms = new cl_platform_id[ret_num_platforms];

    ret = clGetPlatformIDs(ret_num_platforms, platforms, NULL);
    CL_HOST_ERROR_CHECK(ret)

        ret = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);
    CL_HOST_ERROR_CHECK(ret)

        // Create an OpenCL context
        std::cout << SDL_GL_MakeCurrent(graphics->gWindow, graphics->sdlGLContext);
    cl_context_properties props[] =
    {
        CL_GL_CONTEXT_KHR, (cl_context_properties)(graphics->sdlGLContext),
        CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
        CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[0],
        0
    };
    context = clCreateContext(props, 1, &device_id, NULL, NULL, &ret);
    CL_HOST_ERROR_CHECK(ret)



        // Create a command queue
        command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret);
    CL_HOST_ERROR_CHECK(ret)

        // Create memory buffers on the device
        gamestate_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(GameState), nullptr, &ret);
    CL_HOST_ERROR_CHECK(ret)


        gamestateB_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(GameStateB), nullptr, &ret);
    CL_HOST_ERROR_CHECK(ret)




        graphics_peeps_mem_obj = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, graphics->peepInstanceVBO, &ret);
    CL_HOST_ERROR_CHECK(ret)

        graphics_mapTileVBO_mem_obj = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, graphics->mapTileVBO, &ret);
    CL_HOST_ERROR_CHECK(ret)

        graphics_mapTileAttrVBO_mem_obj = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, graphics->mapTileAttrVBO, &ret);
    CL_HOST_ERROR_CHECK(ret)

    graphicsObjects.push_back(graphics_peeps_mem_obj);
    graphicsObjects.push_back(graphics_mapTileVBO_mem_obj);
    graphicsObjects.push_back(graphics_mapTileAttrVBO_mem_obj);


        printf("Building CL Programs...\n");
    // Create a program from the update_kernel source
    program = clCreateProgramWithSource(context, 1,
        (const char**)&source_str, (const size_t*)&source_size, &ret);
    CL_HOST_ERROR_CHECK(ret)

    //make preprocessor defs
    std::string defs = buildPreProcessorString();

    // Build the program
    ret = clBuildProgram(program, 1, &device_id, defs.c_str(), NULL, NULL);



    if (ret != CL_SUCCESS) {
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allocate memory for the log
        char* log = (char*)malloc(log_size);

        // Get the log
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Print the log
        printf("%s\n", log);

        delete log;
    }
    CL_HOST_ERROR_CHECK(ret)

    printf("programs built\n");


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
        preupdate_kernel = clCreateKernel(program, "game_preupdate_1", &ret); CL_HOST_ERROR_CHECK(ret)
        preupdate_kernel_2 = clCreateKernel(program, "game_preupdate_2", &ret); CL_HOST_ERROR_CHECK(ret)
        update_kernel = clCreateKernel(program, "game_update", &ret); CL_HOST_ERROR_CHECK(ret)
        action_kernel = clCreateKernel(program, "game_apply_actions", &ret); CL_HOST_ERROR_CHECK(ret)
        init_kernel = clCreateKernel(program, "game_init_single", &ret); CL_HOST_ERROR_CHECK(ret)
        post_update_single_kernel = clCreateKernel(program, "game_post_update_single", &ret); CL_HOST_ERROR_CHECK(ret)



        kernels.push_back(preupdate_kernel);
        kernels.push_back(preupdate_kernel_2);
        kernels.push_back(update_kernel);
        kernels.push_back(action_kernel);
        kernels.push_back(init_kernel);
        kernels.push_back(post_update_single_kernel);

        // Set the arguments of the kernels
        for (cl_kernel k : kernels)
        {
            ret = clSetKernelArg(k, 0, sizeof(cl_mem), (void*)&gamestate_mem_obj); CL_HOST_ERROR_CHECK(ret)
            ret = clSetKernelArg(k, 1, sizeof(cl_mem), (void*)&gamestateB_mem_obj); CL_HOST_ERROR_CHECK(ret)
            ret = clSetKernelArg(k, 2, sizeof(cl_mem), (void*)&graphics_peeps_mem_obj); CL_HOST_ERROR_CHECK(ret)
            ret = clSetKernelArg(k, 3, sizeof(cl_mem), (void*)&graphics_mapTileVBO_mem_obj); CL_HOST_ERROR_CHECK(ret)
            ret = clSetKernelArg(k, 4, sizeof(cl_mem), (void*)&graphics_mapTileAttrVBO_mem_obj); CL_HOST_ERROR_CHECK(ret)
        }
        


        //get stats
        cl_ulong usedLocalMemSize;
    clGetKernelWorkGroupInfo(update_kernel,
        device_id,
        CL_KERNEL_LOCAL_MEM_SIZE,
        sizeof(cl_ulong),
        &usedLocalMemSize,
        NULL);
    printf("CL_KERNEL_LOCAL_MEM_SIZE: %u\n", usedLocalMemSize);




    ret = clEnqueueWriteBuffer(command_queue, gamestate_mem_obj, CL_TRUE, 0,
        sizeof(GameState), gameState.get(), 0, NULL, NULL);
    CL_HOST_ERROR_CHECK(ret)

    ret = clEnqueueWriteBuffer(command_queue, gamestateB_mem_obj, CL_TRUE, 0,
        sizeof(GameStateB), gameState.get(), 0, NULL, NULL);
    CL_HOST_ERROR_CHECK(ret)



    AquireAllGraphicsObjects();

        ret = clEnqueueNDRangeKernel(command_queue, init_kernel, 1, NULL,
            SingleKernelWorkItems, NULL, 0, NULL, &initEvent);
        CL_HOST_ERROR_CHECK(ret)


    ReleaseAllGraphicsObjects();
    clWaitForEvents(1, &initEvent);
    ReadFullGameState();
}

void GameGPUCompute::Stage1()
{
    if (gameStateB->pauseState != 0)
        return;


    AquireAllGraphicsObjects();

    ret = clFinish(command_queue);
    CL_HOST_ERROR_CHECK(ret)

     ret = clEnqueueNDRangeKernel(command_queue, action_kernel, 1, NULL,
        SingleKernelWorkItems, NULL, 0, NULL, &actionEvent);
    CL_HOST_ERROR_CHECK(ret)

     ret = clFinish(command_queue);
    CL_HOST_ERROR_CHECK(ret)

     ret = clEnqueueNDRangeKernel(command_queue, preupdate_kernel, 1, NULL,
        WorkItems, NULL, 1, &actionEvent, &preUpdateEvent1);
    CL_HOST_ERROR_CHECK(ret)

    ret = clFinish(command_queue);
    CL_HOST_ERROR_CHECK(ret)

    ret = clEnqueueNDRangeKernel(command_queue, preupdate_kernel_2, 1, NULL,
        WorkItems, NULL, 1, &preUpdateEvent1, &preUpdateEvent2);
    CL_HOST_ERROR_CHECK(ret)



    ret = clEnqueueNDRangeKernel(command_queue, update_kernel, 1, NULL,
        WorkItems, NULL, 1, &preUpdateEvent2, &updateEvent);
    CL_HOST_ERROR_CHECK(ret)
    

    ret = clEnqueueNDRangeKernel(command_queue, post_update_single_kernel, 1, NULL,
        SingleKernelWorkItems, NULL, 1, &updateEvent, &postupdateEvent);
    CL_HOST_ERROR_CHECK(ret)


    ret = clFinish(command_queue);
    CL_HOST_ERROR_CHECK(ret)
    

    ReleaseAllGraphicsObjects();

    ret = clFinish(command_queue);
    CL_HOST_ERROR_CHECK(ret)



    // Read the memory buffer C on the device to the local variable C
    ret = clEnqueueReadBuffer(command_queue, gamestateB_mem_obj, CL_TRUE, 0,
        sizeof(GameStateB), gameStateB.get(), 0, NULL, &readEvent);
    CL_HOST_ERROR_CHECK(ret)

    ret = clFinish(command_queue);
    CL_HOST_ERROR_CHECK(ret)



}

void GameGPUCompute::ReadFullGameState()
{
    // Read the memory buffer C on the device to the local variable C
     ret = clEnqueueReadBuffer(command_queue, gamestate_mem_obj, CL_TRUE, 0,
        sizeof(GameState), gameState.get(), 0, NULL, &readEvent);
    CL_HOST_ERROR_CHECK(ret)

    // Read the memory buffer C on the device to the local variable C
    ret = clEnqueueReadBuffer(command_queue, gamestateB_mem_obj, CL_TRUE, 0,
        sizeof(GameStateB), gameStateB.get(), 0, NULL, &readEvent);
    CL_HOST_ERROR_CHECK(ret)
    
    ret = clFinish(command_queue);
    CL_HOST_ERROR_CHECK(ret)
}

void GameGPUCompute::WriteFullGameState()
{
    ret = clEnqueueWriteBuffer(command_queue, gamestate_mem_obj, CL_TRUE, 0,
        sizeof(GameState), gameState.get(), 0, NULL, &writeEvent);
    CL_HOST_ERROR_CHECK(ret)

    WriteGameStateB();






    ret = clFinish(command_queue);
    CL_HOST_ERROR_CHECK(ret)
}

void GameGPUCompute::WriteGameStateB()
{
    ret = clEnqueueWriteBuffer(command_queue, gamestateB_mem_obj, CL_TRUE, 0,
        sizeof(GameStateB), gameStateB.get(), 0, NULL, &writeEvent);
    CL_HOST_ERROR_CHECK(ret)
}

std::string GameGPUCompute::buildPreProcessorString()
{
    std::string str;
    for (auto pair : compileDefinitions)
    {
        str += std::string("-D ") + pair.first + std::string("=") + compileVariantString(pair.second);
    }

    return str;
}

std::string GameGPUCompute::compileVariantString(GPUCompileVariant variant)
{

    std::stringstream stream;

    try
    {
        stream << std::get<std::string>(variant);
        return stream.str();
    }
    catch(const std::exception& e) {};

    try
    {
        stream << std::get<int>(variant);
        return stream.str();
    }
    catch (const std::exception& e) {};

    try
    {
        stream << std::get<float>(variant);
        return stream.str();
    }
    catch (const std::exception& e) {};
    

}

void GameGPUCompute::AquireAllGraphicsObjects()
{
    for (auto obj : graphicsObjects)
    {
        ret = clEnqueueAcquireGLObjects(command_queue, 1, &obj, 0, 0, 0);
        CL_HOST_ERROR_CHECK(ret)
    }
    //ret = clFinish(command_queue);
    //CL_HOST_ERROR_CHECK(ret)
}

void GameGPUCompute::ReleaseAllGraphicsObjects()
{
    for (auto obj : graphicsObjects)
    {
        ret = clEnqueueReleaseGLObjects(command_queue, 1, &obj, 0, 0, 0);
        CL_HOST_ERROR_CHECK(ret)
    }
    //ret = clFinish(command_queue);
    //CL_HOST_ERROR_CHECK(ret)
}
