#pragma once







#define MAX_PEEPS (1024*16)

#define WARPSIZE (32)
#define TOTALWORKITEMS MAX_PEEPS

#define SQRT_MAXSECTORS (128)
#define SECTOR_SIZE (32)

#define MAX_CLIENTS (256)

#define OFFSET_NULL (0xFFFFFFFF)



struct Cell;
struct MapSector;
struct Peep {

	//State:
	cl_int valid;
	int map_x_Q15_16;
	int map_y_Q15_16;
	int xv_Q15_16;
	int yv_Q15_16;

	int target_x_Q16;
	int target_y_Q16;

	int faction;

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
	int xidx;
	int yidx;
	cl_uint lock;
} typedef MapSector;

struct ClientAction {

	cl_uint submittedTickIdx;//the client tickidx when the action was created
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
} typedef ActionTracking;


struct ActionWrap {
	ClientAction action;
	ActionTracking tracking;

} typedef ActionWrap;


struct ClientState {

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

	cl_uint selectedPeepsLastIdx;
} typedef ClientState;

struct GameState {
	Peep peeps[MAX_PEEPS];
	MapSector sectors[SQRT_MAXSECTORS][SQRT_MAXSECTORS];
	cl_int mapWidth;
	cl_int mapHeight;


	ClientState clientStates[MAX_CLIENTS];
	cl_int numClients;

	ActionWrap clientActions[32];
	cl_int numActions;


	cl_uint tickIdx;
}typedef GameState;



