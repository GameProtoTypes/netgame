#include "GameGPUCompute.h"

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <variant>

#include <chrono>
#include <filesystem>
#include <format>


 



#include "glew.h"
#include "GameGraphics.h"


GameGPUCompute::GameGPUCompute()
{



}

GameGPUCompute::~GameGPUCompute()
{
    cl_int ret = clFlush(command_queue); CL_HOST_ERROR_CHECK(ret)
    ret = clFinish(command_queue); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseKernel(update_kernel); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseKernel(post_update_kernel); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseKernel(init_kernel); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseKernel(preupdate_kernel); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseKernel(preupdate_kernel_2); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseKernel(action_kernel); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseKernel(game_updatepre1_kernel); CL_HOST_ERROR_CHECK(ret)

    ret = clReleaseProgram(gameProgram); CL_HOST_ERROR_CHECK(ret)
    ret = clReleaseMemObject(staticData_mem_obj); CL_HOST_ERROR_CHECK(ret)
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
void GameGPUCompute::BuildKernelRunSizes()
{
    if(vendor == NVIDIA)
        warpSize = 32;
    else if(vendor == AMD)
        warpSize = 64;

    GameUpdateWorkItems = warpSize*numComputeUnits*512;
    WorkItems[0] = static_cast<size_t>(GameUpdateWorkItems) ;
    WorkItemsInitMulti[0] =  0 ;
    WorkItems1Warp[0] =  warpSize ;
    SingleKernelWorkItems[0] =  1 ;
    SingleKernelWorkItemsPerWorkGroup[0] = 1 ;
}

void GameGPUCompute::AddCLSource(std::string path)
{

    


    clSourcePaths.push_back(path);
    uint64_t size = std::filesystem::file_size(path);
    
    std::ifstream stream;
    stream.open(path, std::ios::binary);
    uint64_t chsum = 0;
    std::vector<char> data;
    int i = 0;
    while(!stream.eof())
    {
        char byte;
        stream.read(&byte, 1);
        
        data.push_back(byte);
    }
    printf("size: %d\n", data.size());

    for(uint64_t i = 0; i < data.size(); i++)
        chsum += data[i];
    
    clSources.push_back(data);
    


    std::ofstream dateModOUtFile(path + ".ldm", std::ios::trunc);
    if(dateModOUtFile){
        dateModOUtFile << chsum;
    }
    else
        std::cout << "unable to write to dateModOUtFile" << std::endl;

    
}
void GameGPUCompute::RunInitCompute0()
{
    ret = clGetPlatformIDs(0, NULL, &ret_num_platforms);
    printf("num cl platforms: %d\n", ret_num_platforms);


    platforms = new cl_platform_id[ret_num_platforms];

    ret = clGetPlatformIDs(ret_num_platforms, platforms, NULL);
    CL_HOST_ERROR_CHECK(ret)

    ret = clGetDeviceIDs(platforms[pfidx], CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);
    CL_HOST_ERROR_CHECK(ret)

    char platformVerInfo[256];
    size_t sizeRet;
    clGetPlatformInfo(platforms[pfidx], CL_PLATFORM_VERSION,256, &platformVerInfo, &sizeRet);
    std::cout << "CL_PLATFORM_VERSION: " << platformVerInfo << std::endl;

    char platformVendorInfo[256];
    clGetPlatformInfo(platforms[pfidx], CL_PLATFORM_VENDOR,256, &platformVendorInfo, &sizeRet);
    std::cout << "CL_PLATFORM_VENDOR: " << platformVendorInfo << std::endl;

    if(std::string(platformVendorInfo).find("NVIDIA") != std::string::npos)
    {
        printf("NVIDIA DETECTED.\n");
        vendor = NVIDIA;
    }
    else if(std::string(platformVendorInfo).find("Advanced Micro Devices")  != std::string::npos)
    {
        printf("AMD DETECTED.\n");
        vendor = AMD;
    }


    clGetDeviceInfo(device_id,
    CL_DEVICE_MAX_COMPUTE_UNITS,
    sizeof(numComputeUnits),
    &numComputeUnits,
    NULL);

    std::cout << "CL_DEVICE_MAX_COMPUTE_UNITS: " << numComputeUnits << std::endl;


}
void GameGPUCompute::RunInitCompute1()
{
    // Load the update_kernel source code into the array source_str



    AddCLSource("openCL/clGame.cl");


    std::vector<char*> sourcePtrs;
    std::vector<uint64_t> sourceSizes;
    
    for(int i = 0; i < clSources.size(); i++)
    {
        sourcePtrs.push_back((char*)&clSources[i][0]);
        sourceSizes.push_back(clSources[i].size());
    }




    printf("source loading done\n");
    // Get platform and device information


    


    // Create an OpenCL context
    std::cout << SDL_GL_MakeCurrent(graphics->gWindow, graphics->sdlGLContext) << std::endl;
    cl_context_properties props[] =
    {
        CL_GL_CONTEXT_KHR, (cl_context_properties)(graphics->sdlGLContext),
        CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
        CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[pfidx],
        0
    };
    context = clCreateContext(props, 1, &device_id, NULL, NULL, &ret);
    if (ret != 0)
        printf("Unable To Initialize OpenCL.  Check your Driviers.");
    
    CL_HOST_ERROR_CHECK(ret)




    printf("Building FULL Game Program\n");
    // Create a gameProgram from the update_kernel source
    gameProgram = clCreateProgramWithSource(context, sourceSizes.size(),
        (const char**)&sourcePtrs[0], (const size_t*)&sourceSizes[0], &ret);
    CL_HOST_ERROR_CHECK(ret)


        //make preprocessor defs
        //std::string defs = buildPreProcessorString();
        writePreProcessorHeaderFile();


    // Build the gameProgram




    printf("Building....\n");
    ret = clBuildProgram(gameProgram, 1, &device_id, "-I openCL", NULL, NULL);

    //if (ret != CL_SUCCESS) {
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(gameProgram, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allocate memory for the log
        char* log = (char*)malloc(log_size);

        // Get the log
        clGetProgramBuildInfo(gameProgram, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Print the log
        printf("%s\n", log);



        delete log;
    //}
    CL_HOST_ERROR_CHECK(ret)

        printf("CL Programs built.\n");

    // Create a command queue
    command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE , &ret);
    CL_HOST_ERROR_CHECK(ret)
    command_queue2 = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE , &ret);
    CL_HOST_ERROR_CHECK(ret)
    {
        //run size tests
        printf("Running Size Tests\n");
        sizeTests_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(SIZETESTSDATA), nullptr, &ret);
        CL_HOST_ERROR_CHECK(ret)

        sizetests_kernel = clCreateKernel(gameProgram, "size_tests_kernel", &ret); CL_HOST_ERROR_CHECK(ret)
        CL_HOST_ERROR_CHECK(ret)

        cl_event sizeTestEvent;
        ret = clSetKernelArg(sizetests_kernel, 0, sizeof(cl_mem), (void*)&sizeTests_mem_obj); CL_HOST_ERROR_CHECK(ret)

        printf("running sizetest kernel..\n");
        ret = clEnqueueNDRangeKernel(command_queue, sizetests_kernel, 1, NULL,
            SingleKernelWorkItems, NULL, 0, NULL, &sizeTestEvent);
        CL_HOST_ERROR_CHECK(ret)


        ret = clWaitForEvents(1, &sizeTestEvent);
        CL_HOST_ERROR_CHECK(ret)

        ret = clEnqueueReadBuffer(command_queue, sizeTests_mem_obj, CL_TRUE, 0,
            sizeof(SIZETESTSDATA), &structSizes, 0, NULL, &sizeTestEvent);
        CL_HOST_ERROR_CHECK(ret)


        clWaitForEvents(1, &sizeTestEvent);

        std::cout << "GAMESTATE SIZE: " << structSizes.gameStateStructureSize << std::endl;
        std::cout << "STATICDATA SIZE: " << structSizes.staticDataStructSize << std::endl;

        assert(structSizes.gameStateStructureSize);
        assert(structSizes.staticDataStructSize);
    }
}





