#pragma once


#include "cl_type_glue.h"
#include "dynamicDefines.h"


#include "gpuvectortypes.h"

#define MAX_TRACKNODES (1024*8)

#define SQRT_MAXSECTORS (128)
#define SECTOR_SIZE (8)

#define MAX_PATHS (8096)

//#define MAX_CLIENTS (1024)

#define OFFSET_NULL (0xFFFFFFFF)
#define OFFSET_NULL_2D  ((offsetPtr2){0xFFFFFFFF , 0xFFFFFFFF})
#define OFFSET_NULL_3D  ((offsetPtr3){0xFFFFFFFF , 0xFFFFFFFF,  0xFFFFFFFF})

#define OFFSET_NULL_SHORT_3D  ((offsetPtrShort3){0xFFFF , 0xFFFF,  0xFFFF})



#define CL_CHECKED_ARRAY_SET(ARRAY, ARRAY_SIZE, INDEX, VALUE) { if(INDEX >= ARRAY_SIZE) {printf("[CL] OUT OF BOUNDS INDEX SET ON ARRAY "  #ARRAY " line %d \n", __LINE__); } else ARRAY[INDEX] = VALUE; }
#define CL_CHECKED_ARRAY_GET_PTR(ARRAY, ARRAY_SIZE, INDEX, POINTER) {if(INDEX >= ARRAY_SIZE) {printf("[CL] OUT OF BOUNDS INDEX GET ON ARRAY "  #ARRAY " line %d \n", __LINE__); POINTER = NULL;} else POINTER = &ARRAY[INDEX];}
#define CL_CHECK_NULL(POINTER){if(POINTER == NULL) {printf("[CL] " #POINTER " POINTER IS NULL line %d \n", __LINE__);}}
#define CL_THROW_ASSERT(){printf("[CL] ASSERT line %d \n", __LINE__);}




#define OFFSET_TO_PTR(ARRAY, OFFSET, POINTER) { if(OFFSET == OFFSET_NULL){  POINTER = NULL;} else POINTER = &(ARRAY[OFFSET]);} 
#define OFFSET_TO_PTR_2D(ARRAY2D, OFFSET2D, POINTER) { if((OFFSET2D.x == OFFSET_NULL) || (OFFSET2D.y == OFFSET_NULL)){ POINTER = NULL; } else POINTER = &(ARRAY2D[OFFSET2D.x][OFFSET2D.y]); } 
#define OFFSET_TO_PTR_3D(ARRAY3D, OFFSET3D, POINTER) { if((OFFSET3D.x == OFFSET_NULL) || (OFFSET3D.y == OFFSET_NULL) || (OFFSET3D.z == OFFSET_NULL)){ POINTER = NULL; } else POINTER = &(ARRAY3D[OFFSET3D.x][OFFSET3D.y][OFFSET3D.z]); } 

#define CHECKED_OFFSET_TO_PTR_3D(ARRAY3D, ARRAYSIZE3D, OFFSET3D, POINTER) { if((OFFSET3D.x == OFFSET_NULL) || (OFFSET3D.y == OFFSET_NULL) || (OFFSET3D.z == OFFSET_NULL)){ POINTER = NULL; } else { if(OFFSET3D.x >= ARRAYSIZE3D.x || OFFSET3D.y >= ARRAYSIZE3D.y || OFFSET3D.z >= ARRAYSIZE3D.z) { printf("[CL] OUT OF BOUNDS INDEX GET ON ARRAY "  #ARRAY3D " line %d \n", __LINE__); }   POINTER = &(ARRAY3D[OFFSET3D.x][OFFSET3D.y][OFFSET3D.z]);} } 



#define BITSET(BITBANK, BITFLAG) {(BITBANK) |= (1 << BITFLAG);}
#define BITCLEAR(BITBANK, BITFLAG) {(BITBANK) &= ~(1 << BITFLAG);}
#define BITGET(BITBANK, BITFLAG) ((BITBANK) & (1 << BITFLAG))
#define BITGET_MF(BITBANK, BITFLAG) (((BITBANK) & (1 << BITFLAG))>>BITFLAG)




