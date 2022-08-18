#pragma once

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <CL/cl_gl_ext.h>
#endif


#include <memory>
#include <variant>
#include <string>
#include <vector>

#include "peep.h"
#include "assert.h"

#define GAMECOMPUTE_MAX_SOURCE_SIZE (0x100000)
#define CL_HOST_ERROR_CHECK(ret) if (ret != 0) {printf("[GAMECOMPUTE] ret at %d is %d\n", __LINE__, ret); errorState = true; fflush(stdout); assert(0); }

typedef  std::variant<int, float, std::string> GPUCompileVariant;

class GameGraphics;
class GameGPUCompute
{
public:


	GameGPUCompute(std::shared_ptr<GameState> gameState, std::shared_ptr<GameStateActions> gameStateActions, GameGraphics* graphics);
	~GameGPUCompute();



	void AddCompileDefinition(std::string name, GPUCompileVariant val);

	void RunInitCompute();

	void Stage1();

	void ReadFullGameState();
	void WriteFullGameState();


	void WriteGameStateB();

	cl_context context;

	cl_command_queue command_queue;
	cl_program program;


	cl_kernel preupdate_kernel;
	cl_kernel preupdate_kernel_2;
	cl_kernel update_kernel;
	cl_kernel post_update_single_kernel;
	cl_kernel action_kernel;
	cl_kernel init_kernel;

	std::vector<cl_kernel> kernels;

	cl_event initEvent;
	cl_event preUpdateEvent1;
	cl_event preUpdateEvent2;
	cl_event updateEvent;    
	cl_event postupdateEvent;
	cl_event actionEvent;
	cl_event readEvent;
	cl_event writeEvent;    
	
	
	// Execute the OpenCL update_kernel on the list
    size_t SingleKernelWorkItems[1] = { 1 };
    size_t SingleKernelWorkItemsPerWorkGroup[1] = { 1 };


    size_t WorkItems[1] = { GAME_UPDATE_WORKITEMS };
	size_t WorkItems1Warp[1] = { WARPSIZE };

	cl_mem staticData_mem_obj;
	cl_mem gamestate_mem_obj;
	cl_mem gamestateB_mem_obj;

	cl_mem graphics_peeps_mem_obj;
	cl_mem graphics_mapTile1VBO_mem_obj;
	cl_mem graphics_mapTile1AttrVBO_mem_obj;

	cl_mem graphics_mapTile2VBO_mem_obj;
	cl_mem graphics_mapTile2AttrVBO_mem_obj;

	std::vector<cl_mem> graphicsObjects;


	std::shared_ptr<GameState> gameState;
	std::shared_ptr<GameStateActions> gameStateActions;
	GameGraphics* graphics = nullptr;

	std::vector<std::pair<std::string, GPUCompileVariant>> compileDefinitions;

	bool errorState = false;
	cl_int ret = 0;

	std::string buildPreProcessorString();
	std::string compileVariantString(GPUCompileVariant variant);


	void AquireAllGraphicsObjects();
	void ReleaseAllGraphicsObjects();
};

