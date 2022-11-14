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

#include "assert.h"

#include "Common.h"
#include "openCL/cpu_gpu_structs.h"

#define GAMECOMPUTE_MAX_SOURCE_SIZE (0x100000)
#define CL_HOST_ERROR_CHECK(ret) if (ret != 0) {printf("[GAMECOMPUTE] ret at %d is %d\n", __LINE__, ret); errorState = true; fflush(stdout); assert(0); }



#define WARPSIZE (64)


typedef  std::variant<int, float, std::string> GPUCompileVariant;





class GameState_Pointer
{
public:
	GameState_Pointer(uint64_t size) { 
		data = new int8_t[size];
		memset(data, 0, size);
	}
	~GameState_Pointer() { delete[] data; }
	void* data = nullptr;
};





class GameGraphics;
class GameGPUCompute
{
public:


	GameGPUCompute();
	~GameGPUCompute();

	void OverlapTest();

	void AddCLSource(std::string path);


	void AddCompileDefinition(std::string name, GPUCompileVariant val);

	void RunInitCompute0();
	void RunInitCompute1();
	void RunInitCompute2();

	void Stage1_Begin();
	void Stage1_End();

	bool stage1_Running = false;

	void ReadFullGameState();
	void WriteFullGameState();


	void WriteGameStateB();

	std::string GameStateString(int tickIdx);
	void SaveGameStateBase();
	void SaveGameStateDiff(std::vector<char>* data,bool deleteFile = true);
	void LoadGameStateFromDiff(int tickidx);
	void LoadGameStateFromDiff(std::string diffFileName, std::string resultGameStateFileName);
	void LoadGameStateFromDiff(std::vector<char>* data,  int id);

	enum VENDOR
	{
		NVIDIA,
		AMD
	};

	cl_context context;

	cl_device_id device_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    int pfidx = 0;
	cl_platform_id* platforms;
	VENDOR vendor;

	cl_command_queue command_queue;
	cl_command_queue command_queue2;

	cl_program gameProgram;
	cl_program rayGuiHeader;
	cl_program fullProgram;
	cl_program testProgram;

	cl_kernel sizetests_kernel;

	cl_kernel preupdate_kernel;
	cl_kernel preupdate_kernel_2;
	cl_kernel game_updatepre1_kernel;
	cl_kernel update_kernel;
	cl_kernel update2_kernel;
	cl_kernel post_update_kernel;
	cl_kernel action_kernel;
	cl_kernel gui_kernel;
	cl_kernel init_kernel;
	cl_kernel init_kernal_multi;
	cl_kernel init_kernal_multi2;
	cl_kernel init_kernel_2;



	std::vector<cl_kernel> kernels;

	cl_event initEvent;
	cl_event initMultiEvent;
	cl_event initMultiEvent2;
	cl_event init2Event;

	cl_event glBindEventCommandQue1;
	cl_event glBindEventCommandQue2;

	cl_event preUpdateEvent1;
	cl_event preUpdateEvent2;
	cl_event updatepre1Event;    
	cl_event updateEvent;  
	cl_event update2Event;      
	cl_event postupdateEvent;
	cl_event actionEvent;
	cl_event guiEvent;
	cl_event readEvent;
	cl_event writeEvent;    

	


	SIZETESTSDATA structSizes;

	int warpSize = 32;
	int maxPeeps = 1024*8;
	int maxParticles = 32;
	int mapDim = 1024;
	int mapDepth = 32;
	int mapTileSize = 5;
    int maxGuiRects = 1024*16;
	int maxLines = 1024*64;

    long GameUpdateWorkItems ;
	int numComputeUnits;
    size_t WorkItems[1]  ;
	size_t WorkItemsInitMulti[1] ;
	size_t WorkItems1Warp[1] ;
    size_t SingleKernelWorkItems[1];
    size_t SingleKernelWorkItemsPerWorkGroup[1];



	cl_mem sizeTests_mem_obj;


	cl_mem staticData_mem_obj;
	cl_mem gamestate_mem_obj;
	cl_mem gamestateB_mem_obj;

	cl_mem staticData_mem_obj_ro;
	cl_mem gamestate_mem_obj_ro;
	cl_mem gamestateB_mem_obj_ro;


	cl_mem graphics_peeps_mem_obj;
	cl_mem graphics_particles_mem_obj;

	cl_mem graphics_mapTile1VBO_mem_obj;
	cl_mem graphics_mapTile1AttrVBO_mem_obj;
	cl_mem graphics_mapTile1OtherAttrVBO_mem_obj;

	cl_mem graphics_mapTile2VBO_mem_obj;
	cl_mem graphics_mapTile2AttrVBO_mem_obj;
	cl_mem graphics_mapTile2OtherAttrVBO_mem_obj;

	cl_mem graphics_guiVBO_obj;
	cl_mem graphics_linesVBO_obj;

	std::vector<cl_mem> graphicsObjects;

	void BuildKernelRunSizes();


	std::shared_ptr<GameState_Pointer> gameState;
	std::shared_ptr<GameStateActions> gameStateActions;
	GameGraphics* graphics = nullptr;

	std::vector<std::pair<std::string, GPUCompileVariant>> compileDefinitions;

	bool errorState = false;
	cl_int ret = 0;

	std::string buildPreProcessorString();
	void writePreProcessorHeaderFile();

	std::string compileVariantString(GPUCompileVariant variant);


	void AquireAllGraphicsObjects();
	void ReleaseAllGraphicsObjects();



	std::vector<uint64_t> clSourceCHKSUMS;
    std::vector<std::string> clSourcePaths;
    std::vector<std::vector<char>> clSources;
};

