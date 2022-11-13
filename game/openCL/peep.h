#pragma once


#include "cl_type_glue.h"
#include "dynamicDefines.h"


#include "gpuvectortypes.h"

#define MAX_TRACKNODES (1024*8)

#define SQRT_MAXSECTORS (MAPDIM/2)
#define SECTOR_SIZE (10)

#define MAX_PATHS (8096)

//#define MAX_CLIENTS (1024)

#define OFFSET_NULL (0xFFFFFFFF)
#define OFFSET_NULL_2D  ((offsetPtr2){0xFFFFFFFF , 0xFFFFFFFF})
#define OFFSET_NULL_3D  ((offsetPtr3){0xFFFFFFFF , 0xFFFFFFFF,  0xFFFFFFFF})

#define OFFSET_NULL_SHORT_3D  ((offsetPtrShort3){0xFFFF , 0xFFFF,  0xFFFF})



#define CL_CHECKED_ARRAY_SET(ARRAY, ARRAY_SIZE, INDEX, VALUE) { if(INDEX >= ARRAY_SIZE) {printf("[CL] OUT OF BOUNDS INDEX SET ON ARRAY "  #ARRAY " line %d \n", __LINE__); } else ARRAY[INDEX] = VALUE; }
#define CL_CHECKED_ARRAY_GET_PTR(ARRAY, ARRAY_SIZE, INDEX, POINTER) {if(INDEX >= ARRAY_SIZE) {printf("[CL] OUT OF BOUNDS INDEX %u ON ARRAY "  #ARRAY " line %d \n", #INDEX, __LINE__); POINTER = NULL;} else POINTER = &ARRAY[INDEX];}
#define CL_CHECK_NULL(POINTER){if(POINTER == NULL) {printf("[CL] " #POINTER " POINTER IS NULL line %d \n", __LINE__);}}
#define CL_THROW_ASSERT(){printf("[CL] ASSERT line %d \n", __LINE__);}




#define OFFSET_TO_PTR(ARRAY, OFFSET, POINTER) { if(OFFSET == OFFSET_NULL){  POINTER = NULL;} else POINTER = &(ARRAY[OFFSET]);  } 
#define OFFSET_TO_PTR_2D(ARRAY2D, OFFSET2D, POINTER) { if((OFFSET2D.x == OFFSET_NULL) || (OFFSET2D.y == OFFSET_NULL)){ POINTER = NULL; } else POINTER = &(ARRAY2D[OFFSET2D.x][OFFSET2D.y]);} 
#define OFFSET_TO_PTR_3D(ARRAY3D, OFFSET3D, POINTER) { if((OFFSET3D.x == OFFSET_NULL) || (OFFSET3D.y == OFFSET_NULL) || (OFFSET3D.z == OFFSET_NULL)){ POINTER = NULL; } else POINTER = &(ARRAY3D[OFFSET3D.x][OFFSET3D.y][OFFSET3D.z]);} 

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


	offsetPtr orderPtr;
	bool orderInProgress;

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
	bool valid;
	ge_int3 mapCoord_Q16;
	offsetPtr nextOPtr;
	offsetPtr prevOPtr;

	bool completed;
	bool processing;

}typedef AStarPathNode;

struct DrivePhysics
{
	int32_t target_x_Q16;
	int32_t target_y_Q16;
	int32_t target_z_Q16;

	offsetPtr targetPathNodeOPtr;
	offsetPtr prevPathNodeOPtr;

	bool nearTarget;


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


enum ItemTypes
{
	ItemType_INVALID_ITEM,
	ItemType_IRON_ORE,
	ItemType_IRON_DUST,
	ItemType_IRON_BAR,
	ItemType_ROCK_DUST,

	ItemType_TIN_ORE,
	ItemType_TIN_DUST,

	ItemType_COBALT_ORE,
	ItemType_NICKLE_ORE,

	ItemType_RUTHENIUM_ORE,
	ItemType_RUTHENIUM_DUST,

