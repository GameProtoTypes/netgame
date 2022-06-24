#include "GameGPUCompute.h"


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>


#include "glew.h"

GameGPUCompute::GameGPUCompute(GameState* gameState)
{

    // Load the update_kernel source code into the array source_str
    FILE* fp;
    char* source_str;
    size_t source_size;

    fp = fopen("vector_add_kernel.c", "r");
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


    cl_int ret = clGetPlatformIDs(0, NULL, &ret_num_platforms);
    printf("num platforms: %d", ret_num_platforms);


    cl_platform_id* platforms = NULL;
    platforms = (cl_platform_id*)malloc(ret_num_platforms * sizeof(cl_platform_id));

    ret = clGetPlatformIDs(ret_num_platforms, platforms, NULL);
    CL_ERROR_CHECK(ret)

        ret = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);
    CL_ERROR_CHECK(ret)

        // Create an OpenCL context
         context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    CL_ERROR_CHECK(ret)




        // Create a command queue
        command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret);
    CL_ERROR_CHECK(ret)

        // Create memory buffers on the device for each vector 
        //cl_mem peeps_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(Peep), nullptr, &ret);
         gamestate_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(GameState), nullptr, &ret);
    CL_ERROR_CHECK(ret)


        printf("before building\n");
    // Create a program from the update_kernel source
     program = clCreateProgramWithSource(context, 1,
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
     preupdate_kernel = clCreateKernel(program, "game_preupdate_1", &ret); CL_ERROR_CHECK(ret)
     preupdate_kernel_2 = clCreateKernel(program, "game_preupdate_2", &ret); CL_ERROR_CHECK(ret)
     update_kernel = clCreateKernel(program, "game_update", &ret); CL_ERROR_CHECK(ret)
     init_kernel = clCreateKernel(program, "game_init_single", &ret); CL_ERROR_CHECK(ret)


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


    this->gameState = gameState;

    RunInitCompute();
}

GameGPUCompute::~GameGPUCompute()
{
    cl_int ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(update_kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(gamestate_mem_obj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);
}

void GameGPUCompute::RunInitCompute()
{
    cl_int ret = clEnqueueWriteBuffer(command_queue, gamestate_mem_obj, CL_TRUE, 0,
        sizeof(GameState), gameState, 0, NULL, NULL);
    CL_ERROR_CHECK(ret)



    ret = clEnqueueNDRangeKernel(command_queue, init_kernel, 1, NULL,
        SingleKernelWorkItems, NULL, 0, NULL, &initEvent);
    CL_ERROR_CHECK(ret)

    clWaitForEvents(1, &initEvent);

}

void GameGPUCompute::Stage1()
{

    cl_int ret = clEnqueueNDRangeKernel(command_queue, preupdate_kernel, 1, NULL,
        WorkItems, NULL, 0, NULL, &preUpdateEvent1);
    CL_ERROR_CHECK(ret)

        ret = clEnqueueNDRangeKernel(command_queue, preupdate_kernel_2, 1, NULL,
            WorkItems, NULL, 1, &preUpdateEvent1, &preUpdateEvent2);
    CL_ERROR_CHECK(ret)


        clWaitForEvents(1, &preUpdateEvent2);
    cl_uint waitListCnt = 1;

    ret = clEnqueueNDRangeKernel(command_queue, update_kernel, 1, NULL,
        WorkItems, NULL, waitListCnt, &preUpdateEvent2, &updateEvent);
    CL_ERROR_CHECK(ret)



    ret = clFinish(command_queue);
    CL_ERROR_CHECK(ret)



    // Read the memory buffer C on the device to the local variable C

    ret = clEnqueueReadBuffer(command_queue, gamestate_mem_obj, CL_TRUE, 0,
        sizeof(GameState), gameState, 0, NULL, &readEvent);
    CL_ERROR_CHECK(ret)

        ret = clFinish(command_queue);
    CL_ERROR_CHECK(ret)



}

void GameGPUCompute::WriteGameState()
{
    
    cl_int ret = clEnqueueWriteBuffer(command_queue, gamestate_mem_obj, CL_TRUE, 0,
        sizeof(GameState), gameState, 0, NULL, &writeEvent);
    CL_ERROR_CHECK(ret)
}
