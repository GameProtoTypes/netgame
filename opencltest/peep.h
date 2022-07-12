#pragma once



#ifndef __OPENCL_VERSION__
#include <stdint.h>

#else



#include <cl_type_glue.h>



#endif

#include "cpugpuvectortypes.h"

#define MAX_PEEPS (1024*32)
#define MAX_TRACKNODES (1024*8)
#define MAPDIM (1024)
#define MAPDEPTH (32)
#define MAP_TILE_SIZE (5)


#define WARPSIZE (32)
#define TOTALWORKITEMS MAX_PEEPS

#define SQRT_MAXSECTORS (128)
#define SECTOR_SIZE (32)

#define MAX_CLIENTS (1024)



#define OFFSET_NULL (0xFFFFFFFF)
#define CL_CHECKED_ARRAY_SET(ARRAY, ARRAY_SIZE, INDEX, VALUE) { if(INDEX >= ARRAY_SIZE) {printf("[CL] OUT OF BOUNDS INDEX SET ON ARRAY "  #ARRAY " line %d \n", __LINE__); } ARRAY[INDEX] = VALUE; }
#define CL_CHECKED_ARRAY_GET_PTR(ARRAY, ARRAY_SIZE, INDEX, POINTER) {if(INDEX >= ARRAY_SIZE) {printf("[CL] OUT OF BOUNDS INDEX GET ON ARRAY "  #ARRAY " line %d \n", __LINE__); POINTER = NULL;} POINTER = &ARRAY[INDEX];}
#define CL_CHECK_NULL(POINTER){if(POINTER == NULL) {printf("[CL] " #POINTER " POINTER IS NULL line %d \n", __LINE__);}}





struct Cell;
struct MapSector;

#pragma pack(push, 4)
struct PeepState_RenderLevel
{
	cl_int valid;
	ge_int3 pos_Q16;


	int32_t faction;

	cl_int attackState;
	cl_int health;
	cl_int deathState;
}typedef PeepState_RenderLevel;
#pragma pack(pop)

struct PeepState_SimLevel
{
	int32_t xv_Q15_16;
	int32_t yv_Q15_16;

	int32_t target_x_Q16;
	int32_t target_y_Q16;

	cl_long netForcex_Q16;
	cl_long netForcey_Q16;

	
}typedef PeepState_SimLevel;

#pragma pack(push, 4)
struct Peep {
	cl_uint Idx;

	struct PeepState_RenderLevel stateRender;
	struct PeepState_SimLevel stateSim;

	cl_int minDistPeep_Q16;
	struct Peep* minDistPeep;

	ge_int3 mapTileLoc_Q16;
	
	struct MapSector* mapSector_pending;
	struct MapSector* mapSector;

	struct Peep* nextSectorPeep;
	struct Peep* prevSectorPeep;

	//selection by clients
	cl_uint nextSelectionPeepIdx[MAX_CLIENTS];
	cl_uint prevSelectionPeepIdx[MAX_CLIENTS];


} typedef Peep;
#pragma pack(pop)

struct PeepRenderSupport {
	cl_int render_selectedByClient;
}typedef PeepRenderSupport;






enum MapTileBitAttr {
	MapTileBitAttr_MiningMarkBit = 0,
	MapTileBitAttr_Visible = 1

} typedef MapTileBitAttr;

enum MapTile {
	MapTile_Dirt = 0,
	MapTile_Sand,
	MapTile_LightGrass,
	MapTile_DarkGrass,
	MapTile_Rock,
	MapTile_IronOre,
	MapTile_CopperOre,
	MapTile_DiamondOre,
	MapTile_MossyRock,

	MapTile_NONE = 255
}typedef MapTile;


struct MapLevel {
	cl_short tiles[MAPDIM][MAPDIM];
} typedef MapLevel;



struct Map {
	MapLevel levels[MAPDEPTH];
	cl_int mapWidth;
	cl_int mapHeight;
} typedef Map;




struct MapSector {
	Peep* lastPeep;
	int32_t xidx;
	int32_t yidx;
	cl_uint lock;
} typedef MapSector;

struct ClientAction {

	//cl_uint submittedTickIdx;//the client tickidx when the action was created
	cl_uint scheduledTickIdx;//when it is scheduled to take effect on all clients


	cl_int action_DoSelect;
	cl_int params_DoSelect_StartX_Q16;
	cl_int params_DoSelect_StartY_Q16;
	cl_int params_DoSelect_EndX_Q16;
	cl_int params_DoSelect_EndY_Q16;

	cl_int action_CommandToLocation;
	cl_int params_CommandToLocation_X_Q16;
	cl_int params_CommandToLocation_Y_Q16;

	cl_int action_SetZView;
	cl_int params_ZViewIdx;
} typedef ClientAction;

struct ActionTracking {	

	cl_int clientId;

	cl_uint hostGivenId;
	cl_uint clientGivenId;
	cl_int ticksLate;//action could not be applied on client at scheduled tickId;
	bool finalActionVerified;
	bool clientApplied;

} typedef ActionTracking;


struct ActionWrap {
	ClientAction action;
	ActionTracking tracking;

} typedef ActionWrap;


struct SynchronizedClientState {

	cl_int connected;
	cl_uint selectedPeepsLastIdx;

	PeepRenderSupport peepRenderSupport[MAX_PEEPS];
} typedef SynchronizedClientState;

struct GameState {

	Peep peeps[MAX_PEEPS];
	Map map;
	MapSector sectors[SQRT_MAXSECTORS][SQRT_MAXSECTORS];
	

	SynchronizedClientState clientStates[MAX_CLIENTS];
	cl_int numClients;
}typedef GameState;

struct GameStateB {
	ActionWrap clientActions[32];
	cl_int numActions;


	cl_uint tickIdx;
	int32_t pauseState;


	//ClientSide only stuff that is processed in cl but not strictly gamestate
	cl_uint clientId;
	cl_int mapZView;
	cl_int mapZView_1;


}typedef GameStateB;