	ItemType_RHODIUM_ORE,
	ItemType_RHODIUM_DUST,

	ItemType_PALADIUM_ORE,
	ItemType_PALADIUM_DUST,

	ItemType_OSMIUM_ORE,
	ItemType_OSMIUM_DUST,

	ItemType_IRIDIUM_ORE,
	ItemType_IRIDIUM_DUST,

	ItemType_PLATINUM_ORE,
	ItemType_PLATINUM_DUST,


	ItemType_SOME_ORE = 256,

	ItemTypes_NUMITEMS
} typedef ItemTypes;

struct Inventory
{
	int counts[ItemTypes_NUMITEMS];
} typedef Inventory;

struct Peep {
	offsetPtr ptr;

	struct PeepState_Basic stateBasic;
	struct PeepPhysics physics;
	struct PeepCommunication comms;

	Inventory inventory;


	cl_int minDistPeep_Q16;
	offsetPtr minDistPeepPtr;

	offsetPtrShort3 mapCoord;
	offsetPtrShort3 mapCoord_1;


	ge_int3 posMap_Q16;
	ge_int3 lastGoodPosMap_Q16;

	offsetPtr2 sectorPtr;
	int sectorListIdx;
	


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
	MapTile_GoldOre,
	MapTile_IronOre,
	MapTile_DiamondOre,
	MapTile_MossyRock,
	MapTile_Shadow_0,
	MapTile_Shadow_1,
	MapTile_Shadow_2,
	MapTile_Shadow_3,
	MapTile_Shadow_4,
	MapTile_Lava     = 16,
	MapTile_WOODGRID,
	MapTile_MACHINE_CRUSHER,
	MapTile_MACHINE_FURNACE,
	MapTile_Shadow_5 = 25,
	MapTile_Shadow_6,
	MapTile_Shadow_7,
	MapTile_Shadow_8,
	MapTile_Shadow_9,
	MapTile_MACHINE_COMMAND_CENTER = 35,
	MapTile_Shadow_10 = 41,
	MapTile_Shadow_11,
	MapTile_Shadow_12,
	MapTile_Shadow_13,
	MapTile_Shadow_14,
	MapTile_Shadow_15,
	MapTile_Shadow_16,

	MapTile_NONE = 255
}typedef MapTile;


#define NUM_ITEMTILES (3)
enum ItemTile {
	ItemTile_Dust = 20,
	ItemTile_Bar = 21,
	ItemTile_Ore = 22
}typedef ItemTile;

union TileUnion {
	MapTile mapTile;
	ItemTile itemTile;
}typedef TileUnion;


struct MapLevel {	
	//[XXXX|FFFF|FFFF|FFFF|FFFF|FFFF|AAAA|AAAA]
	//A - MapTile
	//F - FLAGS
	cl_uint data[MAPDIM][MAPDIM];

	offsetPtr machinePtr[MAPDIM][MAPDIM];

	uint peepCounts[MAPDIM][MAPDIM];
	uint peepCounts_Final[MAPDIM][MAPDIM];
} typedef MapLevel;

struct Map {
	MapLevel levels[MAPDEPTH];
	cl_int mapWidth;
	cl_int mapHeight;


} typedef Map;

#define MAX_PEEPS_PER_SECTOR (16)
struct MapSector {
	offsetPtr peepPtrs[MAX_PEEPS_PER_SECTOR];
	cl_int ptrIterator;
	offsetPtr2 ptr;
	cl_uint lock;
	bool empty;

	int chunkStart;
} typedef MapSector;


struct AStarNode {
	ge_short3 tileIdx;
	int h_Q16;
	int g_Q16;

	offsetPtr nextOPtr;
	offsetPtr prevOPtr;

} typedef AStarNode;

enum AStarPathFindingProgress
{
	AStarPathFindingProgress_Ready,
	AStarPathFindingProgress_Searching,
	AStarPathFindingProgress_Finished,
	AStarPathFindingProgress_Failed,

