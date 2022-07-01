#pragma once



#ifndef __OPENCL_VERSION__
#include <stdint.h>

#else



#include <cl_type_glue.h>



#endif


#define MAX_PEEPS (1024*32)

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
struct Peep {

	cl_int valid;
	int32_t map_x_Q15_16;
	int32_t map_y_Q15_16;
	
	
	
	int32_t xv_Q15_16;
	int32_t yv_Q15_16;

	int32_t target_x_Q16;
	int32_t target_y_Q16;

	int32_t faction;

	cl_int attackState;
	cl_int health;
	cl_int deathState;
	


	cl_long netForcex_Q16;
	cl_long netForcey_Q16;

	cl_int minDistPeep_Q16;
	struct Peep* minDistPeep;

	
	struct MapSector* mapSector_pending;
	struct MapSector* mapSector;

	struct Peep* nextSectorPeep;
	struct Peep* prevSectorPeep;

	//selection by clients
	cl_uint nextSelectionPeepIdx[MAX_CLIENTS];
	cl_uint prevSelectionPeepIdx[MAX_CLIENTS];


} typedef Peep;

struct PeepRenderSupport {
	cl_int render_selectedByClient;
}typedef PeepRenderSupport;


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

struct ClientSideClientState {
	cl_int mousex;
	cl_int mousey;
	cl_int mouse_dragBeginx;
	cl_int mouse_dragBeginy;
	cl_int mousescroll;
	cl_int clicked;

	cl_int mousePrimaryDown;
	cl_int mousePrimaryPressed;
	cl_int mousePrimaryReleased;

	cl_int mouseSecondaryDown;
	cl_int mouseSecondaryPressed;
	cl_int mouseSecondaryReleased;

	cl_float viewX;
	cl_float viewY;
	cl_float view_beginX;
	cl_float view_beginY;
	cl_float viewScale;
};
struct SynchronizedClientState {

	cl_int connected;
	cl_uint selectedPeepsLastIdx;
} typedef SynchronizedClientState;

struct GameState {
	Peep peeps[MAX_PEEPS];
	MapSector sectors[SQRT_MAXSECTORS][SQRT_MAXSECTORS];
	cl_int mapWidth;
	cl_int mapHeight;


	SynchronizedClientState clientStates[MAX_CLIENTS];
	cl_int numClients;

	ActionWrap clientActions[32];
	cl_int numActions;


	cl_uint tickIdx;
	int32_t pauseState;
}typedef GameState;



