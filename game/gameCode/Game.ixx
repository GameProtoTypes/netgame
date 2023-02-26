export module Game;



export import :Graphics;

import GE.Basic;
import GE.ImGui;
using namespace GE; 

export namespace Game {

const int MAX_ACTIONS_PER_TICK = 8;
const int MAX_CLIENTS = 1024;
const int GAME_UPDATE_WORKITEMS = 4;
const int warpSize = 32;
const int maxPeeps = 1024*1;
const int maxParticles = 32;
const int mapDim = 512;
const int mapDepth = 32;
const int mapTileSize = 5;
const int maxGuiRects = 1024*16;
const int maxLines = 1024*64;
const int maxExplorers = 1024;



constexpr ge_int MAP_TILE_SIZE = 5;
const int SQRT_MAXSECTORS = (mapDim/2);
const int SECTOR_SIZE = (10);






struct Cell;
struct MapSector;


enum PeepState_BitFlags
{
	PeepState_BitFlags_valid = 0,
	PeepState_BitFlags_deathState,
	PeepState_BitFlags_visible
};

#define PEEP_ORDER_STACK_SIZE (10)
struct PeepState_Basic
{
	ge_uint bitflags0;

	ge_int faction;

	ge_int health;
	ge_int deathState;


	ge_int buriedGlitchState;


	ge_offsetPtr orderPtrStack[PEEP_ORDER_STACK_SIZE];
	ge_int orderStackIdx;


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
	ge_int radius_Q16;
}typedef PhysicsCircleShape;






struct AStarPathNode
{
	bool valid;
	ge_int3 mapCoord_Q16;
	ge_offsetPtr nextOPtr;
	ge_offsetPtr prevOPtr;

	bool completed;
	bool processing;

}typedef AStarPathNode;

struct DrivePhysics
{
	ge_int target_x_Q16;
	ge_int target_y_Q16;
	ge_int target_z_Q16;

	ge_offsetPtr targetPathNodeOPtr;
	ge_offsetPtr prevPathNodeOPtr;

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
	ge_int orders_channel;
	ge_int message_TargetReached;
	ge_int message_TargetReached_pending;
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
	ge_int counts[ItemTypes_NUMITEMS];
} typedef Inventory;

struct Peep {
	ge_offsetPtr ptr;

	struct PeepState_Basic stateBasic;
	struct PeepPhysics physics;
	struct PeepCommunication comms;

	Inventory inventory;


	ge_int minDistPeep_Q16;
	ge_offsetPtr minDistPeepPtr;

	ge_offsetPtrShort3 mapCoord;
	ge_offsetPtrShort3 mapCoord_1;


	ge_int3 posMap_Q16;
	ge_int3 lastGoodPosMap_Q16;

	ge_offsetPtr2 sectorPtr;
	ge_int sectorListIdx;
	


	//selection by clients
	ge_offsetPtr nextSelectionPeepPtr[MAX_CLIENTS];
	ge_offsetPtr prevSelectionPeepPtr[MAX_CLIENTS];

} typedef Peep;






struct Triangle3D {
	ge_int3 verts_Q16[3];
} typedef Triangle3D;

struct Triangle3DHeavy{
	Triangle3D base;

	ge_int3 normal_Q16;
	ge_int3 u_Q16;
	ge_int3 v_Q16;

	ge_ubyte valid;
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
	MapTile_MACHINE_MINING_SITE,
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
	ge_uint data[mapDim][mapDim];

	ge_offsetPtr machinePtr[mapDim][mapDim];

	ge_uint peepCounts[mapDim][mapDim];
	ge_uint peepCounts_Final[mapDim][mapDim];
} typedef MapLevel;

struct Map {
	MapLevel levels[mapDepth];
	ge_int mapWidth;
	ge_int mapHeight;


} typedef Map;

const int MAX_PEEPS_PER_SECTOR  = (16);
struct MapSector {
	ge_offsetPtr peepPtrs[MAX_PEEPS_PER_SECTOR];
	ge_int ptrIterator;
	ge_offsetPtr2 ptr;
	ge_uint lock;
	bool empty;

	ge_int chunkStart;
} typedef MapSector;


struct AStarNode {
	ge_short3 tileIdx;
	ge_int h_Q16;
	ge_int g_Q16;

	ge_offsetPtr nextOPtr;
	ge_offsetPtr prevOPtr;

} typedef AStarNode;

enum AStarPathFindingProgress
{
	AStarPathFindingProgress_Ready,
	AStarPathFindingProgress_Searching,
	AStarPathFindingProgress_Finished,
	AStarPathFindingProgress_Failed,