	AStarPathFindingProgress_ResetReady


} typedef AStarPathFindingProgress;


#define ASTARHEAPSIZE ((MAPDIM*MAPDIM*MAPDEPTH)/100)
#define ASTARDETAILSSIZE ((MAPDIM*MAPDIM*MAPDEPTH)/100)
struct AStarSearch_BFS {
	AStarNode details[ASTARDETAILSSIZE];
	offsetPtr nextDetailNodePtr;
	
	offsetPtr openHeap_OPtrs[ASTARHEAPSIZE];
	cl_int openHeapSize;
	
	offsetPtr endNodeOPtr;
	offsetPtr startNodeOPtr;

	offsetPtr closedMap[MAPDIM][MAPDIM][MAPDEPTH];
	offsetPtr openMap[MAPDIM][MAPDIM][MAPDEPTH];
	
	AStarPathFindingProgress state;

	offsetPtr pathOPtr;

} typedef AStarSearch_BFS;



#define ASTARPATHSTEPSSIZE ((MAPDIM*MAPDIM*MAPDEPTH)/10)
#define ASTAR_MAX_PATHS (1024)
struct AStarPathSteps
{
	AStarPathNode pathNodes[ASTARPATHSTEPSSIZE];
	int nextListIdx;

	offsetPtr pathStarts[ASTAR_MAX_PATHS];
	int nextPathStartIdx;

}typedef AStarPathSteps;


enum AStarJobStatus
{
	AStarJobStatus_Disabled,
	AStarJobStatus_Pending,
	AStarJobStatus_Working,
	AStarJobStatus_Done
} typedef AStarJobStatus;

#define ASTAR_MAX_JOBS (128)
struct AStarJob
{
	ge_int3 startLoc;
	ge_int3 endLoc;

	AStarJobStatus status;
	offsetPtr pathPtr;
} typedef AStarJob;




struct PeepRenderSupport {
	cl_int render_selectedByClient;
}typedef PeepRenderSupport;







enum MachineTypes
{
	MachineTypes_CRUSHER,
	MachineTypes_SMELTER,
	MachineTypes_COMMAND_CENTER,
	MachineTypes_MINING_SITE,

	MachineTypes_NUMTYPES
} typedef MachineTypes;

enum MachineRecipes
{
	MachineRecipe_INVALID,
	MachineRecipe_IRON_ORE_CRUSHING,
	MachineRecipe_IRON_DUST_SMELTING,

	MachineRecipe_NUMRECIPES
} typedef MachineRecipes;



struct MachineRecipe
{
	ItemTypes inputTypes[8]; int numInputs;
	ItemTypes outputTypes[8]; int numOutputs;

	int inputRatio[8];
	int outputRatio[8];
} typedef MachineRecipe;

struct MachineDesc
{
	MachineTypes type;
	MapTile tile;



	int processingTime;
} typedef MachineDesc;

enum MachineState
{
	MachineState_Idle,
	MachineState_Running,
	MachineState_Damaged
} typedef MachineState;


#define MAX_MACHINES (1024*4)
struct Machine
{
	bool valid;
	offsetPtrShort3 mapTilePtr;
	offsetPtr MachineDescPtr;//ptr into bank of descriptions.

	offsetPtr recipePtr;

	int tickProgess;
	MachineState state;

	Inventory inventoryIn;
	Inventory inventoryOut;

	//Command Center Sepecific
	offsetPtr rootOrderPtr;
	int orderLen;

} typedef Machine;





#define MAX_ORDERS (1024)
enum OrderActions
{
	OrderAction_NONE,
	OrderAction_MINE,
	OrderAction_DROPOFF_MACHINE,
	OrderAction_PICKUP_MACHINE,
	OrderAction_WAYPOINT,
	OrderAction_JUMP_TO_ORDER
} typedef OrderActions;


struct OrderActionData
{
	offsetPtr jumpToOrderPtr;

};


struct Order
{
	offsetPtr ptr;
	bool valid;
	bool pendingDelete;
	int refCount;


