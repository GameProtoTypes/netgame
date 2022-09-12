#pragma once


#include "cl_type_glue.h"
#include "dynamicDefines.h"
#include "sizeTests.h"

#include "cpugpuvectortypes.h"

#define MAX_TRACKNODES (1024*8)

#define SQRT_MAXSECTORS (128)
#define SECTOR_SIZE (8)

#define MAX_PATHS (8096)

//#define MAX_CLIENTS (1024)

#define OFFSET_NULL (0xFFFFFFFF)
#define OFFSET_NULL_2D  ((offsetPtr2){0xFFFFFFFF , 0xFFFFFFFF})
#define OFFSET_NULL_3D  ((offsetPtr3){0xFFFFFFFF , 0xFFFFFFFF,  0xFFFFFFFF})

#define CL_CHECKED_ARRAY_SET(ARRAY, ARRAY_SIZE, INDEX, VALUE) { if(INDEX >= ARRAY_SIZE) {printf("[CL] OUT OF BOUNDS INDEX SET ON ARRAY "  #ARRAY " line %d \n", __LINE__); } else ARRAY[INDEX] = VALUE; }
#define CL_CHECKED_ARRAY_GET_PTR(ARRAY, ARRAY_SIZE, INDEX, POINTER) {if(INDEX >= ARRAY_SIZE) {printf("[CL] OUT OF BOUNDS INDEX GET ON ARRAY "  #ARRAY " line %d \n", __LINE__); POINTER = NULL;} else POINTER = &ARRAY[INDEX];}
#define CL_CHECK_NULL(POINTER){if(POINTER == NULL) {printf("[CL] " #POINTER " POINTER IS NULL line %d \n", __LINE__);}}

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

#pragma pack(push, 4)
struct PeepState_Basic
{
	cl_uint bitflags0;

	int32_t faction;

	cl_int health;
	cl_int deathState;


	cl_int buriedGlitchState;


}typedef PeepState_Basic;
#pragma pack(pop)


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

	offsetPtr nextPathNodeOPtr;

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


#pragma pack(push, 4)
struct Peep {
	offsetPtr ptr;

	struct PeepState_Basic stateBasic;
	struct PeepPhysics physics;
	struct PeepCommunication comms;

	cl_int minDistPeep_Q16;
	offsetPtr minDistPeepPtr;

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
#pragma pack(pop)


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




enum MapTileFlags {
	MapTileFlags_claimed = 10,
	MapTileFlags_miningInProgress,
	
	MapTileFlags_LowCornerTPLEFT,
	MapTileFlags_LowCornerTPRIGHT,
	MapTileFlags_LowCornerBTMRIGHT,
	MapTileFlags_LowCornerBTMLEFT,

	MapTileFlags_RotBit1,
	MapTileFlags_RotBit2,

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

#define ASTARHEAPSIZE ((MAPDIM*MAPDIM*MAPDEPTH)/10)
struct AStarSearch {
	AStarNode details[MAPDIM][MAPDIM][MAPDEPTH];
	
	offsetPtr3 openHeap_OPtrs[ASTARHEAPSIZE];
	cl_int openHeapSize;
	offsetPtr3 endNodeOPtr;
	offsetPtr3 startNodeOPtr;

	cl_uchar closedMap[MAPDIM][MAPDIM][MAPDEPTH];
	cl_uchar openMap[MAPDIM][MAPDIM][MAPDEPTH];
	
} typedef AStarSearch;



#define ASTARPATHSTEPSSIZE ((MAPDIM*MAPDIM*MAPDEPTH)/10)
struct AStarPathSteps
{
	AStarPathNode pathNodes[ASTARPATHSTEPSSIZE];
	int nextListIdx;

}typedef AStarPathSteps;




struct PeepRenderSupport {
	cl_int render_selectedByClient;
}typedef PeepRenderSupport;


struct ClientGuiState
{
	int guiRenderRectIdx;


	unsigned int guiStyle[RAYGUI_MAX_CONTROLS*(RAYGUI_MAX_PROPS_BASE + RAYGUI_MAX_PROPS_EXTENDED)];

 	BOOL guiStyleLoaded ;         // Style loaded flag for lazy style initialization
	GuiState guiState ;    // Gui global state, if !STATE_NORMAL, forces defined state

	Font guiFont ;                // Gui current font (WARNING: highly coupled to raylib)
	BOOL guiLocked ;              // Gui lock state (no inputs processed)
	float guiAlpha ;               // Gui element transpacency on drawing

	unsigned int guiIconScale;       // Gui icon default scale (if icons enabled)

} typedef ClientGuiState;


struct SynchronizedClientState {

	cl_int connected;
	cl_uint selectedPeepsLastIdx;

	PeepRenderSupport peepRenderSupport[MAX_PEEPS];

	ClientGuiState gui;

} typedef SynchronizedClientState;





struct GameState {

	SynchronizedClientState clientStates[MAX_CLIENTS];
	cl_int numClients;


	Peep peeps[MAX_PEEPS];
	
	Particle particles[MAX_PARTICLES];
	
	Map map;
	MapSector sectors[SQRT_MAXSECTORS][SQRT_MAXSECTORS];
	
	AStarSearch mapSearchers[1];
	AStarPathSteps paths;



} typedef GameState;


struct StaticData {
	ge_int3 directionalOffsets[26];

}typedef StaticData;