struct Cell;
struct MapSector;


enum PeepState_BitFlags
{
	PeepState_BitFlags_valid = 0,
	PeepState_BitFlags_deathState,
	PeepState_BitFlags_visible
};


struct PeepState_Basic
{
	cl_uint bitflags0;

	int32_t faction;

	cl_int health;
	cl_int deathState;


	cl_int buriedGlitchState;



	
	offsetPtr aStarSearchPtr;



}typedef PeepState_Basic;




struct BasePhysics
{	
	ge_int3 pos_Q16;
	ge_int3 v_Q16;

	ge_int3 pos_post_Q16;
	ge_int3 vel_add_Q16;

	float CS_angle_rad;

}typedef BasePhysics;


//constraint for a value to be clamped by the plane defined by p pos and d direction
struct LinearConstraint 
{
	ge_int3 p_Q16;
	ge_int3 d_Q16;
} typedef LinearConstraint;


struct PhysicsCircleShape
{
	int radius_Q16;
}typedef PhysicsCircleShape;


struct AStarPathNode
{
	ge_int3 mapCoord_Q16;
	offsetPtr nextOPtr;
	offsetPtr prevOPtr;
}typedef AStarPathNode;

struct DrivePhysics
{
	int32_t target_x_Q16;
	int32_t target_y_Q16;
	int32_t target_z_Q16;

	offsetPtr targetPathNodeOPtr;
	offsetPtr prevPathNodeOPtr;

	bool nearTarget;

	int drivingToTarget;
} typedef DrivePhysics;



struct PeepPhysics
{
	struct BasePhysics base;
	struct PhysicsCircleShape shape;
	struct DrivePhysics drive;
	struct LinearConstraint posConstraints[1];
	struct LinearConstraint velConstraints[1];
} typedef PeepPhysics;


struct PeepCommunication {
	int orders_channel;
	int message_TargetReached;
	int message_TargetReached_pending;
}typedef PeepCommunication;



struct Peep {
	offsetPtr ptr;

	struct PeepState_Basic stateBasic;
	struct PeepPhysics physics;
	struct PeepCommunication comms;

	cl_int minDistPeep_Q16;
	offsetPtr minDistPeepPtr;

	offsetPtrShort3 mapCoord;
	offsetPtrShort3 mapCoord_1;


	ge_int3 posMap_Q16;
	ge_int3 lastGoodPosMap_Q16;
	
	offsetPtr2 mapSector_pendingPtr;
	offsetPtr2 mapSectorPtr;

	offsetPtr nextSectorPeepPtr;
	offsetPtr prevSectorPeepPtr;

	//selection by clients
	offsetPtr nextSelectionPeepPtr[MAX_CLIENTS];
	offsetPtr prevSelectionPeepPtr[MAX_CLIENTS];

} typedef Peep;



struct Particle {

	QMP32_2 pos;
	QMP32_2 vel;
}typedef Particle;


struct Triangle3D {
	ge_int3 verts_Q16[3];
} typedef Triangle3D;

struct Triangle3DHeavy{
	Triangle3D base;

	ge_int3 normal_Q16;
	ge_int3 u_Q16;
	ge_int3 v_Q16;

	cl_uchar valid;
} typedef Triangle3DHeavy;

struct  ConvexHull{
	Triangle3DHeavy triangles[14];
} typedef ConvexHull;



struct MapExplorerAgent
{
	ge_short3 tileLoc;
} typedef MapExplorerAgent;


enum MapTileFlags {
	MapTileFlags_claimed = 10,
	MapTileFlags_miningInProgress,
	
	MapTileFlags_LowCornerTPLEFT,
	MapTileFlags_LowCornerTPRIGHT,
	MapTileFlags_LowCornerBTMRIGHT,
	MapTileFlags_LowCornerBTMLEFT,