	ge_int3 mapDest_Coord;
	bool destinationSet;
	offsetPtr aStarJobPtr;
	offsetPtr pathToDestPtr;
	bool selectingOrderLocation;
	bool dirtyPathing;

	OrderActions action;
	offsetPtr nextExecutionOrder;
	offsetPtr prevExecutionOrder;

	offsetPtr nextValidOrder;
	offsetPtr prevValidOrder;



	




} typedef Order;

enum OrderManupAction
{	
	OrderManupAction_NONE,
	OrderManupAction_Delete,
	OrderManupAction_MoveUp,
	OrderManupAction_MoveDown,
}typedef OrderManupAction;














enum EditorTools
{
	EditorTools_None,
	EditorTools_Select,
	EditorTools_Delete,
	EditorTools_Create,

	EditorTools_TileTargetSelect
} typedef EditorTools;



struct SynchronizedClientState {

	cl_int connected;
	cl_uint selectedPeepsLastIdx;

	offsetPtr selectedPeepPrimary;

	PeepRenderSupport peepRenderSupport[MAX_PEEPS];


	ge_int2 mouseGUIBegin; cl_uchar mouseOnGUiBegin;
	ge_int2 mouseGUIEnd;

	
	ge_int2 mouseWorldBegin_Q16;
	ge_int2 mouseWorldEnd_Q16;

	int mapZView;
	int mapZView_1;
	bool updateMap;

	EditorTools curTool;
	EditorTools curTool_1;

	MachineTypes curToolMachine;

	ge_short3 selectedMapCoord;
	offsetPtr selectedMachine;



	offsetPtr curEditingOrderPtr;
	bool curEditingOrder_targeting;
	ge_int3 tileTargetMapCoord;
	bool tileTargetFound;


	
	SyncedGui gui;

} typedef SynchronizedClientState;



#define HEAPSIZE (1024*100)

struct GameState {

	SynchronizedClientState clientStates[MAX_CLIENTS];
	cl_int numClients;


	Peep peeps[MAX_PEEPS];
	
	Particle particles[MAX_PARTICLES];

	MapExplorerAgent explorerAgents[GAME_UPDATE_WORKITEMS];
	
	Map map;
	MapSector sectors[SQRT_MAXSECTORS][SQRT_MAXSECTORS];
	
	AStarSearch_BFS mapSearchers[1];

	AStarJob mapSearchJobQueue[ASTAR_MAX_JOBS];
	offsetPtr curMapSearchJobPtr;
	offsetPtr lastMapSearchJobPtr;
	

	AStarPathSteps paths;

	Machine machines[MAX_MACHINES];
	int nextMachineIdx;
	MachineDesc machineDescriptions[MachineTypes_NUMTYPES];
	MachineRecipe machineRecipes[MachineRecipe_NUMRECIPES];
	MachineRecipes validMachineRecipes[MachineTypes_NUMTYPES][MachineRecipe_NUMRECIPES];
	TileUnion ItemTypeTiles[ItemTypes_NUMITEMS];
	float3 ItemColors[ItemTypes_NUMITEMS];


	Order orders[MAX_ORDERS];
	int nextOrderIdx;
	

	int s[32];

	//------------------------------------------------------not synced
	SyncedGui fakePassGui;
	GuiStyle guiStyle;
	cl_uint debugLinesIdx;

} typedef GameState;


#define ITEMTYPE_STRING_MAX_LENGTH (128)

struct StaticData {
	ge_int3 directionalOffsets[26];


}typedef StaticData;

__constant char ItemTypeStrings[ItemTypes_NUMITEMS][ITEMTYPE_STRING_MAX_LENGTH] = { 
	"Invalid",
    "Iron Ore",
    "Iron Dust",
    "Iron Bar",
    "Rock Dust"
  };