void GameGPUCompute::RunInitCompute2()
{
    BuildKernelRunSizes();


    WorkItemsInitMulti[0] = static_cast<size_t>(mapDim * mapDim) ;

        // Create memory buffers on the device
        //assert(structSizes.staticDataStructSize);
        printf("structSizes.staticDataStructSize: %d\n", structSizes.staticDataStructSize);
        staticData_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, structSizes.staticDataStructSize, nullptr, &ret);
    CL_HOST_ERROR_CHECK(ret)

       // assert(structSizes.gameStateStructureSize);
        printf("structSizes.gameStateStructureSize: %d\n", structSizes.gameStateStructureSize);
        gamestate_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, structSizes.gameStateStructureSize, nullptr, &ret);
    CL_HOST_ERROR_CHECK(ret)


        gamestateB_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(GameStateActions), nullptr, &ret);
    CL_HOST_ERROR_CHECK(ret)



        staticData_mem_obj_ro = clCreateBuffer(context, CL_MEM_READ_ONLY, structSizes.staticDataStructSize, nullptr, &ret);
    CL_HOST_ERROR_CHECK(ret)
        gamestate_mem_obj_ro = clCreateBuffer(context, CL_MEM_READ_ONLY, structSizes.gameStateStructureSize, nullptr, &ret);
    CL_HOST_ERROR_CHECK(ret)
        gamestateB_mem_obj_ro = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(GameStateActions), nullptr, &ret);
    CL_HOST_ERROR_CHECK(ret)





        graphics_peeps_mem_obj = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, graphics->peepInstanceVBO, &ret);
    CL_HOST_ERROR_CHECK(ret)


        graphics_particles_mem_obj = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, graphics->particleInstanceVBO, &ret);
    CL_HOST_ERROR_CHECK(ret)

        graphics_mapTile1VBO_mem_obj = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, graphics->mapTile1VBO, &ret);
    CL_HOST_ERROR_CHECK(ret)

        graphics_mapTile1AttrVBO_mem_obj = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, graphics->mapTile1AttrVBO, &ret);
    CL_HOST_ERROR_CHECK(ret)

        graphics_mapTile1OtherAttrVBO_mem_obj = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, graphics->mapTile1OtherAttrVBO, &ret);
    CL_HOST_ERROR_CHECK(ret)


        graphics_mapTile2VBO_mem_obj = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, graphics->mapTile2VBO, &ret);
    CL_HOST_ERROR_CHECK(ret)

        graphics_mapTile2AttrVBO_mem_obj = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, graphics->mapTile2AttrVBO, &ret);
    CL_HOST_ERROR_CHECK(ret)

        graphics_mapTile2OtherAttrVBO_mem_obj = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, graphics->mapTile2OtherAttrVBO, &ret);
    CL_HOST_ERROR_CHECK(ret)
        graphics_guiVBO_obj = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, graphics->guiRectInstanceVBO, &ret);
    CL_HOST_ERROR_CHECK(ret)
        graphics_linesVBO_obj = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, graphics->linesVBO, &ret);
    CL_HOST_ERROR_CHECK(ret)

    graphicsObjects.push_back(graphics_peeps_mem_obj);
    graphicsObjects.push_back(graphics_particles_mem_obj);
    graphicsObjects.push_back(graphics_mapTile1VBO_mem_obj);
    graphicsObjects.push_back(graphics_mapTile1AttrVBO_mem_obj);
    graphicsObjects.push_back(graphics_mapTile1OtherAttrVBO_mem_obj);
    graphicsObjects.push_back(graphics_mapTile2VBO_mem_obj);
    graphicsObjects.push_back(graphics_mapTile2AttrVBO_mem_obj);
    graphicsObjects.push_back(graphics_mapTile2OtherAttrVBO_mem_obj);
    graphicsObjects.push_back(graphics_guiVBO_obj);
    graphicsObjects.push_back(graphics_linesVBO_obj);


    cl_ulong localMemSize;
    clGetDeviceInfo(device_id,
        CL_DEVICE_LOCAL_MEM_SIZE,
        sizeof(localMemSize),
        &localMemSize,
        NULL);

    std::cout << "CL_DEVICE_LOCAL_MEM_SIZE: " << localMemSize << std::endl;


    cl_ulong globalMemSize;
    clGetDeviceInfo(device_id,
        CL_DEVICE_GLOBAL_MEM_SIZE,
        sizeof(globalMemSize),
        &globalMemSize,
        NULL);

    std::cout << "CL_DEVICE_GLOBAL_MEM_SIZE: " << globalMemSize << std::endl;





    // Create the OpenCL update_kernel
        preupdate_kernel = clCreateKernel(gameProgram, "game_preupdate_1", &ret); CL_HOST_ERROR_CHECK(ret)
        preupdate_kernel_2 = clCreateKernel(gameProgram, "game_preupdate_2", &ret); CL_HOST_ERROR_CHECK(ret)
        game_updatepre1_kernel = clCreateKernel(gameProgram, "game_updatepre1", &ret); CL_HOST_ERROR_CHECK(ret)
        update_kernel = clCreateKernel(gameProgram, "game_update", &ret); CL_HOST_ERROR_CHECK(ret)
        update2_kernel = clCreateKernel(gameProgram, "game_update2", &ret); CL_HOST_ERROR_CHECK(ret)
        action_kernel = clCreateKernel(gameProgram, "game_apply_actions", &ret); CL_HOST_ERROR_CHECK(ret)
        gui_kernel = clCreateKernel(gameProgram, "game_hover_gui", &ret); CL_HOST_ERROR_CHECK(ret)
        init_kernel = clCreateKernel(gameProgram, "game_init_single", &ret); CL_HOST_ERROR_CHECK(ret)
        init_kernal_multi = clCreateKernel(gameProgram, "game_init_multi", &ret); CL_HOST_ERROR_CHECK(ret)
        init_kernal_multi2 = clCreateKernel(gameProgram, "game_init_multi2", &ret); CL_HOST_ERROR_CHECK(ret)
        init_kernel_2 = clCreateKernel(gameProgram, "game_init_single2", &ret); CL_HOST_ERROR_CHECK(ret)


        post_update_kernel = clCreateKernel(gameProgram, "game_post_update", &ret); CL_HOST_ERROR_CHECK(ret)


        kernels.push_back(preupdate_kernel);
        kernels.push_back(preupdate_kernel_2);
        kernels.push_back(game_updatepre1_kernel);
        kernels.push_back(update_kernel);
        kernels.push_back(update2_kernel);
        kernels.push_back(action_kernel);
        kernels.push_back(gui_kernel);
        kernels.push_back(init_kernel);
        kernels.push_back(init_kernal_multi);
        kernels.push_back(init_kernal_multi2);
        kernels.push_back(init_kernel_2);
        kernels.push_back(post_update_kernel);

        // Set the arguments of the kernels
        for (cl_kernel k : kernels)
        {
            int i = 0;


            ret = clSetKernelArg(k, i++, sizeof(cl_mem), (void*)&staticData_mem_obj); CL_HOST_ERROR_CHECK(ret)
            ret = clSetKernelArg(k, i++, sizeof(cl_mem), (void*)&gamestate_mem_obj); CL_HOST_ERROR_CHECK(ret)
            ret = clSetKernelArg(k, i++, sizeof(cl_mem), (void*)&gamestateB_mem_obj); CL_HOST_ERROR_CHECK(ret)
           
            

            ret = clSetKernelArg(k, i++, sizeof(cl_mem), (void*)&graphics_peeps_mem_obj); CL_HOST_ERROR_CHECK(ret)
            ret = clSetKernelArg(k, i++, sizeof(cl_mem), (void*)&graphics_particles_mem_obj); CL_HOST_ERROR_CHECK(ret)
            ret = clSetKernelArg(k, i++, sizeof(cl_mem), (void*)&graphics_mapTile1VBO_mem_obj); CL_HOST_ERROR_CHECK(ret)
            ret = clSetKernelArg(k, i++, sizeof(cl_mem), (void*)&graphics_mapTile1AttrVBO_mem_obj); CL_HOST_ERROR_CHECK(ret)
            ret = clSetKernelArg(k, i++, sizeof(cl_mem), (void*)&graphics_mapTile1OtherAttrVBO_mem_obj); CL_HOST_ERROR_CHECK(ret)
            ret = clSetKernelArg(k, i++, sizeof(cl_mem), (void*)&graphics_mapTile2VBO_mem_obj); CL_HOST_ERROR_CHECK(ret)
            ret = clSetKernelArg(k, i++, sizeof(cl_mem), (void*)&graphics_mapTile2AttrVBO_mem_obj); CL_HOST_ERROR_CHECK(ret)
            ret = clSetKernelArg(k, i++, sizeof(cl_mem), (void*)&graphics_mapTile2OtherAttrVBO_mem_obj); CL_HOST_ERROR_CHECK(ret)

            ret = clSetKernelArg(k, i++, sizeof(cl_mem), (void*)&graphics_guiVBO_obj); CL_HOST_ERROR_CHECK(ret)
            ret = clSetKernelArg(k, i++, sizeof(cl_mem), (void*)&graphics_linesVBO_obj); CL_HOST_ERROR_CHECK(ret)
           


        }
        


        //get stats
        cl_ulong usedLocalMemSize;
    clGetKernelWorkGroupInfo(update_kernel,
        device_id,
        CL_KERNEL_LOCAL_MEM_SIZE,
        sizeof(cl_ulong),
        &usedLocalMemSize,
        NULL);
    std::cout << "CL_KERNEL_LOCAL_MEM_SIZE: " << usedLocalMemSize << std::endl;




    ret = clEnqueueWriteBuffer(command_queue, gamestate_mem_obj, CL_TRUE, 0,
        structSizes.gameStateStructureSize, gameState.get()->data, 0, NULL, NULL);
    CL_HOST_ERROR_CHECK(ret)

    ret = clEnqueueWriteBuffer(command_queue, gamestateB_mem_obj, CL_TRUE, 0,
        sizeof(GameStateActions), gameStateActions.get(), 0, NULL, NULL);
    CL_HOST_ERROR_CHECK(ret)



    AquireAllGraphicsObjects();

        ret = clEnqueueNDRangeKernel(command_queue, init_kernel, 1, NULL,
            SingleKernelWorkItems, NULL, 0, NULL, &initEvent);
        CL_HOST_ERROR_CHECK(ret)

        ret = clEnqueueNDRangeKernel(command_queue, init_kernal_multi, 1, NULL,
            WorkItemsInitMulti, NULL, 1, &initEvent, &initMultiEvent);
        CL_HOST_ERROR_CHECK(ret)

        ret = clEnqueueNDRangeKernel(command_queue, init_kernal_multi2, 1, NULL,
            WorkItemsInitMulti, NULL, 1, &initMultiEvent, &initMultiEvent2);
        CL_HOST_ERROR_CHECK(ret)



        ret = clEnqueueNDRangeKernel(command_queue, init_kernel_2, 1, NULL,
            SingleKernelWorkItems, NULL, 1, &initMultiEvent2, &init2Event);
        CL_HOST_ERROR_CHECK(ret)

    clWaitForEvents(1, &init2Event);    

    ret = clFinish(command_queue);
    CL_HOST_ERROR_CHECK(ret)
        
    ReleaseAllGraphicsObjects();
    
    ReadFullGameState();
}