	MapTileFlags_RotBit1,
	MapTileFlags_RotBit2,

	MapTileFlags_Explored,

	MapTileFlags_PeepCount0,
	MapTileFlags_PeepCount1,
	MapTileFlags_PeepCount2,

	MapTileFlags_LASTFLAG
} typedef MapTileFlags;




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
	MapTile_Shadow_0,
	MapTile_Shadow_1,
	MapTile_Shadow_2,
	MapTile_Shadow_3,
	MapTile_Shadow_4,
	MapTile_Lava     = 16,
	MapTile_Shadow_5 = 25,
	MapTile_Shadow_6,
	MapTile_Shadow_7,
	MapTile_Shadow_8,
	MapTile_Shadow_9,
	MapTile_Shadow_10 = 41,
	MapTile_Shadow_11,
	MapTile_Shadow_12,
	MapTile_Shadow_13,
	MapTile_Shadow_14,
	MapTile_Shadow_15,
	MapTile_Shadow_16,

	MapTile_NONE = 255
}typedef MapTile;


struct MapLevel {	
	//[XXXX|FFFF|FFFF|FFFF|FFFF|FFFF|AAAA|AAAA]
	//A - MapTile
	//F - FLAGS
	cl_uint data[MAPDIM][MAPDIM];

	uint peepCounts[MAPDIM][MAPDIM];
	uint peepCounts_Final[MAPDIM][MAPDIM];
} typedef MapLevel;

struct Map {
	MapLevel levels[MAPDEPTH];
	cl_int mapWidth;
	cl_int mapHeight;
} typedef Map;


struct MapSector {
	offsetPtr lastPeepPtr;
	offsetPtr2 ptr;
	cl_uint lock;
} typedef MapSector;


struct AStarNode {
	ge_short3 tileIdx;
	int h_Q16;
	int g_Q16;

	offsetPtr3 nextOPtr;
	offsetPtr3 prevOPtr;

} typedef AStarNode;

enum AStarPathFindingProgress
{
	AStarPathFindingProgress_Ready,
	AStarPathFindingProgress_Searching,
	AStarPathFindingProgress_Finished,
	AStarPathFindingProgress_Failed


} typedef AStarPathFindingProgress;


#define ASTARHEAPSIZE ((MAPDIM*MAPDIM*MAPDEPTH)/10)
struct AStarSearch_BFS {
	AStarNode details[MAPDIM][MAPDIM][MAPDEPTH];
	
	offsetPtr3 openHeap_OPtrs[ASTARHEAPSIZE];
	cl_int openHeapSize;
	offsetPtr3 endNodeOPtr;
	offsetPtr3 startNodeOPtr;

	cl_uchar closedMap[MAPDIM][MAPDIM][MAPDEPTH];
	cl_uchar openMap[MAPDIM][MAPDIM][MAPDEPTH];
	
	AStarPathFindingProgress state;

	offsetPtr pathOPtr;

} typedef AStarSearch_BFS;

struct AStarNode_IDA
{
	ge_short3 tileLoc;
	bool searchedSuccessors[26];
	int gCost;

} typedef AStarNode_IDA;

#define ASTARSEARCH_IDA_PATHMAXSIZE ((MAPDIM*2))
struct AStarSearch_IDA {

	ge_short3 startLoc;
	ge_short3 endLoc;

	AStarNode_IDA path[ASTARSEARCH_IDA_PATHMAXSIZE];
	int pathEndIdx;

	int bound;
	int t;

	AStarPathFindingProgress state;
	offsetPtr pathOPtr;
} typedef AStarSearch_IDA;



struct AStarPathJob
{
	ge_short3 startLoc;
	ge_short3 endLoc;
} typedef AStarPathJob;