	AStarPathFindingProgress_ResetReady


} typedef AStarPathFindingProgress;


const int ASTARHEAPSIZE = ((mapDim*mapDim*mapDepth)/100);
const int ASTARDETAILSSIZE = ((mapDim*mapDim*mapDepth)/100);
struct AStarSearch_BFS {
	AStarNode details[ASTARDETAILSSIZE];
	ge_offsetPtr nextDetailNodePtr;
	
	ge_offsetPtr openHeap_OPtrs[ASTARHEAPSIZE];
	ge_int openHeapSize;
	
	ge_offsetPtr endNodeOPtr;
	ge_offsetPtr startNodeOPtr;

	ge_offsetPtr closedMap[mapDim][mapDim][mapDepth];
	ge_offsetPtr openMap[mapDim][mapDim][mapDepth];
	
	AStarPathFindingProgress state;

	ge_offsetPtr pathOPtr;

} typedef AStarSearch_BFS;



const int ASTARPATHSTEPSSIZE = ((mapDim*mapDim*mapDepth)/10);
const int ASTAR_MAX_PATHS = (1024);
struct AStarPathSteps
{
	AStarPathNode pathNodes[ASTARPATHSTEPSSIZE];
	ge_int nextListIdx;

	ge_offsetPtr pathStarts[ASTAR_MAX_PATHS];
	ge_int nextPathStartIdx;

}typedef AStarPathSteps;


enum AStarJobStatus
{
	AStarJobStatus_Disabled,
	AStarJobStatus_Pending,
	AStarJobStatus_Working,
	AStarJobStatus_Done
} typedef AStarJobStatus;

const int ASTAR_MAX_JOBS = (128);
struct AStarJob
{
	ge_int3 startLoc;
	ge_int3 endLoc;

	AStarJobStatus status;
	ge_offsetPtr pathPtr;
} typedef AStarJob;




struct PeepRenderSupport {
	ge_int render_selectedByClient;
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
	ItemTypes inputTypes[8]; ge_int numInputs;
	ItemTypes outputTypes[8]; ge_int numOutputs;

	ge_int inputRatio[8];
	ge_int outputRatio[8];
} typedef MachineRecipe;

struct MachineDesc
{
	MachineTypes type;
	MapTile tile;



	ge_int processingTime;
} typedef MachineDesc;

enum MachineState
{
	MachineState_Idle,
	MachineState_Running,
	MachineState_Damaged
} typedef MachineState;




const int MAX_MACHINES = (1024*4);
struct Machine
{
	bool valid;
	ge_offsetPtrShort3 mapTilePtr;
	ge_offsetPtr MachineDescPtr;//ptr into bank of descriptions.

	ge_offsetPtr recipePtr;

	ge_int tickProgess;
	MachineState state;

	Inventory inventoryIn;
	Inventory inventoryOut;

	
	ge_offsetPtr rootOrderPtr;
	ge_int orderLen;



} typedef Machine;





const int MAX_ORDERS = (4096);
enum OrderActions
{
	OrderAction_NONE,
	OrderAction_MINE,
	OrderAction_DROPOFF_MACHINE,
	OrderAction_PICKUP_MACHINE,
	OrderAction_OPERATE_MACHINE,
	OrderAction_WAYPOINT,
	OrderAction_JUMP_TO_ORDER
	

} typedef OrderActions;


struct OrderActionData
{
	ge_offsetPtr jumpToOrderPtr;

};


struct Order
{
	ge_offsetPtr ptr;
	bool valid;
	bool pendingDelete;
	ge_int refCount;


	ge_int3 mapDest_Coord;
	bool destinationSet;
	ge_offsetPtr aStarJobPtr;
	ge_offsetPtr pathToDestPtr;
	bool selectingOrderLocation;
	bool dirtyPathing;

	OrderActions action;
	ge_offsetPtr nextExecutionOrder;
	ge_offsetPtr prevExecutionOrder;

	ge_offsetPtr nextValidOrder;
	ge_offsetPtr prevValidOrder;



	




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

	ge_int connected;
	ge_uint selectedPeepsLastIdx;

	ge_offsetPtr selectedPeepPrimary;

	PeepRenderSupport peepRenderSupport[maxPeeps];


	ge_int2 mouseGUIBegin; ge_ubyte mouseOnGUiBegin;
	ge_int2 mouseGUIEnd;

	
	ge_int2 mouseWorldBegin_Q16;
	ge_int2 mouseWorldEnd_Q16;