void GameGPUCompute::Stage1_Begin()
{
    if (gameStateActions->pauseState != 0)
        return;


    BuildKernelRunSizes();

    ret = clFinish(command_queue);
    CL_HOST_ERROR_CHECK(ret)
    ret = clFinish(command_queue2);
    CL_HOST_ERROR_CHECK(ret)

    AquireAllGraphicsObjects();

    ret = clFinish(command_queue);
    CL_HOST_ERROR_CHECK(ret)
    ret = clFinish(command_queue2);
    CL_HOST_ERROR_CHECK(ret)

    ret = clEnqueueNDRangeKernel(command_queue, action_kernel, 1, NULL,
        SingleKernelWorkItems, NULL, 0, NULL, &actionEvent);
    CL_HOST_ERROR_CHECK(ret)


    ret = clEnqueueNDRangeKernel(command_queue2, gui_kernel, 1, NULL,
        SingleKernelWorkItems, NULL, 0, NULL, &guiEvent);
    CL_HOST_ERROR_CHECK(ret)

    clFlush(command_queue);
    clFlush(command_queue2);

    ret = clEnqueueNDRangeKernel(command_queue, preupdate_kernel, 1, NULL,
         WorkItems, NULL, 1, &actionEvent, &preUpdateEvent1);
    CL_HOST_ERROR_CHECK(ret)



    ret = clEnqueueNDRangeKernel(command_queue, preupdate_kernel_2, 1, NULL,
        WorkItems, NULL, 1, &preUpdateEvent1, &preUpdateEvent2);
    CL_HOST_ERROR_CHECK(ret)



    ret = clEnqueueNDRangeKernel(command_queue, game_updatepre1_kernel, 1, NULL,
        WorkItems, NULL, 1, &preUpdateEvent2, &updatepre1Event);
    CL_HOST_ERROR_CHECK(ret)

    ret = clEnqueueNDRangeKernel(command_queue, update_kernel, 1, NULL,
        WorkItems, NULL, 1, &updatepre1Event, &updateEvent);
    CL_HOST_ERROR_CHECK(ret)
    



    ret = clEnqueueNDRangeKernel(command_queue, update2_kernel, 1, NULL,
        WorkItems, NULL, 1, &updateEvent, &update2Event);
    CL_HOST_ERROR_CHECK(ret)
    


    cl_event w[2] = {update2Event, guiEvent};
    ret = clEnqueueNDRangeKernel(command_queue, post_update_kernel, 1, NULL,
        WorkItems, NULL, 2, w, &postupdateEvent);
    CL_HOST_ERROR_CHECK(ret)

    stage1_Running = true;



}
void GameGPUCompute::Stage1_End()
{
    if(!stage1_Running)
        return;

    ReleaseAllGraphicsObjects();

    ret = clFinish(command_queue);
    CL_HOST_ERROR_CHECK(ret)
    ret = clFinish(command_queue2);
    CL_HOST_ERROR_CHECK(ret)

    // Read the memory buffer C on the device to the local variable C
    ret = clEnqueueReadBuffer(command_queue, gamestateB_mem_obj, CL_TRUE, 0,
        sizeof(GameStateActions), gameStateActions.get(), 0, NULL, &readEvent);
    CL_HOST_ERROR_CHECK(ret)

    ret = clFinish(command_queue);
    CL_HOST_ERROR_CHECK(ret)
    ret = clFinish(command_queue2);
    CL_HOST_ERROR_CHECK(ret)

    stage1_Running = false;
}
void GameGPUCompute::ReadFullGameState()
{
    std::cout << "Reading Game State..." << std::endl;
    ret = clFinish(command_queue);
    CL_HOST_ERROR_CHECK(ret)

    // Read the memory buffer C on the device to the local variable C
     ret = clEnqueueReadBuffer(command_queue, gamestate_mem_obj, CL_TRUE, 0,
         structSizes.gameStateStructureSize, gameState.get()->data, 0, NULL, &readEvent);
    CL_HOST_ERROR_CHECK(ret)

    // Read the memory buffer C on the device to the local variable C
    ret = clEnqueueReadBuffer(command_queue, gamestateB_mem_obj, CL_TRUE, 0,
        sizeof(GameStateActions), gameStateActions.get(), 0, NULL, &readEvent);
    CL_HOST_ERROR_CHECK(ret)
    
    ret = clFinish(command_queue);
    CL_HOST_ERROR_CHECK(ret)

    std::cout << "Reading Game State [Finished]" << std::endl;
}