#define ASTARJOBSCHEDULER_MAX_JOBS (256)
struct AStarJobScheduler
{
	AStarPathJob jobQueue[ASTARJOBSCHEDULER_MAX_JOBS];
	int curJobIdx;
} typedef AStarJobScheduler;


#define ASTARPATHSTEPSSIZE ((MAPDIM*MAPDIM*MAPDEPTH)/10)
#define ASTAR_MAX_PATHS (1024)
struct AStarPathSteps
{
	AStarPathNode pathNodes[ASTARPATHSTEPSSIZE];
	int nextListIdx;

	offsetPtr pathStarts[ASTAR_MAX_PATHS];
	int nextPathStartIdx;

}typedef AStarPathSteps;




struct PeepRenderSupport {
	cl_int render_selectedByClient;
}typedef PeepRenderSupport;


enum GuiStatePassType
{
	GuiStatePassType_NoLogic,
	GuiStatePassType_Synced
} typedef GuiStatePassType;


struct GuiStyle
{
	float2 UV_WHITE;

	float3 TEXT_COLOR;

	float3 BUTTON_COLOR;
	float3 BUTTON_COLOR_HOVER;
	float3 BUTTON_COLOR_ACTIVE;

	float3 SLIDER_COLOR_BACKGROUND;

}typedef GuiStyle;



#define SYNCGUI_MAX_WIDGETS (512)
#define SYNCGUI_MAX_DEPTH (8)
struct SyncedGui
{
	int guiRenderRectIdx;
	int nextId;

	ge_int2 mouseLocBegin;
	ge_int2 mouseLoc;
	ge_int2 mouseFrameDelta;
	int mouseState;

	int fakeDummyInt;

	cl_uchar ignoreAll;//1 when mouse button pushed off-gui and then goes on gui while button(s) held
	cl_uchar dragOff;//1 when gui held and then mouse dragged off.
	cl_uchar draggedOff;//1 when gui held and then mouse dragged off and released.

	cl_uchar mouseDragging;

	int lastActiveWidget;
	int hoverWidget;
	int activeWidget;

	cl_uchar mouseOnGUI;

	ge_int2 widgetOffsetStack[SYNCGUI_MAX_DEPTH];
	int wOSidx;

	ge_int4 clipStack[SYNCGUI_MAX_DEPTH];
	int clipStackIdx;

	ge_int2 curBoundStart;
	ge_int2 curBoundEnd;

	GuiStatePassType passType;

	int fakeInts[SYNCGUI_MAX_WIDGETS];
	int nextFakeIntIdx;


} typedef SyncedGui;

enum EditorTools
{
	EditorTools_None,
	EditorTools_Delete,
	EditorTools_Create
} typedef EditorTools;

struct SynchronizedClientState {

	cl_int connected;
	cl_uint selectedPeepsLastIdx;

	PeepRenderSupport peepRenderSupport[MAX_PEEPS];


	ge_int2 mouseGUIBegin; cl_uchar mouseOnGUiBegin;
	ge_int2 mouseGUIEnd;

	
	ge_int2 mouseWorldBegin_Q16;
	ge_int2 mouseWorldEnd_Q16;

	int mapZView;
	int mapZView_1;

	EditorTools curTool;
	
	SyncedGui gui;

} typedef SynchronizedClientState;





struct GameState {

	SynchronizedClientState clientStates[MAX_CLIENTS];
	cl_int numClients;


	Peep peeps[MAX_PEEPS];
	
	Particle particles[MAX_PARTICLES];

	MapExplorerAgent explorerAgents[GAME_UPDATE_WORKITEMS];
	
	Map map;
	MapSector sectors[SQRT_MAXSECTORS][SQRT_MAXSECTORS];
	
	AStarSearch_BFS mapSearchers[1];

	AStarPathSteps paths;

	SyncedGui fakeGui;
	GuiStyle guiStyle;

	cl_uint debugLinesIdx;



} typedef GameState;


struct StaticData {
	ge_int3 directionalOffsets[26];

}typedef StaticData;