	ge_int mapZView;
	ge_int mapZView_1;
	bool updateMap;

	EditorTools curTool;
	EditorTools curTool_1;

	MachineTypes curToolMachine;

	ge_short3 selectedMapCoord;
	ge_offsetPtr selectedMachine;



	ge_offsetPtr curEditingOrderPtr;
	bool curEditingOrder_targeting;
	ge_int3 tileTargetMapCoord;
	bool tileTargetFound;


	
	SyncedGui gui;

} typedef SynchronizedClientState;



#define HEAPSIZE (1024*100)

struct GameState {

	SynchronizedClientState clientStates[MAX_CLIENTS];
	ge_int numClients;


	Peep peeps[maxPeeps];
	

	MapExplorerAgent explorerAgents[maxExplorers];
	
	Map map;
	MapSector sectors[SQRT_MAXSECTORS][SQRT_MAXSECTORS];
	
	AStarSearch_BFS mapSearchers[1];

	AStarJob mapSearchJobQueue[ASTAR_MAX_JOBS];
	ge_offsetPtr curMapSearchJobPtr;
	ge_offsetPtr lastMapSearchJobPtr;
	

	AStarPathSteps paths;

	Machine machines[MAX_MACHINES];
	ge_int nextMachineIdx;
	MachineDesc machineDescriptions[MachineTypes_NUMTYPES];
	MachineRecipe machineRecipes[MachineRecipe_NUMRECIPES];
	MachineRecipes validMachineRecipes[MachineTypes_NUMTYPES][MachineRecipe_NUMRECIPES];
	TileUnion ItemTypeTiles[ItemTypes_NUMITEMS];
	ge_float3 ItemColors[ItemTypes_NUMITEMS];


	Order orders[MAX_ORDERS];
	ge_int nextOrderIdx;
	

	ge_int s[32];

	//------------------------------------------------------not synced
	SyncedGui fakePassGui;
	GuiStyle guiStyle;
	ge_uint debugLinesIdx;

} typedef GameState;


#define ITEMTYPE_STRING_MAX_LENGTH (128)

struct StaticData {
	ge_int3 directionalOffsets[26];


}typedef StaticData;

const char ItemTypeStrings[ItemTypes_NUMITEMS][ITEMTYPE_STRING_MAX_LENGTH] = { 
	"Invalid",
    "Iron Ore",
    "Iron Dust",
    "Iron Bar",
    "Rock Dust"
  };

enum ClientActionCode {
	ClientActionCode_MouseStateChange,
	ClientActionCode_KeyStateChange,
	ClientActionCode_NONE = 255
} typedef ClientActionCode;

enum ClientActionCode_MouseStateChange_IntParams {
	CAC_MouseStateChange_Param_GUI_X,
	CAC_MouseStateChange_Param_GUI_Y,
	CAC_MouseStateChange_Param_WORLD_X_Q16,
	CAC_MouseStateChange_Param_WORLD_Y_Q16,
	CAC_MouseStateChange_Param_BUTTON_BITS,
	CAC_MAX//Move to largest enum as appropriate
};

enum ClientActionCode_KeyStateChange_IntParams {
	CAC_KeyStateChange_Param_KEY,
	CAC_KeyStateChange_Param_STATE
};


struct ClientAction {
	ge_uint scheduledTickIdx;//when it is scheduled to take effect on all clients



	ClientActionCode actionCode;
	int intParameters[CAC_MAX];//MAX of ClientActionCode_******_IntParams enums

} typedef ClientAction;


struct ActionTracking {

	ge_int clientId;

	ge_uint hostGivenId;
	ge_uint clientGivenId;
	ge_int ticksLate;//action could not be applied on client at scheduled tickId;
	bool finalActionVerified;
	bool clientApplied;
	ge_uint sentTickIdx;//when the action was sent by client

} typedef ActionTracking;



struct ActionWrap {
	ClientAction action;
	ActionTracking tracking;

} typedef ActionWrap;
struct GameStateActions
{
    ActionWrap clientActions[MAX_ACTIONS_PER_TICK];
	ge_int numActions;




	ge_uint tickIdx;
	ge_int pauseState;


	//ClientSide only stuff that is processed in ge but not strictly gamestate and driven by host.
	ge_uint clientId;



	ge_int mouseLocx;
	ge_int mouseLocy;
	ge_int mouseLocWorldx_Q16;
	ge_int mouseLocWorldy_Q16;
	ge_int mouseState;

	float viewMatrix[4][4];
	float viewMatrix_Inv[4][4];

	ge_int dummyVars[32];
};




void RunFrame();


}