void GameGPUCompute::WriteFullGameState()
{
    std::cout << "Writing Game State..." << std::endl;
    ret = clEnqueueWriteBuffer(command_queue, gamestate_mem_obj, CL_TRUE, 0,
        structSizes.gameStateStructureSize, gameState.get()->data, 0, NULL, &writeEvent);
    CL_HOST_ERROR_CHECK(ret)

    WriteGameStateB();


    ret = clFinish(command_queue);
    CL_HOST_ERROR_CHECK(ret)

    std::cout << "Writing Game State [Finished]" << std::endl;
}

void GameGPUCompute::WriteGameStateB()
{
    ret = clEnqueueWriteBuffer(command_queue, gamestateB_mem_obj, CL_TRUE, 0,
        sizeof(GameStateActions), gameStateActions.get(), 0, NULL, &writeEvent);
    CL_HOST_ERROR_CHECK(ret)
}


void GameGPUCompute::SaveGameStateBase()
{
    ReadFullGameState();
    std::ofstream myfile;
    myfile.open("gamestatebase.bin", std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
    if (!myfile.is_open())
        std::cout << "Error Saving File!" << std::endl;
    else
    {
        myfile.write(reinterpret_cast<char*>(gameState.get()->data), structSizes.gameStateStructureSize);
    }
    
    myfile.close();
}


std::string GameGPUCompute::GameStateString(int tickIdx)
{
    return std::format("gamestate_tick_{}_.bin",gameStateActions->tickIdx);
}
void GameGPUCompute::SaveGameStateDiff(std::vector<char>* data, bool deleteFile)
{
    ReadFullGameState();
    std::ofstream myfile;

    myfile.open(GameStateString(gameStateActions->tickIdx), std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
    if (!myfile.is_open())
        std::cout << "Error Saving File!" << std::endl;
    else
    {
        myfile.write(reinterpret_cast<char*>(gameState.get()->data), structSizes.gameStateStructureSize);
    }
    
    myfile.close();
    std::string diffFileName = std::format("diff_tick_{}.diff", gameStateActions->tickIdx);
    std::string command = std::format(".\\windows_binaries\\hdiffz.exe -f -s-512 gamestatebase.bin {} {}", GameStateString(gameStateActions->tickIdx), diffFileName);
    std::system(command.c_str());


    std::filesystem::remove(GameStateString(gameStateActions->tickIdx));


    std::ifstream diffFile;
    diffFile.open(diffFileName, std::ifstream::binary);
    diffFile.seekg(0, diffFile.end);
    unsigned int fileSize = diffFile.tellg();
    diffFile.seekg(0, diffFile.beg);
    
    data->resize(fileSize);
    diffFile.read(data->data(),fileSize);
    diffFile.close();
    std::filesystem::remove(diffFileName);


}


void GameGPUCompute::LoadGameStateFromDiff(std::string diffFileName, std::string resultGameStateFileName)
{

    std::string command = std::format(".\\windows_binaries\\hpatchz.exe -f gamestatebase.bin {} {}", diffFileName, resultGameStateFileName);
    std::system(command.c_str());

    std::ifstream myfile;
    myfile.open(resultGameStateFileName, std::ifstream::binary);
    if (!myfile.is_open())
        std::cout << "Error Reading File!" << std::endl;
    else {
        
        std::cout << "Reading " << structSizes.gameStateStructureSize << " bytes" << std::endl;
        myfile.read(reinterpret_cast<char*>(gameState.get()->data), structSizes.gameStateStructureSize);
    
        if (myfile)
            std::cout << "all characters read successfully.";
        else
            std::cout << "error: only " << myfile.gcount() << " could be read";
    
    }
    myfile.close();

    std::filesystem::remove(resultGameStateFileName);

    WriteFullGameState();
}
void GameGPUCompute::LoadGameStateFromDiff(int tickidx)
{
    LoadGameStateFromDiff(std::format("diff_tick_{}.diff", tickidx), GameStateString(tickidx));
}

void GameGPUCompute::LoadGameStateFromDiff(std::vector<char>* diffdata, int id)
{
    std::ofstream diffFile;

    std::string diffFileName = std::format("netgame_{}.gamediff", id);
    diffFile.open(diffFileName, std::ofstream::binary | std::ofstream::trunc | std::ofstream::ate);
    diffFile.write(diffdata->data(), diffdata->size());
    diffFile.close();

    LoadGameStateFromDiff(diffFileName, std::format("netgame_{}.bin", id));

    std::filesystem::remove(diffFileName);



}





std::string GameGPUCompute::buildPreProcessorString()
{
    std::string str;
    str += std::string("-D ");
    for (auto pair : compileDefinitions)
    {
        str += pair.first + std::string("=") + compileVariantString(pair.second) + "\n";
    }
    std::cout << str;
    return str;
}


void GameGPUCompute::writePreProcessorHeaderFile()
{
    std::string str;
    for (auto pair : compileDefinitions)
    {
        str += std::string("#define ") + pair.first + std::string(" ") + compileVariantString(pair.second) + "\n";
    }
    std::cout << str;

    std::ofstream preProcessorHeader;
    preProcessorHeader.open("openCL/dynamicDefines.h", std::ofstream::out | std::ofstream::trunc);
    preProcessorHeader << str;
    preProcessorHeader.close();
}


std::string GameGPUCompute::compileVariantString(GPUCompileVariant variant)
{

    std::stringstream stream;
    try
    {
        stream << std::get<std::string>(variant);
        return stream.str();
    }
    catch(...) {};

    try
    {
        stream << std::get<int>(variant);
        return "(" + stream.str() + ")";
    }
    catch (...) {};

    try
    {
        stream << std::get<float>(variant);
        return "(" + stream.str() + ")";
    }
    catch (...) {};
    
    return "";
}

void GameGPUCompute::AquireAllGraphicsObjects()
{
    for (auto obj : graphicsObjects)
    {
        if(obj != graphics_guiVBO_obj)
        {
            ret = clEnqueueAcquireGLObjects(command_queue, 1, &obj, 0, 0, 0);
            CL_HOST_ERROR_CHECK(ret)
        }
        else{
            ret = clEnqueueAcquireGLObjects(command_queue2, 1, &obj, 0, 0, 0);
            CL_HOST_ERROR_CHECK(ret)
        }


    }
    //ret = clFinish(command_queue);
    //CL_HOST_ERROR_CHECK(ret)
}

void GameGPUCompute::ReleaseAllGraphicsObjects()
{
    for (auto obj : graphicsObjects)
    {
        if(obj != graphics_guiVBO_obj)
        {
            ret = clEnqueueReleaseGLObjects(command_queue, 1, &obj, 0, 0, 0);
            CL_HOST_ERROR_CHECK(ret)
        }
        else
        {
            ret = clEnqueueReleaseGLObjects(command_queue2, 1, &obj, 0, 0, 0);
            CL_HOST_ERROR_CHECK(ret)
        }
    }
    //ret = clFinish(command_queue);
    //CL_HOST_ERROR_CHECK(ret)
}
