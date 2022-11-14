

#include "clCommon.h"


//#define FLATMAP
#define ALL_EXPLORED
//#define NO_ZSHADING
//#define PEEP_ALL_ALWAYS_VISIBLE
//#define PEEP_DISABLE_TILECORRECTIONS
#define PEEP_PATH_CROWD (4)

#define DEBUG_PATHS

#include "clGUI.h"



RETURN_POINTER SynchronizedClientState* ThisClient(ALL_CORE_PARAMS)
{
    return &gameState->clientStates[gameStateActions->clientId];
}




void Print_GE_INT2(ge_int2 v)
{
    printf("{%d,%d}\n", v.x, v.y);
}
void Print_GE_INT3(ge_int3 v)
{
    printf("{%d,%d,%d}\n", v.x, v.y, v.z);
}
void Print_GE_UINT3(ge_uint3 v)
{
    printf("{%u,%u,%u}\n", v.x, v.y, v.z);
}
void Print_GE_SHORT3(ge_short3 v)
{
    printf("{%d,%d,%d}\n", v.x, v.y, v.z);
}
void PrintFloat2(float2 v)
{
    printf("{%f,%f}\n", v.x, v.y);
}
void PrintFloat3(float3 v)
{
    printf("{%f,%f,%f}\n", v.x, v.y, v.z);
}
void PrintFloat4(float4 v)
{
    printf("{%f,%f,%f,%f}\n", v.x, v.y, v.z, v.w);
}



inline cl_uint BITBANK_GET_SUBNUMBER_UINT(cl_uint bank, cl_int lsbBitIdx, cl_int numBits)
{
    cl_uint mask = 0;
    for (int i = 0; i < numBits; i++)
    {
        mask |= (1 << (i + lsbBitIdx));
    }


    return ((bank & mask) >> lsbBitIdx);
}


inline void BITBANK_SET_SUBNUMBER_UINT(cl_uint* bank, cl_int lsbBitIdx, cl_int numBits, cl_uint number)
{
    int i = 0;
    

    cl_uint mask = 0;
    for (int i = 0; i < numBits; i++)
    {
        mask |= (1 << (i + lsbBitIdx));
    }

    *bank &= ~mask;//clear the region
    *bank |= ((number << lsbBitIdx) & mask);
}



///----------------------------------------------------------------------------------------------------------------
///
/// 


inline MapTile MapDataGetTile(cl_uint tileData) {
    return (MapTile)BITBANK_GET_SUBNUMBER_UINT(tileData, 0, 8);
}
inline void MapDataSetTile(PARAM_GLOBAL_POINTER cl_uint* tileData, MapTile tile) {
    cl_uint tmp = *tileData;
    BITBANK_SET_SUBNUMBER_UINT(&tmp, 0, 8, tile);
    *tileData = tmp;
}

inline int MapTileGetRotation(cl_uint tileData) {
    return BITBANK_GET_SUBNUMBER_UINT(tileData, 8, 2);
}

RETURN_POINTER cl_uint* MapGetDataPointerFromCoord(ALL_CORE_PARAMS, ge_int3 mapcoord)
{
    return &(gameState->map.levels[(mapcoord).z].data[(mapcoord).x][(mapcoord).y]);
}

MapTile MapGetTileFromCoord(ALL_CORE_PARAMS, ge_int3 mapcoord)
{
    return MapDataGetTile(gameState->map.levels[(mapcoord).z].data[(mapcoord).x][(mapcoord).y]);
}

cl_uchar MapRidgeType(ALL_CORE_PARAMS, ge_int3 mapCoords, ge_int3 enterDir)
{
    USE_POINTER cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoords);

    if (MapDataGetTile(*data) == MapTile_NONE)
        return 0;


    /*offsets[0] = (ge_int3){ 1, 0, 0 };
    offsets[1] = (ge_int3){ -1, 0, 0 };
    offsets[2] = (ge_int3){ 0, -1, 0 };
    offsets[3] = (ge_int3){ 0, 1, 0 };
    offsets[4] = (ge_int3){ 0, 0, 1 };
    offsets[5] = (ge_int3){ 0, 0, -1 };*/

    if (VECTOR3_EQUAL(enterDir, staticData->directionalOffsets[0]))
    {
        return 2 - (BITGET_MF(*data, MapTileFlags_LowCornerBTMLEFT) + BITGET_MF(*data, MapTileFlags_LowCornerTPLEFT));
    }
    else if (VECTOR3_EQUAL(enterDir, staticData->directionalOffsets[1]))
    {
        return 2 - (BITGET_MF(*data, MapTileFlags_LowCornerBTMRIGHT) + BITGET_MF(*data, MapTileFlags_LowCornerTPRIGHT));
    }
    else if (VECTOR3_EQUAL(enterDir, staticData->directionalOffsets[2]))
    {
        return 2 - (BITGET_MF(*data, MapTileFlags_LowCornerBTMRIGHT) + BITGET_MF(*data, MapTileFlags_LowCornerBTMLEFT));
    }
    else if (VECTOR3_EQUAL(enterDir, staticData->directionalOffsets[3]))
    {
        return 2 - (BITGET_MF(*data, MapTileFlags_LowCornerTPRIGHT) + BITGET_MF(*data, MapTileFlags_LowCornerTPLEFT));
    }
    return 2;
}

cl_uchar MapDataHas2LowAdjacentCorners( cl_uint* data)
{
    if (BITGET_MF(*data, MapTileFlags_LowCornerTPLEFT) + BITGET_MF(*data, MapTileFlags_LowCornerTPRIGHT) == 2)
        return 1;

    if (BITGET_MF(*data, MapTileFlags_LowCornerTPRIGHT) + BITGET_MF(*data, MapTileFlags_LowCornerBTMRIGHT) == 2)
        return 2;

    if (BITGET_MF(*data, MapTileFlags_LowCornerBTMRIGHT) + BITGET_MF(*data, MapTileFlags_LowCornerBTMLEFT) == 2)
        return 3;

    if (BITGET_MF(*data, MapTileFlags_LowCornerBTMLEFT) + BITGET_MF(*data, MapTileFlags_LowCornerTPLEFT) == 2)
        return 4;

    return 0;
}

cl_uchar MapHas2LowAdjacentCorners(ALL_CORE_PARAMS, ge_int3 mapCoords)
{
    USE_POINTER cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoords);
    cl_uint localCopy = *data;
    return MapDataHas2LowAdjacentCorners(&localCopy);
}


cl_uchar MapTileDataHasLowCorner(cl_int tileData)
{
    return BITBANK_GET_SUBNUMBER_UINT(tileData, MapTileFlags_LowCornerTPLEFT, 4);
}


cl_uchar MapTileData_TileSolid(cl_int tileData)
{
    if(MapTileDataHasLowCorner(tileData) == 0 && MapDataGetTile(tileData) != MapTile_NONE)
    {
        return 1;
    }
    else 
        return 0;
}

cl_uchar MapTileData_PeepCount(cl_uint tileData)
{
    return BITBANK_GET_SUBNUMBER_UINT(tileData, MapTileFlags_PeepCount0, 3);
}
void MapTileData_SetPeepCount(cl_uint* tileData, cl_uchar peepCount)
{
    peepCount = clamp((int)peepCount,0, 7);
    BITBANK_SET_SUBNUMBER_UINT(tileData, MapTileFlags_PeepCount0, 3, peepCount);
}

cl_uchar MapHasLowCorner(ALL_CORE_PARAMS, ge_int3 mapCoords)
{
    USE_POINTER cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoords);

    if (MapDataGetTile(*data) == MapTile_NONE)
        return 0;

    return MapTileDataHasLowCorner(*data);
}
cl_uchar MapDataLowCornerCount(cl_int tileData)
{
    return (BITGET_MF(tileData, MapTileFlags_LowCornerTPLEFT) +
        BITGET_MF(tileData, MapTileFlags_LowCornerTPRIGHT) +
        BITGET_MF(tileData, MapTileFlags_LowCornerBTMRIGHT) +
        BITGET_MF(tileData, MapTileFlags_LowCornerBTMLEFT));
}
cl_uchar MapLowCornerCount(ALL_CORE_PARAMS, ge_int3 mapCoords)
{
    USE_POINTER cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoords);

    if (MapDataGetTile(*data) == MapTile_NONE)
        return 0;

    return MapDataLowCornerCount(*data);
}

cl_uchar MapDataXLevel( cl_uint* data)
{
    if(MapDataHas2LowAdjacentCorners(data) > 0 && MapDataLowCornerCount(*data) == 2)
    {
        return 1;//mid x
    }
    else if(MapDataLowCornerCount(*data) == 3)
    {
        return 0;//low x
    }
    else 
        return 2;//high x
}

cl_uchar MapTileXLevel(ALL_CORE_PARAMS, ge_int3 mapCoords)
{
    USE_POINTER cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoords);
    cl_uint localCopy = *data;
    return MapDataXLevel(&localCopy);
}

cl_uchar MapTileCoordStandInValid(ALL_CORE_PARAMS, ge_int3 mapcoord)
{
    USE_POINTER cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapcoord);
    MapTile tile = MapDataGetTile(*data);
    if (tile == MapTile_NONE)
    {
        return 1;
    }
    else
    {
        if(MapDataLowCornerCount(*data) > 0)
        {
            return 1;
            printf("hmmmm\n");
        }
    }
    return 0;
}


cl_uchar MapTileCoordEnterable(ALL_CORE_PARAMS, ge_int3 mapcoord, ge_int3 enterDirection)
{
    USE_POINTER cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapcoord);
    MapTile tile = MapDataGetTile(*data);
    if (tile == MapTile_NONE)
    {   
        return 1;
    }
    else
        return 0;

    //  if(MapDataLowCornerCount(*data) == 0)//cant enter full blocks
    // {
    //     return 0;
    // }
    // else if(enterDirection.z == 0)//entering partial block from the side
    // {
    //     cl_uchar ridgeType = MapRidgeType(ALL_CORE_PARAMS_PASS, mapcoord, enterDirection);
    //     if (ridgeType == 0)
    //         return 1;
    // }
    // else if(enterDirection.z > 0)//entering partial block from below
    // {
    //     return 1;
    //     ge_int3 dirNoZ = enterDirection;
    //     dirNoZ.z = 0;
    //     cl_uchar ridgeType = MapRidgeType(ALL_CORE_PARAMS_PASS, mapcoord, dirNoZ);
    //     if (ridgeType == 0)//running up continued ramp case.
    //         return 1;

    // }
    // else if(enterDirection.z < 0)//entering partial block from above
    // {
    //     return 1;
    //     if(MapDataLowCornerCount(*data) > 0)//could be a ramp 
    //         return 1;
    // }
    // return 0;
}


void Machine_InitRecipes(ALL_CORE_PARAMS)
{
   USE_POINTER MachineRecipe* recip = &gameState->machineRecipes[MachineRecipe_IRON_ORE_CRUSHING];
   recip->numInputs = 1;
   recip->numOutputs = 2;
   recip->inputTypes[0] = ItemType_IRON_ORE;
   recip->outputTypes[0] = ItemType_IRON_DUST;
   recip->outputTypes[1] = ItemType_ROCK_DUST;

   recip->inputRatio[0] = 1;
   recip->outputRatio[0] = 1;
   recip->outputRatio[1] = 5;



    recip = &gameState->machineRecipes[MachineRecipe_IRON_DUST_SMELTING];
    recip->numInputs = 1;
    recip->numOutputs = 1;
    recip->inputTypes[0] = ItemType_IRON_DUST;
    recip->outputTypes[0] = ItemType_IRON_BAR;

    recip->inputRatio[0] = 5;
    recip->outputRatio[0] = 1;


    gameState->validMachineRecipes[MachineTypes_CRUSHER][0] = MachineRecipe_IRON_ORE_CRUSHING;
    gameState->validMachineRecipes[MachineTypes_SMELTER][0] = MachineRecipe_IRON_DUST_SMELTING;



}

void Machine_InitDescriptions(ALL_CORE_PARAMS)
{
    USE_POINTER MachineDesc* m = &gameState->machineDescriptions[MachineTypes_CRUSHER];
    m->type = MachineTypes_CRUSHER;
    m->tile = MapTile_MACHINE_CRUSHER;
    m->processingTime = 30;


    m = &gameState->machineDescriptions[MachineTypes_SMELTER];
    m->type = MachineTypes_SMELTER;
    m->tile = MapTile_MACHINE_FURNACE;
    m->processingTime = 100;


    m = &gameState->machineDescriptions[MachineTypes_COMMAND_CENTER];
    m->type = MachineTypes_COMMAND_CENTER;
    m->tile = MapTile_MACHINE_COMMAND_CENTER;
    m->processingTime = 100;
}

void InitItemTypeTiles(ALL_CORE_PARAMS)
{
    gameState->ItemTypeTiles[ItemType_IRON_ORE].itemTile = ItemTile_Ore;
    gameState->ItemTypeTiles[ItemType_IRON_DUST].itemTile = ItemTile_Dust;
    gameState->ItemTypeTiles[ItemType_IRON_BAR].itemTile = ItemTile_Bar;
    gameState->ItemTypeTiles[ItemType_ROCK_DUST].itemTile = ItemTile_Dust;


    gameState->ItemColors[ItemType_IRON_ORE] = COLOR_ORANGE;
    gameState->ItemColors[ItemType_IRON_DUST] = COLOR_ORANGE;
    gameState->ItemColors[ItemType_IRON_BAR] = COLOR_ORANGE;
    gameState->ItemColors[ItemType_ROCK_DUST] = COLOR_WHITE;
}

offsetPtr Machine_CreateMachine(ALL_CORE_PARAMS)
{
    offsetPtr ptr = gameState->nextMachineIdx;
    
    bool loopSense = false;
    do
    {
        gameState->nextMachineIdx++;
        if(gameState->nextMachineIdx >= MAX_MACHINES)
        {
            gameState->nextMachineIdx = 0;
            if(loopSense == true)
            {
                printf("Machine_CreateMachine Out of Machine Space (MAX_MACHINES)!\n");
                return OFFSET_NULL;
            }
            loopSense=true;
        }
    }while(gameState->machines[gameState->nextMachineIdx].valid == true);

    USE_POINTER Machine* machine;
    OFFSET_TO_PTR(gameState->machines, ptr, machine);


    machine->rootOrderPtr = OFFSET_NULL;


    return ptr;
}





























inline void AStarNodeInstantiate(PARAM_GLOBAL_POINTER AStarNode* node)
{
    
    node->g_Q16 = TO_Q16(0);
    node->h_Q16 = TO_Q16(0);
    node->nextOPtr = OFFSET_NULL;  
    node->prevOPtr = OFFSET_NULL;
    node->tileIdx.x = -1;
    node->tileIdx.y = -1;
    node->tileIdx.z = -1;

}
void AStarInitPathNode(PARAM_GLOBAL_POINTER AStarPathNode* node)
{
    node->mapCoord_Q16 = (ge_int3){ 0,0,0 };
    node->nextOPtr = OFFSET_NULL;
    node->prevOPtr = OFFSET_NULL;

}


void AStarSearch_BFS_Instantiate(PARAM_GLOBAL_POINTER AStarSearch_BFS* search)
{
    for (int x = 0; x < MAPDIM; x++)
    {
        for (int y = 0; y < MAPDIM; y++)
        {
            for (int z = 0; z < MAPDEPTH; z++)
            {
                search->closedMap[x][y][z] = 0;
                search->openMap[x][y][z] = 0;
            }
        }  
    }

    for(int i = 0; i < ASTARHEAPSIZE; i++)
    {
        search->openHeap_OPtrs[i] = OFFSET_NULL;
    }
    
    search->nextDetailNodePtr = 0;

    search->startNodeOPtr = OFFSET_NULL;
    search->endNodeOPtr = OFFSET_NULL;
    
    search->openHeapSize = 0;
    search->pathOPtr = OFFSET_NULL;

}
void AStarSearch_BFS_InstantiateParrallel(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, cl_ulong idx, int x, int y, int z)
{


    search->closedMap[x][y][z] = 0;
    search->openMap[x][y][z] = 0;

    search->nextDetailNodePtr = 0;

    if(idx < ASTARHEAPSIZE)
    {
        search->openHeap_OPtrs[idx] = OFFSET_NULL;
    }
    
    



}




cl_uchar MapTileCoordValid(ge_int3 mapcoord, int xybuffer)
{
    if ((mapcoord.x >= xybuffer) && (mapcoord.y >= xybuffer) && mapcoord.z >= 0 && (mapcoord.x < MAPDIM-xybuffer) && (mapcoord.y < MAPDIM-xybuffer) && mapcoord.z < MAPDEPTH)
    {
        return 1;
    }
    return 0;
}
cl_uchar AStarNodeValid(PARAM_GLOBAL_POINTER AStarNode* node)
{
    return MapTileCoordValid(SHORT3_TO_INT3(node->tileIdx),1);
}
cl_uchar AStarNode2NodeTraversible(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER AStarNode* node, PARAM_GLOBAL_POINTER AStarNode* prevNode)
{  

    USE_POINTER cl_uint* fromTileData = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, SHORT3_TO_INT3(prevNode->tileIdx));
    USE_POINTER cl_uint* toTileData = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, SHORT3_TO_INT3(node->tileIdx));

    ge_int3 delta = INT3_SUB(SHORT3_TO_INT3(node->tileIdx), SHORT3_TO_INT3( prevNode->tileIdx ));
    if (MapTileCoordEnterable(ALL_CORE_PARAMS_PASS, SHORT3_TO_INT3(node->tileIdx), delta) == 0)
    {

        return 0;
    }





    return 1;




}

void MakeCardinalDirectionOffsets(PARAM_GLOBAL_POINTER ge_int3* offsets)
{
    offsets[0] = (ge_int3){ 1, 0, 0 };
    offsets[1] = (ge_int3){ -1, 0, 0 };
    offsets[2] = (ge_int3){ 0, -1, 0 };
    offsets[3] = (ge_int3){ 0, 1, 0 };
    offsets[4] = (ge_int3){ 0, 0, 1 };
    offsets[5] = (ge_int3){ 0, 0, -1 };

    offsets[6] = (ge_int3){ 1, 0, -1 };
    offsets[7] = (ge_int3){ 0, 1, -1 };
    offsets[8] = (ge_int3){ 1, 1, -1 };
    offsets[9] = (ge_int3){ -1, 0, -1 };
    offsets[10] = (ge_int3){ 0, -1, -1 };
    offsets[11] = (ge_int3){ -1, -1, -1 };
    offsets[12] = (ge_int3){ 1, -1, -1 };
    offsets[13] = (ge_int3){ -1, 1, -1 };

    offsets[14] = (ge_int3){ 1, 0, 1 };
    offsets[15] = (ge_int3){ 0, 1, 1 };
    offsets[16] = (ge_int3){ 1, 1, 1 };
    offsets[17] = (ge_int3){ -1, 0, 1 };
    offsets[18] = (ge_int3){ 0, -1, 1 };
    offsets[19] = (ge_int3){ -1, -1, 1 };
    offsets[20] = (ge_int3){ 1, -1, 1 };
    offsets[21] = (ge_int3){ -1, 1, 1 };

    offsets[22] = (ge_int3){ 1, 1, 0 };
    offsets[23] = (ge_int3){ -1, 1, 0 };
    offsets[24] = (ge_int3){ 1, -1, 0 };
    offsets[25] = (ge_int3){ -1, -1, 0 };
}
int AStarOpenHeapKey(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, PARAM_GLOBAL_POINTER AStarNode* node)
{
    //f
    return node->g_Q16 + node->h_Q16;
}

void AStarOpenHeapTrickleDown(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, cl_int index)
{
    cl_int largerChild;
    USE_POINTER AStarNode* top;
    offsetPtr topOPtr = search->openHeap_OPtrs[index];
    OFFSET_TO_PTR(search->details, topOPtr, top);


    while (index < search->openHeapSize / 2)
    {
        int leftChild = 2 * index + 1;
        int rightChild = leftChild + 1;

        USE_POINTER AStarNode* leftChildNode;
        OFFSET_TO_PTR(search->details, search->openHeap_OPtrs[leftChild], leftChildNode)
        USE_POINTER AStarNode* rightChildNode;
        OFFSET_TO_PTR(search->details, search->openHeap_OPtrs[rightChild], rightChildNode)

        if ((rightChild < search->openHeapSize) && (AStarOpenHeapKey(search, leftChildNode) > AStarOpenHeapKey(search, rightChildNode)))
            largerChild = rightChild;
        else
            largerChild = leftChild;

        USE_POINTER AStarNode* largerChildNode;

        OFFSET_TO_PTR(search->details, search->openHeap_OPtrs[largerChild], largerChildNode)

        if (AStarOpenHeapKey(search, top) <= AStarOpenHeapKey(search, largerChildNode))
            break;

        search->openHeap_OPtrs[index] = search->openHeap_OPtrs[largerChild];
        index = largerChild;

        
    }
    
    search->openHeap_OPtrs[index] = topOPtr;
}

offsetPtr AStarOpenHeapRemove(PARAM_GLOBAL_POINTER AStarSearch_BFS* search)
{
    offsetPtr rootOPtr = search->openHeap_OPtrs[0];

    search->openHeap_OPtrs[0] = search->openHeap_OPtrs[search->openHeapSize-1];
    search->openHeapSize--;

 
    AStarOpenHeapTrickleDown(search, 0);

    return rootOPtr;
}

offsetPtr AStarRemoveFromOpen(PARAM_GLOBAL_POINTER AStarSearch_BFS* search)
{

    offsetPtr nodeOPtr = AStarOpenHeapRemove(search);
    

    USE_POINTER AStarNode* node;
    OFFSET_TO_PTR(search->details, nodeOPtr, node);

    search->openMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z] = 0;
    return nodeOPtr;
}
void AStarAddToClosed(PARAM_GLOBAL_POINTER AStarSearch_BFS* search,PARAM_GLOBAL_POINTER AStarNode* node, offsetPtr ptr)
{
    search->closedMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z] = ptr;
}
bool AStarNodeInClosed(PARAM_GLOBAL_POINTER AStarSearch_BFS* search,PARAM_GLOBAL_POINTER AStarNode* node)
{
    return (bool)search->closedMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z];
}

bool AStarNodeInOpen(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, PARAM_GLOBAL_POINTER AStarNode* node)
{
    return (bool)search->openMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z];
}
bool AStarCoordInOpen(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, ge_short3 tile)
{
    return (bool)search->openMap[tile.x][tile.y][tile.z];
}


void AStarOpenHeapTrickleUp(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, cl_int index)
{
    cl_int prev = (index - 1) / 2;
    offsetPtr bottomOPtr = search->openHeap_OPtrs[index];

    USE_POINTER AStarNode* bottomNode;
    OFFSET_TO_PTR(search->details, bottomOPtr, bottomNode);

    USE_POINTER AStarNode* prevNode;
    OFFSET_TO_PTR(search->details, search->openHeap_OPtrs[prev], prevNode);

    while (index > 0 && AStarOpenHeapKey(search, prevNode) > AStarOpenHeapKey(search, bottomNode))
    {
        search->openHeap_OPtrs[index] = search->openHeap_OPtrs[prev];
        index = prev;
        prev = (prev - 1) / 2;
        OFFSET_TO_PTR(search->details, search->openHeap_OPtrs[prev], prevNode);
    }
    search->openHeap_OPtrs[index] = bottomOPtr;
}


void AStarOpenHeapInsert(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, offsetPtr nodeOPtr)
{
    search->openHeap_OPtrs[search->openHeapSize] = nodeOPtr;
    AStarOpenHeapTrickleUp(search, search->openHeapSize);
    search->openHeapSize++;
    if (search->openHeapSize > ASTARHEAPSIZE)
        printf("ERROR: AStarHeap Size Greater than ASTARHEAPSIZE!\n");

}
void AStarAddToOpen(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, offsetPtr nodeOPtr)
{
    AStarOpenHeapInsert(search, nodeOPtr);

    USE_POINTER AStarNode* node;
    OFFSET_TO_PTR(search->details, nodeOPtr, node);
    search->openMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z] = nodeOPtr;
}

offsetPtr AStarPathStepsNextFreePathNode(PARAM_GLOBAL_POINTER AStarPathSteps* list)
{
    offsetPtr ptr = list->nextListIdx;
    while ((list->pathNodes[ptr].nextOPtr != OFFSET_NULL))
    {
        ptr++;
        if (ptr >= ASTARPATHSTEPSSIZE)
            ptr = 0;
        
        if(ptr == list->nextListIdx)//wrap around
        {
            printf("No More Path Nodes Available.\n");
            return OFFSET_NULL;
        }
    }
    list->nextListIdx = ptr+1;

    return ptr;
}





void AStarRemoveFromClosed(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, PARAM_GLOBAL_POINTER AStarNode* node)
{
    search->closedMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z] = 0;
}

cl_int AStarNodeDistanceHuristic(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, PARAM_GLOBAL_POINTER AStarNode* nodeA, PARAM_GLOBAL_POINTER AStarNode* nodeB)
{
    return TO_Q16(abs(nodeA->tileIdx.x - nodeB->tileIdx.x) + abs(nodeA->tileIdx.y - nodeB->tileIdx.y) + abs(nodeA->tileIdx.z - nodeB->tileIdx.z));
}

void AStarPrintNodeStats(PARAM_GLOBAL_POINTER AStarNode* node)
{
    printf("Node: Loc: ");
    Print_GE_SHORT3(node->tileIdx);
    printf(" H: %f, G: %f\n", FIXED2FLTQ16(node->h_Q16), FIXED2FLTQ16(node->g_Q16));
}
void AStarPrintPathNodeStats(PARAM_GLOBAL_POINTER AStarPathNode* node)
{
    printf("Node: Loc: ");
    Print_GE_INT3_Q16(node->mapCoord_Q16);
}



void AStarPrintPath(PARAM_GLOBAL_POINTER AStarPathSteps* paths, offsetPtr startNodeOPtr)
{
    USE_POINTER AStarPathNode* curNode;
    OFFSET_TO_PTR(paths->pathNodes, startNodeOPtr, curNode);
    while (curNode != NULL)
    {
        AStarPrintPathNodeStats(curNode);
        OFFSET_TO_PTR(paths->pathNodes, curNode->nextOPtr, curNode);
    }
}

cl_uchar GE_INT3_ENTRY_COUNT(ge_int3 a)
{
    int s = 0;
    if (a.x != 0)
        s++;
    if (a.y != 0)
        s++;
    if (a.z != 0)
        s++;

    return s;
}

cl_uchar GE_INT3_SINGLE_ENTRY(ge_int3 a)
{
    int s = 0;
    if (a.x != 0)
        s++;
    if (a.y != 0)
        s++;
    if (a.z != 0)
        s++;

    if (s == 1)
        return 1;
    else
        return 0;
}


// Ret 1: [2,0,2],[-1,-1,0],[4,4,4],[3,0,0] etc
// Ret 0: [2,0,1],[1,-1,1],[2,3,4],[0,0,0] etc
cl_uchar GE_INT3_WHACHAMACOLIT1_ENTRY(ge_int3 a)
{
    if (INT3_ZERO(a))
        return 0;

    int n = a.x + a.y + a.z;
    int s =0;
    if (a.x != 0)
        s++;
    if (a.y != 0)
        s++;
    if (a.z != 0)
        s++;

    int f = 0;
    if (a.x != 0)
        f = a.x;
    else if (a.y != 0)
        f = a.y;
    else if (a.z != 0)
        f = a.z;


    if ((n / s) == f)
    {
        return 1;
    }
    return 0;
}


offsetPtr AStarFormPathSteps(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER AStarSearch_BFS* search,
 PARAM_GLOBAL_POINTER AStarPathSteps* steps,
 offsetPtr startingPathPtr
 )
{
    //start building the list .
    offsetPtr curNodeOPtr =  search->startNodeOPtr;
    USE_POINTER AStarNode* curNode;
    OFFSET_TO_PTR(search->details, curNodeOPtr, curNode);
    CL_CHECK_NULL(curNode);




    offsetPtr startNodeOPtr = OFFSET_NULL;
    USE_POINTER AStarPathNode* pNP = NULL;
    int i = 0;
    while (curNode != NULL)
    { 
        
        offsetPtr index;
        if(i == 0 && (startingPathPtr != OFFSET_NULL))
        {
            index = startingPathPtr;
        }
        else
            index = AStarPathStepsNextFreePathNode(&gameState->paths);

        USE_POINTER AStarPathNode* pN = &gameState->paths.pathNodes[index];
        CL_CHECK_NULL(pN);

        pN->valid = true;
        pN->processing = false;
        pN->completed = true;

        if (i == 0)
            startNodeOPtr = index;


        ge_int3 holdTileCoord = SHORT3_TO_INT3(  curNode->tileIdx  );

        //put location at center of tile
        ge_int3 tileCenter = GE_INT3_TO_Q16(holdTileCoord);
        tileCenter.x += TO_Q16(1) >> 1;
        tileCenter.y += TO_Q16(1) >> 1;
        tileCenter.z += TO_Q16(1) >> 1;


        pN->mapCoord_Q16 = tileCenter;

        if (pNP != NULL)
        {
            pNP->nextOPtr = index;
        }
        pNP = pN;

        if (curNode->nextOPtr != OFFSET_NULL)
        {

            OFFSET_TO_PTR(search->details, curNode->nextOPtr, curNode);

            if(0)
            {
                //iterate until joint in path.
                ge_int3 delta;
                USE_POINTER AStarNode* n2 = curNode;
                do
                {
                    OFFSET_TO_PTR(search->details, n2->nextOPtr, n2);

                    if (n2 != NULL) {
                        delta = INT3_ADD(SHORT3_TO_INT3(n2->tileIdx), INT3_NEG(holdTileCoord));
                    }
                    else
                        delta = (ge_int3){ 0,0,0 };

                } while ((n2 != NULL) && (GE_INT3_WHACHAMACOLIT1_ENTRY(delta) == 1));

                if (n2 != NULL) 
                {
                    USE_POINTER AStarNode* n2Prev;
                    OFFSET_TO_PTR(search->details, n2->prevOPtr, n2Prev);

                    if (curNode != n2Prev)
                        curNode = n2Prev;
                    else
                        curNode = n2;
                }
                else
                {
                    OFFSET_TO_PTR(search->details, search->endNodeOPtr, curNode);
                }
            }
        }
        else
            curNode = NULL;


        i++;
    }
    pNP->nextOPtr = OFFSET_NULL;



    //form prev links
    USE_POINTER AStarPathNode* curNode2;
    offsetPtr curNode2OPtr = startNodeOPtr;
    OFFSET_TO_PTR(steps->pathNodes, startNodeOPtr, curNode2);

    while (curNode2 != NULL)
    {

        USE_POINTER AStarPathNode* p;
        OFFSET_TO_PTR(steps->pathNodes, curNode2->nextOPtr, p);


        if (p != NULL)
            p->prevOPtr = curNode2OPtr;


        curNode2 = p;
        if(curNode2 != NULL)
            curNode2OPtr = curNode2->nextOPtr;
        
    }


    bool loopCheck = false;
    while(steps->pathStarts[steps->nextPathStartIdx] != OFFSET_NULL)
    {
        steps->nextPathStartIdx++;
        if(steps->nextPathStartIdx >= ASTAR_MAX_PATHS)
        {   
            if(loopCheck == true)
            {
                printf("Max Paths Reached!\n");
                break;
            }
            loopCheck = true;
            steps->nextPathStartIdx = 0;
        }
    }

    steps->pathStarts[steps->nextPathStartIdx] = startNodeOPtr;
    steps->nextPathStartIdx++;

   

  //  OFFSET_TO_PTR(steps->pathNodes, startNodeOPtr, curNode2);
   // curNode2->prevOPtr = OFFSET_NULL;

    return startNodeOPtr;
}

void AStarPathSteps_DeletePath(ALL_CORE_PARAMS, offsetPtr pathStartPtr)
{
    for(int i = 0 ; i < ASTAR_MAX_PATHS; i++)
    {
        if(gameState->paths.pathStarts[i] == pathStartPtr)
        {
            gameState->paths.pathStarts[i] = OFFSET_NULL;
        }
    }

    USE_POINTER AStarPathNode* pathNode;
    offsetPtr curPathPtr = pathStartPtr;
    do {
        OFFSET_TO_PTR(gameState->paths.pathNodes, curPathPtr, pathNode);
        curPathPtr = pathNode->nextOPtr;
        pathNode->valid = false;
        pathNode->nextOPtr = OFFSET_NULL;
        pathNode->prevOPtr = OFFSET_NULL;
    } while(curPathPtr != OFFSET_NULL);



}

offsetPtr AStarGrabDetailNode(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, PARAM_GLOBAL_POINTER AStarNode** node_out)
{
    offsetPtr ptr = search->nextDetailNodePtr;

    search->nextDetailNodePtr++;
    if(search->nextDetailNodePtr >= ASTARDETAILSSIZE)
    {
        printf("AStarGrabDetailNode Out of Nodes. (ASTARDETAILSSIZE)\n");
    }

    USE_POINTER AStarNode* node;
    OFFSET_TO_PTR(search->details, ptr, node);

    AStarNodeInstantiate(node);


    *node_out = node;
    

    return ptr;
}


AStarPathFindingProgress AStarSearch_BFS_Continue(ALL_CORE_PARAMS,PARAM_GLOBAL_POINTER AStarSearch_BFS* search, int iterations)
{
    USE_POINTER AStarNode* startNode;
    USE_POINTER AStarNode* targetNode;
    OFFSET_TO_PTR(search->details, search->startNodeOPtr,startNode);
    OFFSET_TO_PTR(search->details, search->endNodeOPtr,targetNode);


    //printf("AStarSearch_BFS_Continue..openHeapSize: %d\n", search->openHeapSize);
    while (search->openHeapSize > 0 && iterations > 0)
    {
        //printf("AStarSearch_BFS_Continue iterating..%d\n", iterations);
        //find node in open with lowest f cost
        offsetPtr currentOPtr = AStarRemoveFromOpen(search);


        USE_POINTER AStarNode* current;
        OFFSET_TO_PTR(search->details, currentOPtr, current);

        //printf("G: "); PrintQ16(current->g_Q16); printf(" H: "); PrintQ16(current->h_Q16);

        AStarAddToClosed(search, current, currentOPtr);//visited
        if (VECTOR3_EQUAL(current->tileIdx , targetNode->tileIdx) )
        {
            printf("AStarSearch_BFS_Continue AStarPathFindingProgress_Finished\n");
            printf("Details Array Size: %d\n", search->nextDetailNodePtr);
            printf("target node ptr: %d\n", search->endNodeOPtr);


            search->state = AStarPathFindingProgress_Finished;
            

            targetNode->nextOPtr = OFFSET_NULL;
            startNode->prevOPtr = OFFSET_NULL;

            targetNode->prevOPtr = current->prevOPtr;

            //form next links
            USE_POINTER AStarNode* curNode = targetNode;
            offsetPtr curNodeOPtr = search->endNodeOPtr;

            while (curNode != NULL)
            {
                USE_POINTER AStarNode* p;
                OFFSET_TO_PTR(search->details, curNode->prevOPtr, p);

                if(p != NULL)
                    p->nextOPtr = curNodeOPtr;

                curNodeOPtr = curNode->prevOPtr;
                curNode = p;

            }

            //form a simplified path
            AStarFormPathSteps(ALL_CORE_PARAMS_PASS , search, &gameState->paths, search->pathOPtr);


            return search->state;//found dest
        }
        
        



        //5 neighbors
        for (int i = 0; i <= 5; i++)
        { 
            ge_short3 prospectiveTileCoord;
            ge_int3 dir = staticData->directionalOffsets[i];
            prospectiveTileCoord.x = current->tileIdx.x + dir.x;
            prospectiveTileCoord.y = current->tileIdx.y + dir.y;
            prospectiveTileCoord.z = current->tileIdx.z + dir.z;
 
            if (MapTileCoordValid(VECTOR3_CAST(prospectiveTileCoord, ge_int3), 1)==0)
            {
                continue;
            }


            USE_POINTER AStarNode* prospectiveNode;
            offsetPtr prospectiveNodeOPtr;
            if(AStarCoordInOpen(search, prospectiveTileCoord))
            {
                prospectiveNodeOPtr = search->openMap[prospectiveTileCoord.x][prospectiveTileCoord.y][prospectiveTileCoord.z];
                OFFSET_TO_PTR(search->details, prospectiveNodeOPtr, prospectiveNode);
            }
            else
            {
                //printf("grabbing new prospective node\n");
                prospectiveNodeOPtr = AStarGrabDetailNode(search, &prospectiveNode);
                prospectiveNode->tileIdx = prospectiveTileCoord;
            }






            if ((AStarNode2NodeTraversible(ALL_CORE_PARAMS_PASS,  prospectiveNode, current) == 0) || (AStarNodeInClosed(search, prospectiveNode)))
            {

                if(VECTOR3_EQUAL(prospectiveNode->tileIdx , targetNode->tileIdx))
                {
                    printf("skippping obvious bullshit. %d, %d\n", AStarNode2NodeTraversible(ALL_CORE_PARAMS_PASS,  prospectiveNode, current), AStarNodeInClosed(search, prospectiveNode));
                }
                continue;
            }




            int totalMoveCost = current->g_Q16 + AStarNodeDistanceHuristic(search, current, prospectiveNode);
           // PrintQ16(totalMoveCost); PrintQ16(current->g_Q16); PrintQ16(prospectiveNode->g_Q16);
            if (((totalMoveCost < prospectiveNode->g_Q16) || !AStarNodeInOpen(search, prospectiveNode)) )
            {
                

                prospectiveNode->g_Q16 = totalMoveCost;
                prospectiveNode->h_Q16 = AStarNodeDistanceHuristic(search, prospectiveNode, targetNode);
                
                prospectiveNode->prevOPtr = currentOPtr;

                //printf("G: "); PrintQ16(prospectiveNode->g_Q16); printf("H: ");  PrintQ16(prospectiveNode->h_Q16);
                
                //printf("Adding pn %d, with prevoptr of %d\n", prospectiveNodeOPtr, prospectiveNode->prevOPtr);
                
                AStarAddToOpen(search, prospectiveNodeOPtr);
            }
        }

        iterations--;
    }

    
    if(search->openHeapSize > 0)
    {
        printf("AStarPathFindingProgress_Searching, openHeap Size: %d\n", search->openHeapSize);
        search->state = AStarPathFindingProgress_Searching;
        return search->state;

    }
    else
    {
        printf("AStarPathFindingProgress_Failed");
        search->state = AStarPathFindingProgress_Failed;
        return search->state;
    }

}

cl_uchar AStarSearch_BFS_Routine(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER AStarSearch_BFS* search, ge_int3 startTile, ge_int3 destTile, int startIterations, offsetPtr futurePathPtr)
{
    if (MapTileCoordValid(startTile,1) == 0)
    {
        printf("AStarSearch_BFS_Routine: Start MapCoord Not Valid.\n");
        return AStarPathFindingProgress_Failed;
    }
    if (MapTileCoordValid(destTile,1) == 0)
    {
        printf("AStarSearch_BFS_Routine: Dest MapCoord Not Valid.\n");
        return AStarPathFindingProgress_Failed;
    }
    if (MapTileCoordStandInValid(ALL_CORE_PARAMS_PASS, startTile) == 0)
    {
        printf("AStarSearch_BFS_Routine: Start Stand In Invalid.\n");
        return AStarPathFindingProgress_Failed;
    }
    if (MapTileCoordStandInValid(ALL_CORE_PARAMS_PASS, destTile)==0)
    {
        printf("AStarSearch_BFS_Routine: Dest Stand In Invalid.\n");
        return AStarPathFindingProgress_Failed;
    }

    printf("starting search from");
    search->state = AStarPathFindingProgress_Searching;
    


    USE_POINTER AStarNode* startNode;    
    {
    //grab a extra node so offset 0 isnt valid.
    AStarGrabDetailNode(search, &startNode);
    }
    search->startNodeOPtr = AStarGrabDetailNode(search, &startNode);
    startNode->tileIdx = (ge_short3){startTile.x,startTile.y,startTile.z};

    USE_POINTER AStarNode* endNode;
    search->endNodeOPtr = AStarGrabDetailNode(search, &endNode);
    endNode->tileIdx = (ge_short3){destTile.x,destTile.y,destTile.z};
    
    
    
    Print_GE_SHORT3(startNode->tileIdx); printf(" to "); Print_GE_SHORT3(endNode->tileIdx);
    
    search->pathOPtr = futurePathPtr;
    USE_POINTER AStarPathNode* path;
    OFFSET_TO_PTR(gameState->paths.pathNodes, search->pathOPtr, path);
    path->processing = true;



    //add start to openList
    startNode->h_Q16 = AStarNodeDistanceHuristic(search, startNode, endNode);
    AStarAddToOpen(search, search->startNodeOPtr );


    return AStarSearch_BFS_Continue(ALL_CORE_PARAMS_PASS, search, startIterations);
}






RETURN_POINTER cl_uint* AStarPathNode_GetMapData(ALL_CORE_PARAMS, AStarPathNode* node)
{
    ge_int3 coord;
    coord = GE_INT3_WHOLE_Q16(node->mapCoord_Q16);
    #ifdef DEBUG
    if(MapTileCoordValid(coord,1) == 0)
    {   
        CL_THROW_ASSERT();
        return NULL;
    }
    #endif

    return MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, coord);
}




//get the last path node from a node in a path
offsetPtr AStarPathNode_LastPathNode(PARAM_GLOBAL_POINTER AStarPathSteps* steps, offsetPtr pathNodeOPtr)
{
    offsetPtr curNodeOPtr = pathNodeOPtr;
    USE_POINTER AStarPathNode* curNode;
    OFFSET_TO_PTR(steps->pathNodes, curNodeOPtr, curNode);


    while(curNode->nextOPtr != OFFSET_NULL)
    {
        OFFSET_TO_PTR(steps->pathNodes, curNode->nextOPtr, curNode);
        curNodeOPtr = curNode->nextOPtr;
    }
    return curNodeOPtr;
}


offsetPtr AStarJob_EnqueueJob(ALL_CORE_PARAMS)
{
    offsetPtr ptr = gameState->lastMapSearchJobPtr;

    if(gameState->lastMapSearchJobPtr+1 == gameState->curMapSearchJobPtr)
    {
        printf("AStarJob_EnqueueJob: out of queue space (ASTAR_MAX_JOBS)");
        return ptr;
    }


    gameState->lastMapSearchJobPtr++;
    if(gameState->lastMapSearchJobPtr >= ASTAR_MAX_JOBS)
    {
        if(gameState->curMapSearchJobPtr == 0)
        {
            printf("AStarJob_EnqueueJob: out of queue space (ASTAR_MAX_JOBS)");
            return ptr;
        }
        gameState->lastMapSearchJobPtr = 0;
    }

    USE_POINTER AStarJob* job;
    OFFSET_TO_PTR(gameState->mapSearchJobQueue, ptr, job);
    job->status = AStarJobStatus_Pending;

    printf("created new job: %d\n", ptr);

    return ptr;
}

void AStarJob_UpdateJobs(ALL_CORE_PARAMS)
{
    USE_POINTER AStarJob* job;
    OFFSET_TO_PTR(gameState->mapSearchJobQueue, gameState->curMapSearchJobPtr, job);
    CL_CHECK_NULL(job);
    if(job->status == AStarJobStatus_Pending)
    {
        if(gameState->mapSearchers[0].state == AStarPathFindingProgress_Ready)
        {
            printf("Starting Search For AStarJob %d\n", gameState->curMapSearchJobPtr);

            AStarPathFindingProgress prog = AStarSearch_BFS_Routine(ALL_CORE_PARAMS_PASS, &gameState->mapSearchers[0],
            job->startLoc, job->endLoc, 1, job->pathPtr);

            if(prog == AStarPathFindingProgress_Failed)
            {
                job->status = AStarJobStatus_Done;
                printf("job failed\n");
            }
            else if(prog == AStarPathFindingProgress_Searching)
            {
                job->status = AStarJobStatus_Working;
            }
            else if(prog == AStarPathFindingProgress_Finished)
            {
                job->status = AStarJobStatus_Done;
                printf("job finished instantly\n");
            }
        }
        else
        {
            printf("cannot service job %d because pathfinder is not ready (%d)\n", gameState->curMapSearchJobPtr, gameState->mapSearchers[0].state);
        }
    } 
    
    if (job->status == AStarJobStatus_Working) 
    {
        if(gameState->mapSearchers[0].state == AStarPathFindingProgress_Finished)
        {
            job->pathPtr = gameState->mapSearchers[0].pathOPtr;
            job->status = AStarJobStatus_Done;

            printf("Job %d Done\n", gameState->curMapSearchJobPtr);


        }
        else if(gameState->mapSearchers[0].state == AStarPathFindingProgress_Failed)
        {
            printf("Job %d Failed\n", gameState->curMapSearchJobPtr);
            job->status = AStarJobStatus_Done;
        }
    }
    
    if (job->status == AStarJobStatus_Done)
    {
        printf("Job %d Finished, advancing..\n", gameState->curMapSearchJobPtr);
        job->status = AStarJobStatus_Disabled;

        //advance
        offsetPtr tmp = gameState->curMapSearchJobPtr;
        gameState->curMapSearchJobPtr++;
        if(gameState->curMapSearchJobPtr >= ASTAR_MAX_JOBS)
        {
            gameState->curMapSearchJobPtr = 0;
        }
    }



    //update AStarPath Searchers
    USE_POINTER AStarSearch_BFS* search = &gameState->mapSearchers[0];
    if(search->state == AStarPathFindingProgress_Searching)
    {
        AStarSearch_BFS_Continue(ALL_CORE_PARAMS_PASS, search, 100);
    }
    else if(	search->state == AStarPathFindingProgress_Finished ||
	search->state == AStarPathFindingProgress_Failed)
    {
        gameState->mapSearchers[0].state = AStarPathFindingProgress_ResetReady;

        
        gameState->mapSearchers[0].startNodeOPtr = OFFSET_NULL;
        gameState->mapSearchers[0].endNodeOPtr = OFFSET_NULL;
        gameState->mapSearchers[0].openHeapSize = 0;
        gameState->mapSearchers[0].pathOPtr = OFFSET_NULL;
    }
}












cl_uchar BaryCentric_In_Triangle_Q16(ge_int3 baryCoords)
{
    if (baryCoords.x >= 0 && baryCoords.x <= TO_Q16(1))
    {
        if (baryCoords.y >= 0 && baryCoords.y <= TO_Q16(1))
        {
            if (baryCoords.z >= 0 && baryCoords.z <= TO_Q16(1))
            {
                return 1;
            }
        }
    }
    return 0;
}


int SOME_INTERNAL_CORDIST(int x, int y)
{
    if (y <= 0.0)
    {
        return -y;
    }
    else if (x <= 0)
        return  x;
    else
        return 0;
}

ge_int3 Triangle3D_ToBaryCentric( Triangle3DHeavy* triangle, ge_int3 point)
{
    ge_int3 U = triangle->u_Q16;
    ge_int3 V = triangle->v_Q16;
    ge_int3 W = INT3_SUB(point, triangle->base.verts_Q16[0]);


    long d00 = GE_INT3_DOT_PRODUCT_Q16(U , U );
    long d01 = GE_INT3_DOT_PRODUCT_Q16(U , V);
    long d11 = GE_INT3_DOT_PRODUCT_Q16(V , V);
    long d20 = GE_INT3_DOT_PRODUCT_Q16(W , U);
    long d21 = GE_INT3_DOT_PRODUCT_Q16(W , V);
    long denom = MUL_PAD_Q16( d00 , d11 ) - MUL_PAD_Q16(d01 , d01);
    long u, v, w;
    v = DIV_PAD_Q16((MUL_PAD_Q16(d11 , d20) - MUL_PAD_Q16(d01 , d21)) , denom);
    w = DIV_PAD_Q16((MUL_PAD_Q16(d00 , d21) - MUL_PAD_Q16(d01 , d20)) , denom);
    u = (TO_Q16(1) - v ) - w;
    return (ge_int3) {
        u, v, w
    };
}

ge_int3 Triangle3DHeavy_ClosestPoint( Triangle3DHeavy* triangle, ge_int3 point_Q16, int* dist_Q16)
{   
    ge_int3 P1 = triangle->base.verts_Q16[0];
    ge_int3 P2 = triangle->base.verts_Q16[1];
    ge_int3 P3 = triangle->base.verts_Q16[2];

    
    //printf("P1: ");
    //Print_GE_INT3_Q16(P1);
    //printf("P2: ");
    //Print_GE_INT3_Q16(P2);
    //printf("P3: ");
    //Print_GE_INT3_Q16(P3);


    ge_int3 P_prime;
    ge_int3 P_prime_bary;

    int Nmag;
    ge_int3 N_n = GE_INT3_NORMALIZE_Q16(triangle->normal_Q16, &Nmag);
    //printf("N_n: ");
    //Print_GE_INT3_Q16(N_n);


    ge_int3 W = INT3_SUB(point_Q16, P1);

    //printf("W: ");
    //Print_GE_INT3_Q16(W);


    int dot = GE_INT3_DOT_PRODUCT_Q16(W, N_n);
    //printf("dot: ");
    //PrintQ16(dot);
    ge_int3 term2 = GE_INT3_SCALAR_MUL_Q16(dot, INT3_NEG(N_n));
    P_prime = INT3_ADD(point_Q16, term2);


    //Triangle2DHeavy_ProjectedPoint(triangle, point_Q16, &P_prime_bary, &P_prime);

    //printf("P_prime: ");
    //Print_GE_INT3_Q16(P_prime);

    P_prime_bary = Triangle3D_ToBaryCentric(triangle, P_prime);
    //printf("P_prime_bary: ");
    //Print_GE_INT3_Q16(P_prime_bary);
    cl_uchar onSurface = BaryCentric_In_Triangle_Q16(P_prime_bary);

    if (onSurface == 1)
    {
        *dist_Q16 = ge_length_v3_Q16(INT3_SUB(point_Q16, P_prime));

        return P_prime;
    }

    ge_int3 P1_P_prime = INT3_SUB(P_prime, P1);
    ge_int3 P2_P_prime = INT3_SUB(P_prime, P2);
    ge_int3 P3_P_prime = INT3_SUB(P_prime, P3);

    ge_int3 R1 = INT3_SUB(P1, P2);
    ge_int3 R2 = INT3_SUB(P2, P3);
    ge_int3 R3 = INT3_SUB(P3, P1);

    int R1_mag;
    ge_int3 R1_N = GE_INT3_NORMALIZE_Q16(R1, &R1_mag);
    int R2_mag;
    ge_int3 R2_N = GE_INT3_NORMALIZE_Q16(R2, &R2_mag);
    int R3_mag;
    ge_int3 R3_N = GE_INT3_NORMALIZE_Q16(R3, &R3_mag);


    ge_int3 R1_N_PERP = GE_VECTOR3_ROTATE_ABOUT_AXIS_POS90_Q16(R1_N, triangle->normal_Q16);
    ge_int3 R2_N_PERP = GE_VECTOR3_ROTATE_ABOUT_AXIS_POS90_Q16(R2_N, triangle->normal_Q16);
    ge_int3 R3_N_PERP = GE_VECTOR3_ROTATE_ABOUT_AXIS_POS90_Q16(R3_N, triangle->normal_Q16);


    int DOT1 = GE_INT3_DOT_PRODUCT_Q16(P1_P_prime, R3_N_PERP);
    int DOT2 = GE_INT3_DOT_PRODUCT_Q16(P2_P_prime, R1_N_PERP);
    int DOT3 = GE_INT3_DOT_PRODUCT_Q16(P3_P_prime, R2_N_PERP);


    ge_int3 D1R3 = GE_INT3_SCALAR_MUL_Q16(DOT1, R3_N_PERP);
    ge_int3 D2R1 = GE_INT3_SCALAR_MUL_Q16(DOT2, R1_N_PERP);
    ge_int3 D3R2 = GE_INT3_SCALAR_MUL_Q16(DOT3, R2_N_PERP);


    //p_prime projected to 3 edges
    ge_int3 P_prime_C1 = INT3_SUB(P_prime, D1R3);
    ge_int3 P_prime_C2 = INT3_SUB(P_prime, D2R1);
    ge_int3 P_prime_C3 = INT3_SUB(P_prime, D3R2);


    //clamp C points to edge limits
    int Z1i = GE_INT3_DOT_PRODUCT_Q16(P1_P_prime, R3_N);
    int Z2i = GE_INT3_DOT_PRODUCT_Q16(P2_P_prime, R1_N);
    int Z3i = GE_INT3_DOT_PRODUCT_Q16(P3_P_prime, R2_N);

    int Z1 = R3_mag - Z1i;
    int Z2 = R1_mag - Z2i;
    int Z3 = R2_mag - Z3i;


    int CD1 = SOME_INTERNAL_CORDIST(Z1, Z1i);
    int CD2 = SOME_INTERNAL_CORDIST(Z2, Z2i);
    int CD3 = SOME_INTERNAL_CORDIST(Z3, Z3i);


    ge_int3 J1 = GE_INT3_SCALAR_MUL_Q16(CD1, R3_N);
    ge_int3 J2 = GE_INT3_SCALAR_MUL_Q16(CD2, R1_N);
    ge_int3 J3 = GE_INT3_SCALAR_MUL_Q16(CD3, R2_N);

    ge_int3 L1 = INT3_ADD(J1, P_prime_C1);
    ge_int3 L2 = INT3_ADD(J2, P_prime_C2);
    ge_int3 L3 = INT3_ADD(J3, P_prime_C3);

    //get closest L to P_prime
    int L1D = ge_length_v3_Q16( INT3_SUB(L1, P_prime) );
    int L2D = ge_length_v3_Q16( INT3_SUB(L2, P_prime) );
    int L3D = ge_length_v3_Q16( INT3_SUB(L3, P_prime) );


    if (L1D < L2D && L1D < L2D)
    {
        *dist_Q16 = ge_length_v3_Q16(INT3_SUB(point_Q16, L1));

        return L1;
    }
    else if (L2D < L1D && L2D < L3D)
    {
        *dist_Q16 = ge_length_v3_Q16(INT3_SUB(point_Q16, L2));

        return L2;
    }
    else
    {
        *dist_Q16 = ge_length_v3_Q16(INT3_SUB(point_Q16, L3));

        return L3;
    }

}



void Triangle3DMakeHeavy( Triangle3DHeavy* triangle)
{
    triangle->u_Q16 = INT3_SUB(triangle->base.verts_Q16[1], triangle->base.verts_Q16[0]);//P_2 - P_1
    triangle->v_Q16 = INT3_SUB(triangle->base.verts_Q16[2], triangle->base.verts_Q16[0]);//P_3 - P_1


    triangle->normal_Q16 =  GE_INT3_CROSS_PRODUCT_Q16(triangle->u_Q16, triangle->v_Q16);


    if (GE_INT3_DOT_PRODUCT_Q16(triangle->normal_Q16, triangle->normal_Q16) < (TO_Q16(1) >> 5)){
        //printf("Warning! Small triangle!\n");
        triangle->valid = 0;
    }
    else{
        triangle->valid = 1;
    }

}
void Triangle3D_Make2Face( Triangle3DHeavy* triangle1,  Triangle3DHeavy* triangle2, ge_int3* fourCorners)
{
    triangle1->base.verts_Q16[0] = fourCorners[0];
    triangle1->base.verts_Q16[1] = fourCorners[1];
    triangle1->base.verts_Q16[2] = fourCorners[2];

    triangle2->base.verts_Q16[0] = fourCorners[2];
    triangle2->base.verts_Q16[1] = fourCorners[3];
    triangle2->base.verts_Q16[2] = fourCorners[0];

    Triangle3DMakeHeavy(triangle1);
    Triangle3DMakeHeavy(triangle2);
}



void MapTileConvexHull_From_TileData( ConvexHull* hull,  cl_uint* tileData)
{
    ge_int3 A = (ge_int3){ TO_Q16(-1) >> 1, TO_Q16(-1) >> 1, TO_Q16(-1) >> 1 };
    ge_int3 B = (ge_int3){ TO_Q16(1) >> 1, TO_Q16(-1) >> 1, TO_Q16(-1) >> 1 };
    ge_int3 C = (ge_int3){ TO_Q16(1) >> 1, TO_Q16(1) >> 1, TO_Q16(-1) >> 1 };
    ge_int3 D = (ge_int3){ TO_Q16(-1) >> 1, TO_Q16(1) >> 1, TO_Q16(-1) >> 1 };

    
    ge_int3 E = (ge_int3){ TO_Q16(-1) >> 1, TO_Q16(-1) >> 1, TO_Q16(1) >> 1 };
    ge_int3 F = (ge_int3){ TO_Q16(1) >> 1, TO_Q16(-1) >> 1, TO_Q16(1) >> 1 };
    ge_int3 G = (ge_int3){ TO_Q16(1) >> 1, TO_Q16(1) >> 1, TO_Q16(1) >> 1 };
    ge_int3 H = (ge_int3){ TO_Q16(-1) >> 1, TO_Q16(1) >> 1, TO_Q16(1) >> 1 };
    ge_int3 X = (ge_int3){ 0, 0, TO_Q16(1) >> 1 };

    cl_uchar lowCornerCount = MapDataLowCornerCount(*tileData);

    if (lowCornerCount > 0) 
    {
        if (BITGET_MF(*tileData, MapTileFlags_LowCornerBTMLEFT) != 0)
            H.z = (TO_Q16(-1) >> 1);

        if (BITGET_MF(*tileData, MapTileFlags_LowCornerBTMRIGHT) != 0)
            G.z = (TO_Q16(-1) >> 1);

        if (BITGET_MF(*tileData, MapTileFlags_LowCornerTPLEFT) != 0)
            E.z = (TO_Q16(-1) >> 1);

        if (BITGET_MF(*tileData, MapTileFlags_LowCornerTPRIGHT) != 0)
            F.z = (TO_Q16(-1) >> 1);


        //simple ramp cases
        uint xlevel = MapDataXLevel(tileData);
        if(xlevel == 0)
        {
            X.z = 0;
        }
        else if(xlevel == 1)
        {
            X.z = TO_Q16(-1) >> 1;
        }



    }


    int i = 0;
    //bottom (1)
    ge_int3 bottomFace[4];
    bottomFace[0] = A;
    bottomFace[1] = B;
    bottomFace[2] = C;
    bottomFace[3] = D;
    Triangle3D_Make2Face(&hull->triangles[i], &hull->triangles[i+1], &bottomFace[0]);
    i+=2;


    ge_int3 NegYFace[4];
    NegYFace[0] = A;
    NegYFace[1] = E;
    NegYFace[2] = F;
    NegYFace[3] = B;
    Triangle3D_Make2Face(&hull->triangles[i], &hull->triangles[i+1], &NegYFace[0]);
    i+=2;

    ge_int3 POSYFace[4];
    POSYFace[0] = C;
    POSYFace[1] = G;
    POSYFace[2] = H;
    POSYFace[3] = D;
    Triangle3D_Make2Face(&hull->triangles[i], &hull->triangles[i+1], &POSYFace[0]);
    i+=2;

    ge_int3 POSXFace[4];
    POSXFace[0] = B;
    POSXFace[1] = F;
    POSXFace[2] = G;
    POSXFace[3] = C;
    Triangle3D_Make2Face(&hull->triangles[i], &hull->triangles[i+1], &POSXFace[0]);
    i+=2;

    ge_int3 NEGXFace[4];
    NEGXFace[0] = D;
    NEGXFace[1] = H;
    NEGXFace[2] = E;
    NEGXFace[3] = A;
    Triangle3D_Make2Face(&hull->triangles[i], &hull->triangles[i+1], &NEGXFace[0]);
    i+=2;

    ge_int3 TOPFace1[4];
    TOPFace1[0] = X;
    TOPFace1[1] = G;
    TOPFace1[2] = F;
    TOPFace1[3] = E;
    Triangle3D_Make2Face(&hull->triangles[i], &hull->triangles[i+1], &TOPFace1[0]);
    i+=2;

    ge_int3 TOPFace2[4];
    TOPFace2[0] = X;
    TOPFace2[1] = E;
    TOPFace2[2] = H;
    TOPFace2[3] = G;
    Triangle3D_Make2Face(&hull->triangles[i], &hull->triangles[i+1], &TOPFace2[0]);
    i+=2;

}




void PeepPrint(Peep* peep)
{
    PrintQ16(peep->physics.drive.target_x_Q16);
}


void PeepPeepPhysics(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep, PARAM_GLOBAL_POINTER Peep* otherPeep)
{

    //calculate force based on penetration distance with otherPeep.
    ge_int3 d_Q16 = INT3_ADD(otherPeep->physics.base.pos_Q16, INT3_NEG(peep->physics.base.pos_Q16));
    cl_int combined_r_Q16 = peep->physics.shape.radius_Q16 + otherPeep->physics.shape.radius_Q16;
    cl_int len_Q16;


    ge_int3 penV_Q16 = d_Q16;
    ge_normalize_v3_Q16(&penV_Q16, &len_Q16);

    if (len_Q16 > peep->physics.shape.radius_Q16 * 2)
        return;

    cl_int penetrationDist_Q16 = (len_Q16 - (combined_r_Q16));
    ge_int3 penetrationForce_Q16;


    //pos_post_Q16
    peep->physics.base.pos_post_Q16.x += MUL_PAD_Q16(penV_Q16.x, penetrationDist_Q16 >> 2);
    peep->physics.base.pos_post_Q16.y += MUL_PAD_Q16(penV_Q16.y, penetrationDist_Q16 >> 2);

    //otherPeep->physics.base.pos_post_Q16.x -= MUL_PAD_Q16(penV_Q16.x, penetrationDist_Q16 >> 1);
    //otherPeep->physics.base.pos_post_Q16.y -= MUL_PAD_Q16(penV_Q16.y, penetrationDist_Q16 >> 1);

    //peep->physics.base.pos_post_Q16.z += MUL_PAD_Q16(penV_Q16.z, penetrationDist_Q16 >> 1);//dont encourage peeps standing on each other


    //V' = V - penV*(V.penV)
    //DeltaV = -penV*(V.penV)

    cl_int dot;
    ge_dot_product_3D_Q16(peep->physics.base.v_Q16, penV_Q16, &dot);
    if (dot > 0) {
        peep->physics.base.vel_add_Q16.x += -MUL_PAD_Q16(penV_Q16.x, dot);
        peep->physics.base.vel_add_Q16.y += -MUL_PAD_Q16(penV_Q16.y, dot);
        //peep->physics.base.vel_add_Q16.z += -MUL_PAD_Q16(penV_Q16.z, dot);//dont encourage peeps standing on each other

        //otherPeep->physics.base.vel_add_Q16.x -= -MUL_PAD_Q16(penV_Q16.x, dot);
        //otherPeep->physics.base.vel_add_Q16.y -= -MUL_PAD_Q16(penV_Q16.y, dot);
    }

    //spread messages
    if (peep->comms.orders_channel == otherPeep->comms.orders_channel)
    {
        if (otherPeep->comms.message_TargetReached)
        {
            peep->comms.message_TargetReached_pending = otherPeep->comms.message_TargetReached;
        }
    }

}

void PeepToPeepInteraction(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep, PARAM_GLOBAL_POINTER Peep* otherPeep)
{
    if (peep->stateBasic.deathState != 0 || otherPeep->stateBasic.deathState != 0)
        return;


    cl_int dist_Q16 = ge_length_v3_Q16(INT3_ADD(peep->physics.base.pos_Q16, -otherPeep->physics.base.pos_Q16));

    if (dist_Q16 < peep->minDistPeep_Q16)
    {
        peep->minDistPeep_Q16 = dist_Q16;
        peep->minDistPeepPtr = otherPeep->ptr;
    }
    

    PeepPeepPhysics(ALL_CORE_PARAMS_PASS, peep, otherPeep);
}

void WorldToMap(ge_int3 world_Q16, ge_int3* out_map_tilecoords_Q16)
{

    ge_int3 b = { TO_Q16(MAP_TILE_SIZE) ,TO_Q16(MAP_TILE_SIZE) ,TO_Q16(MAP_TILE_SIZE) };
    ge_int3 map_coords_Q16 = DIV_v3_Q16(world_Q16, b);

    map_coords_Q16.x += (TO_Q16(MAPDIM) >> 1);//MAPDIM*0.5f
    map_coords_Q16.y += (TO_Q16(MAPDIM) >> 1);//MAPDIM*0.5f.
    *out_map_tilecoords_Q16 = map_coords_Q16;
}

void MapToWorld(ge_int3 map_tilecoords_Q16, ge_int3* world_Q16)
{
    ge_int3 b = { TO_Q16(MAP_TILE_SIZE) ,TO_Q16(MAP_TILE_SIZE) ,TO_Q16(MAP_TILE_SIZE) };

    map_tilecoords_Q16.x -= (TO_Q16(MAPDIM) >> 1);//MAPDIM*0.5f
    map_tilecoords_Q16.y -= (TO_Q16(MAPDIM) >> 1);//MAPDIM*0.5f

    *world_Q16 = GE_INT3_MUL_Q16(map_tilecoords_Q16, b);

}


void PeepGetMapTile(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep, ge_int3 offset, 
 MapTile* out_map_tile, 
 ge_int3* out_tile_world_pos_center_Q16,
  ge_int3* out_map_tile_coord_whole, 
   cl_uint* out_tile_data)
{
    (*out_map_tile_coord_whole).z = WHOLE_Q16(peep->posMap_Q16.z) + (offset.z);
    (*out_map_tile_coord_whole).x = WHOLE_Q16(peep->posMap_Q16.x) + (offset.x);
    (*out_map_tile_coord_whole).y = WHOLE_Q16(peep->posMap_Q16.y) + (offset.y);

    ge_int3 tileCoords_Q16;
    tileCoords_Q16.x = TO_Q16((* out_map_tile_coord_whole).x) + (TO_Q16(1) >> 1);//center of tile
    tileCoords_Q16.y = TO_Q16((* out_map_tile_coord_whole).y) + (TO_Q16(1) >> 1);//center of tile
    tileCoords_Q16.z = TO_Q16((* out_map_tile_coord_whole).z) + (TO_Q16(1) >> 1);//center of tile

    MapToWorld(tileCoords_Q16, out_tile_world_pos_center_Q16);


    if ((*out_map_tile_coord_whole).z < 0 || (*out_map_tile_coord_whole).z >= (MAPDEPTH))
    {
        *out_map_tile = MapTile_NONE;
        return;
    }
    if ((*out_map_tile_coord_whole).x < 0 || (*out_map_tile_coord_whole).x >= (MAPDIM))
    {
        *out_map_tile = MapTile_NONE;
        return;
    }
    if ((*out_map_tile_coord_whole).y < 0 || (*out_map_tile_coord_whole).y >= (MAPDIM))
    {
        *out_map_tile = MapTile_NONE;
        return;
    }
    
    *out_tile_data = gameState->map.levels[(*out_map_tile_coord_whole).z].data[(*out_map_tile_coord_whole).x][(*out_map_tile_coord_whole).y];
    *out_map_tile = MapDataGetTile(*out_tile_data);
    
}

void RegionCollision(PARAM_GLOBAL_POINTER cl_int* out_pen_Q16, cl_int radius_Q16, cl_int W, cl_int lr)
{
    if (W > 0 && lr == -1)//left outside
    {
        *out_pen_Q16 = -(radius_Q16 - W);
        *out_pen_Q16 = clamp(*out_pen_Q16, -(radius_Q16), 0);
    }
    else if (W < 0 && lr == 1)//right outside
    {
        *out_pen_Q16 = (radius_Q16 + W);
        *out_pen_Q16 = clamp(*out_pen_Q16, 0, radius_Q16);
    }
    else if (W > 0 && lr == 1)//right inside
    {
        *out_pen_Q16 = (W + radius_Q16);
        *out_pen_Q16 = clamp(*out_pen_Q16, 0, W + radius_Q16);
    }
    else if (W < 0 && lr == -1)//left inside
    {
        *out_pen_Q16 = (W - radius_Q16);
        *out_pen_Q16 = clamp(*out_pen_Q16, W - radius_Q16, 0);
    }
}




ge_int3 MapTileConvexHull_ClosestPointToPoint( ConvexHull* hull, ge_int3 point_Q16)
{
    int smallestDist_Q16 = TO_Q16(1000);
    ge_int3 closestPoint;
    for (int i = 0; i < 14; i++)
    {
        Triangle3DHeavy* tri = &hull->triangles[i];
        if(tri->valid == 0)
            continue;

        int dist_Q16;
        ge_int3 closest = Triangle3DHeavy_ClosestPoint(tri, point_Q16, &dist_Q16);
        //printf("Dist(%d): ", i);
        //PrintQ16(dist_Q16);
        if (dist_Q16 < smallestDist_Q16)
        {
            smallestDist_Q16 = dist_Q16;
            closestPoint = closest;
        }
    }
    //printf("Chosen Dist: ");
    //PrintQ16(smallestDist_Q16);
    //printf("Chosen Point: ");
    //Print_GE_INT3_Q16(closestPoint);
    return closestPoint;
}

cl_uchar MapTileConvexHull_PointInside( ConvexHull* hull, ge_int3 point)
{
    //check dot product of point to verts against normal of the triangle
    for (int i = 0; i < 14; i++)
    {
        Triangle3DHeavy* tri = &hull->triangles[i];
        if(tri->valid == 0)
            continue;

        for(int v = 0; v < 3; v++)
        {
            ge_int3 vert = hull->triangles[i].base.verts_Q16[v];
            
            ge_int3 point_vert = INT3_SUB(point, vert);
            //int mag;
            //ge_int3 point_vert_normalized = GE_INT3_NORMALIZE_Q16(point_vert, &mag);

            int dot = GE_INT3_DOT_PRODUCT_Q16(point_vert, hull->triangles[i].normal_Q16);
            if(dot <= 0)
                return 0;
        }
    }

    return 1;
}

void PeepMapTileCollisions(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep)
{

    //maptile collisions
    MapTile tiles[26];
    ge_int3 tileCenters_Q16[26];
    ge_int3 dummy;
    cl_uint tileDatas[26];


    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 0, 0 }, & tiles[0], & tileCenters_Q16[0],&dummy, &tileDatas[0]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 0, 0 }, & tiles[1], & tileCenters_Q16[1], &dummy, & tileDatas[1]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, -1, 0 }, & tiles[2], & tileCenters_Q16[2], &dummy, & tileDatas[2]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 1, 0 }, & tiles[3], & tileCenters_Q16[3], &dummy, & tileDatas[3]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 0, 1 }, & tiles[4], & tileCenters_Q16[4], &dummy, & tileDatas[4]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 0, -1 }, & tiles[5], & tileCenters_Q16[5], &dummy, & tileDatas[5]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 0, 0 }, & tiles[6], & tileCenters_Q16[6],&dummy, &tileDatas[6]);
    
    // {
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){0, 0, 0}, &tiles[6], &tileCenters_Q16[6], &dummy, &tileDatas[5]);

    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){1, 0, -1}, &tiles[6], &tileCenters_Q16[6], &dummy, &tileDatas[6]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){0, 1, -1}, &tiles[7], &tileCenters_Q16[7], &dummy, &tileDatas[7]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){1, 1, -1}, &tiles[8], &tileCenters_Q16[8], &dummy, &tileDatas[8]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){-1, 0, -1}, &tiles[9], &tileCenters_Q16[9], &dummy, &tileDatas[9]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){0, -1, -1}, &tiles[10], &tileCenters_Q16[10], &dummy, &tileDatas[10]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){-1, -1, -1}, &tiles[11], &tileCenters_Q16[11], &dummy, &tileDatas[11]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){1, -1, -1}, &tiles[12], &tileCenters_Q16[12], &dummy, &tileDatas[12]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){-1, 1, -1}, &tiles[13], &tileCenters_Q16[13], &dummy, &tileDatas[13]);

    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){1, 0, 1}, &tiles[14], &tileCenters_Q16[14], &dummy, &tileDatas[14]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){0, 1, 1}, &tiles[15], &tileCenters_Q16[15], &dummy, &tileDatas[15]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){1, 1, 1}, &tiles[16], &tileCenters_Q16[16], &dummy, &tileDatas[16]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){-1, 0, 1}, &tiles[17], &tileCenters_Q16[17], &dummy, &tileDatas[17]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){0, -1, 1}, &tiles[18], &tileCenters_Q16[18], &dummy, &tileDatas[18]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){-1, -1, 1}, &tiles[19], &tileCenters_Q16[19], &dummy, &tileDatas[19]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){1, -1, 1}, &tiles[20], &tileCenters_Q16[20], &dummy, &tileDatas[20]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){-1, 1, 1}, &tiles[21], &tileCenters_Q16[21], &dummy, &tileDatas[21]);

    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){1, 1, 0}, &tiles[22], &tileCenters_Q16[23], &dummy, &tileDatas[22]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){-1, 1, 0}, &tiles[23], &tileCenters_Q16[24], &dummy, &tileDatas[23]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){1, -1, 0}, &tiles[24], &tileCenters_Q16[25], &dummy, &tileDatas[24]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){-1, -1, 0}, &tiles[25], &tileCenters_Q16[26], &dummy, &tileDatas[25]);
    // }
    
    //printf("peep Pos: "); Print_GE_INT3_Q16(peep->physics.base.pos_Q16);
    ConvexHull hull;//hull for use below
    for (int i = 0; i < 7; i++)
    {
       
        MapTile tile = tiles[i];
        
        if (tile != MapTile_NONE)
        {
            ge_int3 futurePos;
            futurePos.x = (peep->physics.base.pos_Q16.x + peep->physics.base.pos_post_Q16.z) + peep->physics.base.v_Q16.x;
            futurePos.y = (peep->physics.base.pos_Q16.y + peep->physics.base.pos_post_Q16.y) + peep->physics.base.v_Q16.y;
            futurePos.z = (peep->physics.base.pos_Q16.z + peep->physics.base.pos_post_Q16.z) + peep->physics.base.v_Q16.z;

            ge_int3 nearestPoint;
            cl_uchar insideSolidRegion;
  
            
            ge_int3 peepPosLocalToHull_Q16 = INT3_SUB(futurePos, tileCenters_Q16[i]);

            peepPosLocalToHull_Q16 = GE_INT3_DIV_Q16(peepPosLocalToHull_Q16, (ge_int3) {
                TO_Q16(MAP_TILE_SIZE), TO_Q16(MAP_TILE_SIZE)
                    , TO_Q16(MAP_TILE_SIZE)
            });

            if(MapTileData_TileSolid(tileDatas[i]) == 0)
            {
                MapTileConvexHull_From_TileData(&hull, &tileDatas[i]);
                nearestPoint = MapTileConvexHull_ClosestPointToPoint(&hull, peepPosLocalToHull_Q16);
                insideSolidRegion = MapTileConvexHull_PointInside(&hull, peepPosLocalToHull_Q16);
            }
            else
            {
                nearestPoint.x = clamp(peepPosLocalToHull_Q16.x,-(TO_Q16(1)>>1), (TO_Q16(1)>>1));
                nearestPoint.y = clamp(peepPosLocalToHull_Q16.y,-(TO_Q16(1)>>1), (TO_Q16(1)>>1));
                nearestPoint.z = clamp(peepPosLocalToHull_Q16.z,-(TO_Q16(1)>>1), (TO_Q16(1)>>1));

                insideSolidRegion = false;
                if(nearestPoint.x > -(TO_Q16(1)>>1) && nearestPoint.x < (TO_Q16(1)>>1))
                    if(nearestPoint.y > -(TO_Q16(1)>>1) && nearestPoint.y < (TO_Q16(1)>>1))
                        if(nearestPoint.z > -(TO_Q16(1)>>1) && nearestPoint.z < (TO_Q16(1)>>1))
                            insideSolidRegion = true;
            }


            peep->stateBasic.buriedGlitchState = insideSolidRegion;

            nearestPoint = GE_INT3_MUL_Q16(nearestPoint, (ge_int3) {
                TO_Q16(MAP_TILE_SIZE), TO_Q16(MAP_TILE_SIZE)
                    , TO_Q16(MAP_TILE_SIZE)
            });

            nearestPoint = INT3_ADD(nearestPoint, tileCenters_Q16[i]);
        

            ge_int3 A;
            A.x = futurePos.x - nearestPoint.x;
            A.y = futurePos.y - nearestPoint.y;
            A.z = futurePos.z - nearestPoint.z;

            //make A vector always point to outside the shape
            if(insideSolidRegion==1){
               A = INT3_NEG(A);
               //printf("inside region!");
            }


            ge_int3 An = A;
            cl_int mag;
            ge_normalize_v3_Q16(&An, &mag);


            if (mag < peep->physics.shape.radius_Q16)
            {
                cl_int dot;
                ge_int3 V = INT3_ADD(peep->physics.base.v_Q16 , peep->physics.base.vel_add_Q16);
                ge_dot_product_3D_Q16( V, An, &dot);
                ge_int3 B;//velocity to cancel
                B.x = MUL_PAD_Q16(An.x, dot);
                B.y = MUL_PAD_Q16(An.y, dot);
                B.z = MUL_PAD_Q16(An.z, dot);


                int pushAmt;
                if(insideSolidRegion)
                    pushAmt = (peep->physics.shape.radius_Q16 + mag);
                else
                    pushAmt = (peep->physics.shape.radius_Q16 - mag);

                //corrections
                peep->physics.base.pos_post_Q16.z += MUL_PAD_Q16(An.z, pushAmt);
                peep->physics.base.pos_post_Q16.y += MUL_PAD_Q16(An.y, pushAmt);
                peep->physics.base.pos_post_Q16.x += MUL_PAD_Q16(An.x, pushAmt);

                
                if((-B.z) < peep->physics.base.vel_add_Q16.z || (-B.z) > peep->physics.base.vel_add_Q16.z)
                    peep->physics.base.vel_add_Q16.z += -B.z;
                    
                if ((-B.y) < peep->physics.base.vel_add_Q16.y || (-B.y) > peep->physics.base.vel_add_Q16.y)
                    peep->physics.base.vel_add_Q16.y += -B.y;

                if ((-B.x) < peep->physics.base.vel_add_Q16.x || (-B.x) > peep->physics.base.vel_add_Q16.x)
                    peep->physics.base.vel_add_Q16.x += -B.x;
                


            }

        }
    }

    //'game gravity'
    //peep->physics.base.v_Q16.z += (TO_Q16(-1) >> 3);
}

void Peep_DetachFromOrder(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep)
{
    if(peep->stateBasic.orderPtr != OFFSET_NULL)
    {
        USE_POINTER Order* order;
        OFFSET_TO_PTR(gameState->orders, peep->stateBasic.orderPtr , order);
        order->refCount--;
        peep->stateBasic.orderPtr = OFFSET_NULL;
    }


    peep->physics.drive.targetPathNodeOPtr = OFFSET_NULL;
}

void Peep_AssignOrder(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep, offsetPtr orderPtr)
{
    if(peep->stateBasic.orderPtr != OFFSET_NULL)
        Peep_DetachFromOrder(ALL_CORE_PARAMS_PASS, peep);

    peep->stateBasic.orderPtr = orderPtr;
    peep->stateBasic.orderInProgress = true;

    USE_POINTER Order* order;
    OFFSET_TO_PTR(gameState->orders, orderPtr, order);

    peep->physics.drive.targetPathNodeOPtr = order->pathToDestPtr;

}



void Order_DeleteOrder(ALL_CORE_PARAMS, offsetPtr orderPtr)
{

    printf("deleting order.\n");
    USE_POINTER Order* order;
    OFFSET_TO_PTR(gameState->orders, orderPtr, order);

    order->valid = false;
    order->action = OrderAction_NONE;
    order->pendingDelete = false;
    if(order->pathToDestPtr != OFFSET_NULL)
    {
        AStarPathSteps_DeletePath(ALL_CORE_PARAMS_PASS, order->pathToDestPtr);
        order->pathToDestPtr = OFFSET_NULL;
    }


    
    //patch surrounding execution orders
    if(order->nextExecutionOrder != OFFSET_NULL)
    {
        USE_POINTER Order* nextExecOrder;
        OFFSET_TO_PTR(gameState->orders, order->nextExecutionOrder, nextExecOrder);

        nextExecOrder->prevExecutionOrder = order->prevExecutionOrder;
        nextExecOrder->dirtyPathing = true;
    }

    if(order->prevExecutionOrder != OFFSET_NULL)
    {
        USE_POINTER Order* prevExecOrder;
        OFFSET_TO_PTR(gameState->orders, order->prevExecutionOrder, prevExecOrder);

        prevExecOrder->nextExecutionOrder = order->nextExecutionOrder;
        prevExecOrder->dirtyPathing = true;
    }

    //next new pointer can be this one
    gameState->nextOrderIdx = orderPtr;
}

USE_POINTER Machine* MachineGetFromMapCoord(ALL_CORE_PARAMS, ge_int3 mapCoord)
{
    offsetPtr ptr = gameState->map.levels[mapCoord.z].machinePtr[mapCoord.x][mapCoord.y];
    USE_POINTER Machine* machine;
    OFFSET_TO_PTR(gameState->machines, ptr, machine);

    return machine;
}

void MapBuildTileView(ALL_CORE_PARAMS, int x, int y)
{
    ge_int3 coord = (ge_int3){x,y,ThisClient(ALL_CORE_PARAMS_PASS)->mapZView };
    USE_POINTER cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, coord);
    MapTile tile = MapDataGetTile(*data);
    MapTile tileUp;
    if (ThisClient(ALL_CORE_PARAMS_PASS)->mapZView < MAPDEPTH-1)
    {
        coord.z = ThisClient(ALL_CORE_PARAMS_PASS)->mapZView + 1;
        USE_POINTER cl_uint* dataup = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, coord);


        tileUp = MapDataGetTile(*dataup);

    }
    else
    {
        tileUp = MapTile_NONE;
    }

    mapTile1AttrVBO[y * MAPDIM + x] = 0;


    //look down...

    int vz = 0;
    while (tile == MapTile_NONE)
    {
        data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, coord);
        tile = MapDataGetTile(*data);
        if (tile == MapTile_NONE) {
            coord.z--;
            #ifndef NO_ZSHADING
                vz++;
            #endif
        }
    }



    if(BITGET(*data, MapTileFlags_Explored) == 0)
    {
        mapTile1VBO[y * MAPDIM + x ] = MapTile_NONE;//or "Haze"
        return;
    }


    cl_uint finalAttr=0;

    finalAttr |= BITGET_MF(*data, MapTileFlags_LowCornerTPLEFT)       ;//A
    finalAttr |= BITGET_MF(*data, MapTileFlags_LowCornerTPRIGHT)  << 1;//B
    finalAttr |= BITGET_MF(*data, MapTileFlags_LowCornerBTMLEFT)  << 2;//C
    finalAttr |= BITGET_MF(*data, MapTileFlags_LowCornerBTMRIGHT) << 3;//D

    cl_uint dataCpy = *data;
    uint xlev = MapDataXLevel(&dataCpy);
    finalAttr |= (2-xlev) << 4;//X


    finalAttr |= (clamp(15-vz-1, 0, 15) << 6);


    mapTile1AttrVBO[ y * MAPDIM + x ] = finalAttr;
    mapTile1OtherAttrVBO[ y * MAPDIM + x ] |= BITBANK_GET_SUBNUMBER_UINT(*data, MapTileFlags_RotBit1, 2);
    
    if (tileUp != MapTile_NONE)//view obstructed by foottile above.
    {

        //if next to visible show it as "wall view"
        mapTile1AttrVBO[y * MAPDIM + x ] = 0;
        cl_uchar isWall = 0;
        int dirOffsets[8] = {0,1,2,3,22,23,24,25}; 
        int orthflags[4] = {0,0,0,0};
        for(int i = 0; i < 4; i++)
        {
            ge_int3 offset = staticData->directionalOffsets[dirOffsets[i]];
            ge_int3 mapCoord = (ge_int3){x +offset.x, y + offset.y, ThisClient(ALL_CORE_PARAMS_PASS)->mapZView + 1 };

            if(MapTileCoordValid(mapCoord,0))
            {
            
                USE_POINTER cl_uint* dataoffup = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoord);
                MapTile tileoffup = MapDataGetTile(*dataoffup);
                
                if(tileoffup == MapTile_NONE)
                {
                    isWall = 1;

                    //test against mouse world coord
                    // ge_int3 mouseMapCoord;
                    // int occluded;
                    // ge_int3 mouseWorld_Q16 = (ge_int3){gameStateActions->mouseLocWorldx_Q16, gameStateActions->mouseLocWorldy_Q16, mapCoord.z};
                   
                    // WorldToMap(mouseWorld_Q16, &mouseMapCoord);
                    // mouseMapCoord = GE_INT3_WHOLE_ONLY_Q16(mouseMapCoord);
                    
                    // if(VECTOR3_EQUAL(mouseMapCoord , mapCoord))
                    // {
                    //     mapTile1VBO[y * MAPDIM + x ] = tile;
                        
                    // }
                    // else
                    // {
                        mapTile1VBO[y * MAPDIM + x ] = tileUp;
                        
                    //}




                    if( i <=3 )
                        orthflags[i] = 1;

                    //fade out effect
                    


                    if((orthflags[0] + orthflags[1] + orthflags[2] + orthflags[3]) == 0)
                    {

                        //TODO better corner effect
                        // mapTile1AttrVBO[ y * MAPDIM + x ] |= 1<<4;

                        // if(dirOffsets[i] == 22) mapTile1AttrVBO[ y * MAPDIM + x ] |= 1 << 0;
                        // if(dirOffsets[i] == 23) mapTile1AttrVBO[ y * MAPDIM + x ] |= 1 << 1;
                        // if(dirOffsets[i] == 24) mapTile1AttrVBO[ y * MAPDIM + x ] |= 1 << 2;
                        // if(dirOffsets[i] == 25) mapTile1AttrVBO[ y * MAPDIM + x ] |= 1 << 3;
                    }


                    mapTile1AttrVBO[y * MAPDIM + x ] |= (clamp(15-vz, 0, 15) << 6);//base shade
                    mapTile1AttrVBO[y * MAPDIM + x ] |= (1 << 10);//corners fade to nothing


                    

                }
            }
        }




        if(isWall == 0)
            mapTile1VBO[y * MAPDIM + x ] = MapTile_NONE;
    }
    else
    {
        mapTile1VBO[y * MAPDIM + x] = tile;
    }
}

void MapBuildTileView3Area(ALL_CORE_PARAMS, int x, int y)
{
    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x,  y);
    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x+1,  y);
    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x,  y+1);
    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x+1,  y+1);

    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x-1,  y);
    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x,  y-1);
    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x-1,  y-1);

    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x-1,  y+1);
    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x+1,  y-1);
}

void MapUpdateShadow(ALL_CORE_PARAMS, int x, int y)
{
    if ((x < 1) || (x >= MAPDIM - 1) || (y < 1) || (y >= MAPDIM - 1))
    {
        return;
    }


    MapTile tile = MapTile_NONE;
    mapTile2VBO[y * MAPDIM + x] = MapTile_NONE;


    int shadowIntensity;
    for (int z = ThisClient(ALL_CORE_PARAMS_PASS)->mapZView+1; z >= 0; z--)
    {       
        if(z >= MAPDEPTH) continue;

        USE_POINTER cl_uint* data = &gameState->map.levels[z].data[x][y];
        MapTile center =MapDataGetTile(*data);

        if (center != MapTile_NONE)
            return;


        // b | c | d
        // e |cen| f
        // g | h | i
        //MapTile b = gameState->map.levels[z].data[x-1][y-1]; 
        //MapTile c = gameState->map.levels[z].data[x][y-1];
        ////MapTile d = gameState->map.levels[z].data[x+1][y-1];
        //MapTile e = gameState->map.levels[z].data[x-1][y];
        //MapTile f = gameState->map.levels[z].data[x + 1][y];
        ////MapTile g = gameState->map.levels[z].data[x - 1][y+1];
        //MapTile h = gameState->map.levels[z].data[x][y + 1];
        ////MapTile i = gameState->map.levels[z].data[x+1][y + 1];


        cl_uchar f = MapRidgeType(ALL_CORE_PARAMS_PASS, (ge_int3) { x + 1, y, z }, (ge_int3) { 1, 0, 0 });
        cl_uchar h = MapRidgeType(ALL_CORE_PARAMS_PASS, (ge_int3) { x , y+1, z }, (ge_int3) { 0, 1, 0 });
        cl_uchar e = MapRidgeType(ALL_CORE_PARAMS_PASS, (ge_int3) { x - 1, y, z }, (ge_int3) { -1, 0, 0 });
        cl_uchar c = MapRidgeType(ALL_CORE_PARAMS_PASS, (ge_int3) { x, y-1, z }, (ge_int3) { 0, -1, 0 });


        if ((f != 0) && (c == 0) && (e == 0) && (h == 0))
            tile = MapTile_Shadow_0;

        if ((f == 0) && (c == 0) && (e != 0) && (h == 0))
            tile = MapTile_Shadow_2;

        if ((f == 0) && (c != 0) && (e == 0) && (h == 0))
            tile = MapTile_Shadow_1;

        if ((f == 0) && (c == 0) && (e == 0) && (h != 0))
            tile = MapTile_Shadow_3;

        //-------------

        if ((f != 0) && (c != 0) && (e == 0) && (h == 0))
            tile = MapTile_Shadow_5;

        if ((f == 0) && (c != 0) && (e != 0) && (h == 0))
            tile = MapTile_Shadow_6;

        if ((f == 0) && (c == 0) && (e != 0) && (h != 0))
            tile = MapTile_Shadow_7;

        if ((f != 0) && (c == 0) && (e == 0) && (h != 0))
            tile = MapTile_Shadow_4;

        //-------------------------

        if ((f != 0) && (c != 0) && (e != 0) && (h == 0))
            tile = MapTile_Shadow_14;

        if ((f != 0) && (c != 0) && (e == 0) && (h != 0))
            tile = MapTile_Shadow_15;

        if ((f != 0) && (c == 0) && (e != 0) && (h != 0))
            tile = MapTile_Shadow_12;

        if ((f == 0) && (c != 0) && (e != 0) && (h != 0))
            tile = MapTile_Shadow_10;
        //----------------------------
        if ((f != 0) && (c == 0) && (e != 0) && (h == 0))
            tile = MapTile_Shadow_16;

        if ((f == 0) && (c != 0) && (e == 0) && (h != 0))
            tile = MapTile_Shadow_8;

        //------------------------------
        if ((f != 0) && (c != 0) && (e != 0) && (h != 0))
            tile = MapTile_Shadow_11;


        mapTile2VBO[y * MAPDIM + x] = tile;
        
        cl_uint finalAttr = (clamp(15-0, 0, 15) << 6);
        mapTile2AttrVBO[ y * MAPDIM + x ] = finalAttr;
    }


}

void MapDeleteTile(ALL_CORE_PARAMS, ge_int3 mapCoord)
{
    if (mapCoord.z > 0) 
    {
        gameState->map.levels[mapCoord.z].data[mapCoord.x][mapCoord.y] = MapTile_NONE;
        

        offsetPtr machinePtr = gameState->map.levels[mapCoord.z].machinePtr[mapCoord.x][mapCoord.y];
        if(machinePtr != OFFSET_NULL)
        {
            USE_POINTER Machine* mach;
            OFFSET_TO_PTR(gameState->machines, machinePtr, mach);
            mach->valid = false;
        }
        gameState->map.levels[mapCoord.z].machinePtr[mapCoord.x][mapCoord.y] = OFFSET_NULL;


        MapBuildTileView3Area(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y);
        MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y);
        MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x + 1, mapCoord.y);
        MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x - 1, mapCoord.y);
        MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y + 1);
        MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y - 1);
    }
}

void PeepDrill(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep)
{
    ge_int3 mapCoord = SHORT3_TO_INT3(peep->mapCoord);
    mapCoord.z--;

    MapTile downTile = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, mapCoord);
    USE_POINTER Machine* machine = MachineGetFromMapCoord(ALL_CORE_PARAMS_PASS, mapCoord);

    printf("digging...\n");

    if(downTile == MapTile_IronOre)
    {
        peep->inventory.counts[ItemType_IRON_ORE]+=10;
    }
    else if(downTile == MapTile_Rock)
    {
        peep->inventory.counts[ItemType_ROCK_DUST]+=100;
    }


    MapDeleteTile(ALL_CORE_PARAMS_PASS,  mapCoord);
}
void TransferInventory(PARAM_GLOBAL_POINTER Inventory* invFrom, PARAM_GLOBAL_POINTER Inventory* invTo)
{
    for(int i = 0; i < ItemTypes_NUMITEMS; i++)
    {
        invTo->counts[i] += invFrom->counts[i];
        invFrom->counts[i] = 0;
    }
}

void PeepPull(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep)
{
    ge_int3 mapCoord = SHORT3_TO_INT3(peep->mapCoord);
    mapCoord.z--;

    MapTile downTile = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, mapCoord);
    USE_POINTER Machine* machine = MachineGetFromMapCoord(ALL_CORE_PARAMS_PASS, mapCoord);

    TransferInventory(&machine->inventoryOut, &peep->inventory);
}
void PeepPush(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep)
{
    ge_int3 mapCoord = SHORT3_TO_INT3(peep->mapCoord);
    mapCoord.z--;

    MapTile downTile = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, mapCoord);
    USE_POINTER Machine* machine = MachineGetFromMapCoord(ALL_CORE_PARAMS_PASS, mapCoord);

    TransferInventory(&peep->inventory, &machine->inventoryIn);
}

void PeepDrivePhysics(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep)
{

    ge_int3 targetVelocity;

    ge_int3 d;
    d.x = peep->physics.drive.target_x_Q16 - peep->physics.base.pos_Q16.x;
    d.y = peep->physics.drive.target_y_Q16 - peep->physics.base.pos_Q16.y;
    d.z = peep->physics.drive.target_z_Q16 - peep->physics.base.pos_Q16.z;
    int len;
    ge_normalize_v3_Q16(&d, &len);

    if (len < TO_Q16(1))
    {
        d.x = MUL_PAD_Q16(d.x, len);
        d.y = MUL_PAD_Q16(d.y, len);
        d.z = MUL_PAD_Q16(d.z, len);
    }

    if(peep->stateBasic.orderPtr != OFFSET_NULL)
    {
        USE_POINTER Order* order;
        OFFSET_TO_PTR(gameState->orders, peep->stateBasic.orderPtr, order);

        if(!order->valid)
            Peep_DetachFromOrder(ALL_CORE_PARAMS_PASS, peep);
    

        if (WHOLE_Q16(len) < 2)//within range of current target
        {



            if(peep->physics.drive.targetPathNodeOPtr != OFFSET_NULL)
            {

                //advance if theres room
                USE_POINTER AStarPathNode* targetPathNode;
                OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.targetPathNodeOPtr,targetPathNode);
                CL_CHECK_NULL(targetPathNode);

                USE_POINTER AStarPathNode* prevpathNode;
                OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.prevPathNodeOPtr,prevpathNode);

                
                //advance
                if(targetPathNode->completed)
                {
                    if(targetPathNode->nextOPtr != OFFSET_NULL)
                    {
                        peep->physics.drive.prevPathNodeOPtr = peep->physics.drive.targetPathNodeOPtr;        
                        peep->physics.drive.targetPathNodeOPtr = targetPathNode->nextOPtr;

                        if (peep->physics.drive.targetPathNodeOPtr != OFFSET_NULL ) 
                        {
                            USE_POINTER AStarPathNode* targetPathNode;
                            OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.targetPathNodeOPtr,targetPathNode);
                            CL_CHECK_NULL(targetPathNode);

                            ge_int3 nextTarget_Q16 = targetPathNode->mapCoord_Q16;
                            MapToWorld(nextTarget_Q16, &nextTarget_Q16);

                            peep->physics.drive.target_x_Q16 = nextTarget_Q16.x;
                            peep->physics.drive.target_y_Q16 = nextTarget_Q16.y;
                            peep->physics.drive.target_z_Q16 = nextTarget_Q16.z;
                        }
                    }
                    else
                    {
                        //final dest reached.
                        peep->comms.message_TargetReached_pending = 255;//send the message

                        if( order != NULL )
                        {

                            //do the order action
                            if(order->action == OrderAction_MINE)
                            {
                                PeepDrill(ALL_CORE_PARAMS_PASS, peep);
                            }
                            else if(order->action == OrderAction_DROPOFF_MACHINE)
                            {
                                PeepPush(ALL_CORE_PARAMS_PASS, peep);
                            }
                            else if(order->action == OrderAction_PICKUP_MACHINE)
                            {
                                PeepPull(ALL_CORE_PARAMS_PASS, peep);
                            }
                            



                            //delete single orders
                            if((order->nextExecutionOrder == OFFSET_NULL && order->prevExecutionOrder == OFFSET_NULL))
                            {
                                printf("peep detach from order\n");
                                Peep_DetachFromOrder(ALL_CORE_PARAMS_PASS, peep);
                            }
                            else//advance order
                            {
                                printf("peep advancing order\n");
                                Peep_AssignOrder(ALL_CORE_PARAMS_PASS, peep, order->nextExecutionOrder);
                            }
                            
                        }
                    }
                }
            }
            
        }
    }
   

    //advacne if theres room


    // Print_GE_INT3_Q16(peep->physics.base.pos_Q16);
    if(peep->physics.drive.targetPathNodeOPtr != OFFSET_NULL)
    {
        USE_POINTER AStarPathNode* targetPathNode;
        OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.targetPathNodeOPtr,targetPathNode);

        if(targetPathNode->completed)
        {
            //cl_uint* mapData = AStarPathNode_GetMapData(ALL_CORE_PARAMS_PASS, targetPathNode);
            
            //CL_CHECK_NULL(mapData);
            ge_int3 coord = GE_INT3_WHOLE_Q16(targetPathNode->mapCoord_Q16);
            int peepCnt = gameState->map.levels[coord.z].peepCounts[coord.x][coord.y];

            //if(peepCnt < PEEP_PATH_CROWD /*|| WHOLE_Q16(len) > 10 */)
            {
                targetVelocity.x = d.x >> 2;
                targetVelocity.y = d.y >> 2;
                targetVelocity.z = d.z >> 2;

                ge_int3 error = INT3_SUB( targetVelocity, peep->physics.base.v_Q16 );

                peep->physics.base.vel_add_Q16.x += error.x;
                peep->physics.base.vel_add_Q16.y += error.y;
                peep->physics.base.vel_add_Q16.z += error.z;
            }
        }
    }


    peep->physics.base.CS_angle_rad = atan2(((float)(d.x))/(1<<16), ((float)(d.y)) / (1 << 16));




}

void WalkAndFight(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep)
{

    //if search is done - start the peep on path
    if( peep->stateBasic.orderPtr != OFFSET_NULL)
    {
        USE_POINTER Order* order;
        OFFSET_TO_PTR(gameState->orders, peep->stateBasic.orderPtr, order);
        CL_CHECK_NULL(order);

        //keep the order alive
        order->pendingDelete = false;

        if(order->pathToDestPtr != OFFSET_NULL)
        {
            USE_POINTER AStarPathNode* path;
            OFFSET_TO_PTR(gameState->paths.pathNodes, order->pathToDestPtr, path);
            CL_CHECK_NULL(path);


                              
            if(path->processing == false && peep->stateBasic.orderInProgress == false)
            {
                peep->physics.drive.targetPathNodeOPtr = order->pathToDestPtr;
                peep->physics.drive.prevPathNodeOPtr = OFFSET_NULL;

                peep->stateBasic.orderInProgress = true;
            }
        }

    }



    if(peep->physics.drive.targetPathNodeOPtr != OFFSET_NULL)
    {
        //drive to the next path node
        USE_POINTER AStarPathNode* nxtPathNode;
        OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.targetPathNodeOPtr, nxtPathNode);
        CL_CHECK_NULL(nxtPathNode);

            ge_int3 worldloc;
            MapToWorld(nxtPathNode->mapCoord_Q16, &worldloc);
            peep->physics.drive.target_x_Q16 = worldloc.x;
            peep->physics.drive.target_y_Q16 = worldloc.y;
            peep->physics.drive.target_z_Q16 = worldloc.z;


            //restrict comms to new channel
            peep->comms.orders_channel = RandomRange(worldloc.x, 0, 10000);//broke
            peep->comms.message_TargetReached = 0;
            peep->comms.message_TargetReached_pending = 0;

    }



    PeepDrivePhysics(ALL_CORE_PARAMS_PASS, peep);
    PeepMapTileCollisions(ALL_CORE_PARAMS_PASS, peep);

}


void PeepPreUpdate1(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep)
{

}

void PeepPreUpdate2(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep)
{
    peep->physics.base.v_Q16.z += peep->physics.base.vel_add_Q16.z;
    peep->physics.base.v_Q16.y += peep->physics.base.vel_add_Q16.y;
    peep->physics.base.v_Q16.x += peep->physics.base.vel_add_Q16.x;

    peep->physics.base.vel_add_Q16.z = 0;
    peep->physics.base.vel_add_Q16.y = 0;
    peep->physics.base.vel_add_Q16.x = 0;




    peep->physics.base.pos_Q16.x += peep->physics.base.v_Q16.x;
    peep->physics.base.pos_Q16.y += peep->physics.base.v_Q16.y;
    peep->physics.base.pos_Q16.z += peep->physics.base.v_Q16.z;

    peep->physics.base.pos_Q16.z += peep->physics.base.pos_post_Q16.z;
    peep->physics.base.pos_Q16.y += peep->physics.base.pos_post_Q16.y;
    peep->physics.base.pos_Q16.x += peep->physics.base.pos_post_Q16.x;


    //hard map limits
    const int lb = 1;
    peep->physics.base.pos_Q16.x = clamp(peep->physics.base.pos_Q16.x, (-(MAP_TILE_SIZE*(((MAPDIM)/2)-lb)))<<16, (MAP_TILE_SIZE*(((MAPDIM)/2)-lb))<<16);
    peep->physics.base.pos_Q16.y = clamp(peep->physics.base.pos_Q16.y, (-(MAP_TILE_SIZE*(((MAPDIM)/2)-lb)))<<16, (MAP_TILE_SIZE*(((MAPDIM)/2)-lb))<<16);
    peep->physics.base.pos_Q16.z = clamp(peep->physics.base.pos_Q16.z, -(MAP_TILE_SIZE)<<16, (MAP_TILE_SIZE*(MAPDEPTH))<<16);


    peep->physics.base.pos_post_Q16.z = 0;
    peep->physics.base.pos_post_Q16.y = 0;
    peep->physics.base.pos_post_Q16.x = 0;

    peep->physics.base.v_Q16.x = 0;
    peep->physics.base.v_Q16.y = 0;
    peep->physics.base.v_Q16.z = 0;

    if (peep->stateBasic.health <= 0)
        peep->stateBasic.deathState = 1;


    //peep comms
    /*
    if (peep->comms.message_TargetReached_pending)
    {
        peep->comms.message_TargetReached = peep->comms.message_TargetReached_pending;
        peep->comms.message_TargetReached--;//message fade
    }
    */



}

int PeepMapVisiblity(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep, int mapZViewLevel)
{
    #ifdef PEEP_ALL_ALWAYS_VISIBLE
        return 1;
    #endif


    ge_int3 maptilecoords;
    maptilecoords.x = WHOLE_Q16(peep->posMap_Q16.x);
    maptilecoords.y = WHOLE_Q16(peep->posMap_Q16.y);
    maptilecoords.z = WHOLE_Q16(peep->posMap_Q16.z);
    
    //search up to z level 0
    ge_int3 offset, tilePWorldCen, tileMapCoordWhole;
    MapTile tile;
    cl_uint tileData;
    offset.x = 0;
    offset.y = 0;
    offset.z = 0;
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, offset, &tile, &tilePWorldCen, &tileMapCoordWhole, &tileData);
    tile = MapDataGetTile(tileData);
    cl_uchar firstTileOK = 0;
    if((MapDataLowCornerCount(tileData) > 0)  || tile == MapTile_NONE)
        firstTileOK = 1;

    tileMapCoordWhole.z++;
    tileData = gameState->map.levels[tileMapCoordWhole.z].data[tileMapCoordWhole.x][tileMapCoordWhole.y];
    tile = MapDataGetTile(tileData);

    while (firstTileOK && (tile == MapTile_NONE) && tileMapCoordWhole.z < MAPDEPTH)
    {
        tileMapCoordWhole.z++;

        tileData = gameState->map.levels[tileMapCoordWhole.z].data[tileMapCoordWhole.x][tileMapCoordWhole.y];
        tile = MapDataGetTile(tileData);
    }
    //printf("%d\n", tileMapCoordWhole.z);

    if (tileMapCoordWhole.z == MAPDEPTH)
    {
        //hit the sky 
        if (maptilecoords.z <= mapZViewLevel+1)
            return 1;
        else
            return 0;
        return 1;
    }
    else
    {
        //'pocket' case
        if (tileMapCoordWhole.z >= mapZViewLevel+2)
        {
            if (maptilecoords.z <= mapZViewLevel+1)
            {
                return 1;
            }
        }

        return 0;
    }
    
}


void MachineUpdate(ALL_CORE_PARAMS,PARAM_GLOBAL_POINTER Machine* machine)
{
    USE_POINTER MachineDesc* desc;
    OFFSET_TO_PTR(gameState->machineDescriptions, machine->MachineDescPtr, desc);

    USE_POINTER MachineRecipe* recip;
    OFFSET_TO_PTR(gameState->machineRecipes, machine->recipePtr, recip);

    bool readyToProcess = true;
    for(int i = 0; i < 8; i++)
    {
        int ratio = recip->inputRatio[i];
        ItemTypes type = recip->inputTypes[i];
        if(ratio >= 1)
        {
            if(machine->inventoryIn.counts[type] >= ratio)
            {
                
            }
            else
            {
                readyToProcess = false;
            }
        }
    }

    if(readyToProcess && machine->state == MachineState_Running)
    {
        machine->tickProgess++;

        if(machine->tickProgess >= desc->processingTime)
        {
            machine->tickProgess = 0;
            MachineTypes type = desc->type;
            
          

            for(int i = 0; i < 8; i++)
            {
                int ratio = recip->inputRatio[i];
                int outRatio = recip->outputRatio[i];
                ItemTypes type = recip->inputTypes[i];
                ItemTypes outType = recip->outputTypes[i];

                machine->inventoryIn.counts[type]-=ratio;
                machine->inventoryOut.counts[outType]+=outRatio;
            }
            
        }
    }



    //update command center order sequences
   if(desc->type == MachineTypes_COMMAND_CENTER)
   {
        if(machine->orderLen > 1)
        {


            USE_POINTER Order* order;
            OFFSET_TO_PTR(gameState->orders, machine->rootOrderPtr, order);

            offsetPtr curOrderPtr = machine->rootOrderPtr;


            while(curOrderPtr != OFFSET_NULL)
            {
                USE_POINTER Order* curOrder;
                OFFSET_TO_PTR(gameState->orders, curOrderPtr, curOrder);

                if(curOrder->destinationSet)
                {
                    USE_POINTER Order* nextOrder;
                    OFFSET_TO_PTR(gameState->orders, curOrder->nextExecutionOrder, nextOrder);

                    if(nextOrder != NULL)
                    {
                        if(nextOrder->destinationSet)
                        {
                            if(nextOrder->dirtyPathing)
                            {
                                if(nextOrder->pathToDestPtr != OFFSET_NULL)
                                {
                                    AStarPathSteps_DeletePath(ALL_CORE_PARAMS_PASS, nextOrder->pathToDestPtr);
                                    nextOrder->pathToDestPtr = OFFSET_NULL;
                                }
                                nextOrder->dirtyPathing = false;
                            }
                            if(nextOrder->pathToDestPtr == OFFSET_NULL)
                            {
                                nextOrder->pathToDestPtr = AStarPathStepsNextFreePathNode(&gameState->paths);
                            }

                            USE_POINTER AStarPathNode* pathNode;
                            OFFSET_TO_PTR(gameState->paths.pathNodes, nextOrder->pathToDestPtr, pathNode);
                            
                            if((!pathNode->completed && !pathNode->processing))
                            {



                                printf("path: %d\n", nextOrder->pathToDestPtr);

                                pathNode->processing = true;

                                //enqueue job for path from previous order to this order
                                nextOrder->aStarJobPtr = AStarJob_EnqueueJob(ALL_CORE_PARAMS_PASS);

                                USE_POINTER AStarJob* job;
                                OFFSET_TO_PTR(gameState->mapSearchJobQueue, nextOrder->aStarJobPtr, job);


                                job->startLoc = curOrder->mapDest_Coord;
                                job->endLoc = nextOrder->mapDest_Coord;
                                job->pathPtr = nextOrder->pathToDestPtr;

                                nextOrder->pathToDestPtr = job->pathPtr;
                            }

                        }        
                    }
                }


                curOrderPtr = curOrder->nextExecutionOrder;


                if(curOrderPtr == machine->rootOrderPtr)
                   break;
            }


        }



        


   }




}


void PeepUpdate(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep)
{

    peep->minDistPeep_Q16 = (1 << 30);
    peep->minDistPeepPtr = OFFSET_NULL;


    cl_int x = ((peep->physics.base.pos_Q16.x >> 16) / (SECTOR_SIZE));
    cl_int y = ((peep->physics.base.pos_Q16.y >> 16) / (SECTOR_SIZE));


    USE_POINTER MapSector* cursector = &(gameState->sectors[x + SQRT_MAXSECTORS / 2][y + SQRT_MAXSECTORS / 2]);
    CL_CHECK_NULL(cursector)


    //traverse sector
    int minx = cursector->ptr.x - 1; if (minx == 0xFFFFFFFF) minx = 0;
    int miny = cursector->ptr.y - 1; if (miny == 0xFFFFFFFF) miny = 0;

    int maxx = cursector->ptr.x + 1; if (maxx >= SQRT_MAXSECTORS) maxx = SQRT_MAXSECTORS-1;
    int maxy = cursector->ptr.y + 1; if (maxy >= SQRT_MAXSECTORS) maxy = SQRT_MAXSECTORS-1;
    
    for(cl_int sectorx = minx; sectorx <= maxx; sectorx++)
    {
        for (cl_int sectory = miny; sectory <= maxy; sectory++)
        {

            USE_POINTER MapSector* sector = &gameState->sectors[sectorx][sectory];
            CL_CHECK_NULL(sector);


            for(int i = 0; i < MAX_PEEPS_PER_SECTOR; i++)
            {
                if(sector->peepPtrs[i] == OFFSET_NULL)
                    continue;

                USE_POINTER Peep* otherPeep;
                OFFSET_TO_PTR(gameState->peeps, sector->peepPtrs[i], otherPeep);
                
                if (otherPeep != peep) {
                    
                    PeepToPeepInteraction(ALL_CORE_PARAMS_PASS, peep, otherPeep);
                }
            }
        }
    }
   
    ge_int3 posMap_Q16;
    WorldToMap( peep->physics.base.pos_Q16, &posMap_Q16);
    peep->posMap_Q16 = posMap_Q16;

    ge_int3 maptilecoords;
    maptilecoords.x = WHOLE_Q16(peep->posMap_Q16.x);
    maptilecoords.y = WHOLE_Q16(peep->posMap_Q16.y);
    maptilecoords.z = WHOLE_Q16(peep->posMap_Q16.z);

    ge_int3 maptilecoords_prev;
    maptilecoords_prev.x = WHOLE_Q16(peep->lastGoodPosMap_Q16.x);
    maptilecoords_prev.y = WHOLE_Q16(peep->lastGoodPosMap_Q16.y);
    maptilecoords_prev.z = WHOLE_Q16(peep->lastGoodPosMap_Q16.z);


    //update visibility
    if (!VECTOR3_EQUAL(maptilecoords, maptilecoords_prev) || ThisClient(ALL_CORE_PARAMS_PASS)->updateMap)
    {
        if (PeepMapVisiblity(ALL_CORE_PARAMS_PASS, peep, ThisClient(ALL_CORE_PARAMS_PASS)->mapZView))
        {     
            BITSET(peep->stateBasic.bitflags0, PeepState_BitFlags_visible);
        }
        else
        {
            BITCLEAR(peep->stateBasic.bitflags0, PeepState_BitFlags_visible);
        }
    }



    //update map coord tracking
    if(VECTOR3_EQUAL(peep->mapCoord, maptilecoords) == 0)
    {



        peep->mapCoord_1 = peep->mapCoord;
        peep->mapCoord = VECTOR3_CAST(maptilecoords, offsetPtrShort3);

        //printf("a");
            // cl_uint* mapData = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, VECTOR3_CAST(peep->mapCoord, ge_int3));
            // peepCnt = MapTileData_PeepCount(*mapData);
            // MapTileData_SetPeepCount(mapData, peepCnt+1);
        
        //int b = atomic_inc(&gameState->map.levels[peep->mapCoord.z].peepCounts[peep->mapCoord.x][peep->mapCoord.y]);
        //int a = atomic_dec(&gameState->map.levels[peep->mapCoord_1.z].peepCounts[peep->mapCoord_1.x][peep->mapCoord_1.y]);

        //printf("%d,%d\n", a,b);
    }
        

    
    

    


    //revert position to last good if needed
    MapTile curTile;
    cl_uint tileData;
    ge_int3 dummy;
    ge_int3 dummy2;
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 0, 0 }, &curTile, &dummy, & dummy2, & tileData);

    

    if (peep->stateBasic.buriedGlitchState != 0)
    {
        //revert to center of last good map position
        #ifndef PEEP_DISABLE_TILECORRECTIONS

            ge_int3 lastGoodPos;
            MapToWorld(peep->lastGoodPosMap_Q16, &lastGoodPos);

            peep->physics.base.pos_post_Q16.x += lastGoodPos.x - peep->physics.base.pos_Q16.x;
            peep->physics.base.pos_post_Q16.y += lastGoodPos.y - peep->physics.base.pos_Q16.y;
            peep->physics.base.pos_post_Q16.z += lastGoodPos.z - peep->physics.base.pos_Q16.z;

            peep->physics.base.vel_add_Q16.x = -peep->physics.base.v_Q16.x;
            peep->physics.base.vel_add_Q16.y = -peep->physics.base.v_Q16.y;
            peep->physics.base.vel_add_Q16.z = -peep->physics.base.v_Q16.z;

        #endif
    }
    else
    {

        peep->lastGoodPosMap_Q16 = peep->posMap_Q16;
    }
    
    WalkAndFight(ALL_CORE_PARAMS_PASS, peep);

}

void ParticleUpdate(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Particle* p)
{
    p->pos.x = ADD_QMP32(p->pos.x, p->vel.x);
    p->pos.y = ADD_QMP32(p->pos.y, p->vel.y);
}














void PrintSelectionPeepStats(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* p)
{
///    Print_GE_INT3_Q16(p->physics.base.pos_Q16);
    USE_POINTER Peep* peep = p;
    MapTile data[22];
    ge_int3 tileCenters_Q16[22];
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 0, 0 }, & data[0], & tileCenters_Q16[0]); printf("{ 1, 0, 0 }: %d\n", data[0]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 0, 0 }, & data[1], & tileCenters_Q16[1]); printf("{ -1, 0, 0 }: %d\n", data[1]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, -1, 0 }, & data[2], & tileCenters_Q16[2]); printf("{ 0, -1, 0 }: %d\n", data[2]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 1, 0 }, & data[3], & tileCenters_Q16[3]); printf("{ 0, 1, 0 }: %d\n", data[3]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 0, 1 }, & data[4], & tileCenters_Q16[4]); printf("{ 0, 0, 1 }: %d\n", data[4]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 0, -1 }, & data[5], & tileCenters_Q16[5]); printf("{ 0, 0, -1 }: %d\n", data[5]);

    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 0, -1 }, & data[6], & tileCenters_Q16[6]); printf("{ 1, 0, -1 }: %d\n", data[6]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 1, -1 }, & data[7], & tileCenters_Q16[7]); printf("{ 0, 1, -1 }: %d\n", data[7]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 1, -1 }, & data[8], & tileCenters_Q16[8]); printf("{ 1, 1, -1 }: %d\n", data[8]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 0, -1 }, & data[9], & tileCenters_Q16[9]); printf("{ -1, 0, -1 }: %d\n", data[9]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, -1, -1 }, & data[10], & tileCenters_Q16[10]); printf("{ 0, -1, -1 }: %d\n", data[10]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, -1, -1 }, & data[11], & tileCenters_Q16[11]); printf("{ -1, -1, -1 }: %d\n", data[11]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, -1, -1 }, & data[12], & tileCenters_Q16[12]); printf("{ 1, -1, -1 }: %d\n", data[12]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 1, -1 }, & data[13], & tileCenters_Q16[13]); printf("{ -1, 1, -1 }: %d\n", data[13]);

    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 0, 1 }, & data[14], & tileCenters_Q16[14]); printf("{ 1, 0, 1 }: %d\n", data[14]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 1, 1 }, & data[15], & tileCenters_Q16[15]); printf("{ 0, 1, 1 }: %d\n", data[15]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 1, 1 }, & data[16], & tileCenters_Q16[16]); printf("{ 1, 1, 1 }: %d\n", data[16]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 0, 1 }, & data[17], & tileCenters_Q16[17]); printf("{ -1, 0, 1 }: %d\n", data[17]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, -1, 1 }, & data[18], & tileCenters_Q16[18]); printf("{ 0, -1, 1 }: %d\n", data[18]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, -1, 1 }, & data[19], & tileCenters_Q16[19]); printf("{ -1, -1, 1 }: %d\n", data[19]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, -1, 1 }, & data[20], & tileCenters_Q16[20]); printf("{ 1, -1, 1 }: %d\n", data[20]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 1, 1 }, & data[21], & tileCenters_Q16[21]); printf("{ -1, 1, 1 }: %d\n", data[21]);
}

void MapTileCoordClamp( ge_int3* mapCoord, int xybuffer)
{
    (*mapCoord).x = clamp((*mapCoord).x, xybuffer, MAPDIM - 1 - xybuffer);
    (*mapCoord).y = clamp((*mapCoord).y, xybuffer, MAPDIM - 1 - xybuffer);
    (*mapCoord).z = clamp((*mapCoord).z, 0, MAPDEPTH - 1);
}


void GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS, ge_int2 world_Q16, 
 ge_int3* mapcoord_whole, 
 int* occluded, int zViewRestrictLevel)
{
    ge_int3 wrld_Q16;
    wrld_Q16.x = world_Q16.x;
    wrld_Q16.y = world_Q16.y;
    WorldToMap(wrld_Q16, &(*mapcoord_whole));
    (*mapcoord_whole).x = WHOLE_Q16((*mapcoord_whole).x);
    (*mapcoord_whole).y = WHOLE_Q16((*mapcoord_whole).y);


    MapTileCoordClamp(mapcoord_whole,1);

    for (int z = zViewRestrictLevel; z >= 0; z--)
    {
        USE_POINTER cl_uint* data = &gameState->map.levels[z].data[(*mapcoord_whole).x][(*mapcoord_whole).y];
        cl_uint dataCopy = *data;
        MapTile tile = MapDataGetTile(dataCopy);

        if (tile != MapTile_NONE)
        {
            (*mapcoord_whole).z = z;
            if (z == ThisClient(ALL_CORE_PARAMS_PASS)->mapZView-1)
            {
                *occluded = 1;
            }
            else if (z == ThisClient(ALL_CORE_PARAMS_PASS)->mapZView)
            {
                *occluded = 1;
            }
            else
            {
                *occluded = 0;
            }
            return;
        }
    }

   // printf("3");
    *occluded = 1;
}







void LINES_DrawLine(ALL_CORE_PARAMS, float2 screenPosStart, float2 screenPosEnd, float3 color)
{
    if(gameState->debugLinesIdx+10 >= MAX_LINES)
    {
        printf("out of debug line space!\n");
        return;
    }

    linesVBO[gameState->debugLinesIdx*5 + 0] = screenPosStart.x;
    linesVBO[gameState->debugLinesIdx*5 + 1] = screenPosStart.y;

    linesVBO[gameState->debugLinesIdx*5 + 2] = color.x;
    linesVBO[gameState->debugLinesIdx*5 + 3] = color.y;
    linesVBO[gameState->debugLinesIdx*5 + 4] = color.z;

    
    linesVBO[gameState->debugLinesIdx*5 + 5] = screenPosEnd.x;
    linesVBO[gameState->debugLinesIdx*5 + 6] = screenPosEnd.y;

    linesVBO[gameState->debugLinesIdx*5 + 7] = color.x;
    linesVBO[gameState->debugLinesIdx*5 + 8] = color.y;
    linesVBO[gameState->debugLinesIdx*5 + 9] = color.z;

    gameState->debugLinesIdx+=10;

}
void LINES_ClearAll(ALL_CORE_PARAMS)
{
    for(int i = gameState->debugLinesIdx*10; i >=0; i--)
        linesVBO[i] = 0.0f;

    gameState->debugLinesIdx=0;;
}

float4 Matrix_Float4_Times_Vec4(global float mat[][4], float4 vec)
{
    float4 res;
    res.x = mat[0][0] * vec.x + mat[0][1] * vec.y + mat[0][2] * vec.z + mat[0][3] * vec.w;
    res.y = mat[1][0] * vec.x + mat[1][1] * vec.y + mat[1][2] * vec.z + mat[1][3] * vec.w;
    res.z = mat[2][0] * vec.x + mat[2][1] * vec.y + mat[2][2] * vec.z + mat[2][3] * vec.w;
    res.w = mat[3][0] * vec.x + mat[3][1] * vec.y + mat[3][2] * vec.z + mat[3][3] * vec.w;
    return res;
}

void LINES_DrawLineWorld(ALL_CORE_PARAMS, float2 worldPosStart, float2 worldPosEnd, float3 color)
{
    float4 worldPosStart4 = (float4)(worldPosStart.x, worldPosStart.y, 0.0f, 1.0f);
    float4 screenPosStart4 = Matrix_Float4_Times_Vec4(&gameStateActions->viewMatrix[0], worldPosStart4);
    float2 screenPosStart2 = (float2)(screenPosStart4.x, screenPosStart4.y);

    float4 worldPosEnd4 = (float4)(worldPosEnd.x, worldPosEnd.y, 0.0f, 1.0f);
    float4 screenPosEnd4 = Matrix_Float4_Times_Vec4(&gameStateActions->viewMatrix[0], worldPosEnd4);
    float2 screenPosEnd2 = (float2)(screenPosEnd4.x, screenPosEnd4.y);

    // printf("[%f,%f,%f,%f]\n", gameStateActions->viewMatrix[0][0], gameStateActions->viewMatrix[0][1], gameStateActions->viewMatrix[0][2], gameStateActions->viewMatrix[0][3]);
    // printf("[%f,%f,%f,%f]\n", gameStateActions->viewMatrix[1][0], gameStateActions->viewMatrix[1][1], gameStateActions->viewMatrix[1][2], gameStateActions->viewMatrix[1][3]);
    // printf("[%f,%f,%f,%f]\n", gameStateActions->viewMatrix[2][0], gameStateActions->viewMatrix[2][1], gameStateActions->viewMatrix[2][2], gameStateActions->viewMatrix[2][3]);
    // printf("[%f,%f,%f,%f]\n", gameStateActions->viewMatrix[3][0], gameStateActions->viewMatrix[3][1], gameStateActions->viewMatrix[3][2], gameStateActions->viewMatrix[3][3]);
    // printf("---------------------\n");
   // gameStateActions->
    LINES_DrawLine(ALL_CORE_PARAMS_PASS,  screenPosStart2, screenPosEnd2,  color);
}

ge_int2 GUI_TO_WORLD_Q16(ALL_CORE_PARAMS, ge_int2 guiCoord)
{
    float4 v;
    v.x = guiCoord.x;
    v.y = guiCoord.y;
    v.z = 0.0;
    v.w = 1.0f;
    float4 worldPos = Matrix_Float4_Times_Vec4(&gameStateActions->viewMatrix_Inv[0], v);

    printf("%f, %f, %f, %f\n", worldPos.x, worldPos.y, worldPos.z, worldPos.w);

    return (ge_int2){0,0};
}

float2 TileToUV(TileUnion tile)
{

    //duplicate of geomMapTile.glsl code.
    float2 uv;
    uv.x = ((uint)tile.mapTile & 15u) / 16.0;
    uv.y = (((uint)tile.mapTile >> 4u) & 15u) / 16.0;    
    return uv;
}


void PrintMouseState(int mouseState)
{
    if(BITGET(mouseState, MouseButtonBits_PrimaryDown))
        printf("MouseButtonBits_PrimaryDown |");
    if(BITGET(mouseState, MouseButtonBits_PrimaryPressed))
        printf("MouseButtonBits_PrimaryPressed |");
    if(BITGET(mouseState, MouseButtonBits_PrimaryReleased))
        printf("MouseButtonBits_PrimaryReleased |");
    if(BITGET(mouseState, MouseButtonBits_SecondaryDown))
        printf("MouseButtonBits_SecondaryDown |");
    if(BITGET(mouseState, MouseButtonBits_SecondaryPressed))
        printf("MouseButtonBits_SecondaryPressed |");
    if(BITGET(mouseState, MouseButtonBits_SecondaryReleased))
        printf("MouseButtonBits_SecondaryReleased |");

    printf("\n");
}


void InventoryGui(GUIID_DEF_ALL, PARAM_GLOBAL_POINTER Inventory* inventory)
{
    GUI_COMMON_WIDGET_START()

    if(!goodStart)
    {
        return;
    }

    int j = 0;
    for(int i = 0; i < ItemTypes_NUMITEMS; i++)
    {
        int count = inventory->counts[i];
        if(count > 0)
        {
            const int height = 40;
            float2 uv = TileToUV(gameState->ItemTypeTiles[i]);
            GUI_IMAGE(GUIID_PASS, origPos + (ge_int2)(0,(j+1)*height + 100), (ge_int2)(height,height), 0,  uv, uv + MAP_TILE_UV_WIDTH_FLOAT2, gameState->ItemColors[i]);


            LOCAL_STR(cntstr, "-----------");
            CL_ITOA(count, cntstr, cntstr_len, 10 );                 
            GUI_TEXT(GUIID_PASS, origPos + (ge_int2)(height,(j+1)*height + 100), (ge_int2)(50,height), 0, cntstr);

            GUI_TEXT_CONST(GUIID_PASS, origPos + (ge_int2)(height+50,(j+1)*height + 100), (ge_int2)(50,height), 0, &ItemTypeStrings[i][0]);
            j++;

        }
    }
}


offsetPtr Order_GetNewOrder(ALL_CORE_PARAMS)
{
    offsetPtr ptr = gameState->nextOrderIdx;
    gameState->nextOrderIdx++;
    bool loopCheck = false;
    while( gameState->orders[gameState->nextOrderIdx].valid)
    {
        gameState->nextOrderIdx++;
        if(gameState->nextOrderIdx >= MAX_ORDERS)
        {
            gameState->nextOrderIdx = 0;
            if(loopCheck)
            {
                printf("Out of Order Space! (MAX_ORDERS)\n");
            }
            loopCheck = true;
        }
    }

    USE_POINTER Order* order;
    OFFSET_TO_PTR(gameState->orders, ptr, order);

    order->valid = true;
    order->ptr = ptr;
    order->pendingDelete = false;
    order->refCount = 0;
    order->prevExecutionOrder = OFFSET_NULL;
    order->nextExecutionOrder = OFFSET_NULL;
    order->pathToDestPtr = OFFSET_NULL;
    order->selectingOrderLocation = false;
    order->destinationSet = false;
    order->aStarJobPtr = OFFSET_NULL;

    return ptr;
}


void PeepCommandGui(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER SyncedGui* gui, PARAM_GLOBAL_POINTER SynchronizedClientState* client)
{
    if(client->selectedPeepPrimary != OFFSET_NULL)
    {
        USE_POINTER Peep* peep;
        OFFSET_TO_PTR(gameState->peeps, client->selectedPeepPrimary, peep);

        LOCAL_STRL(mw, "Miner 123456", mwlen); 
        CL_ITOA(peep->ptr, (mw)+6,mwlen-6, 10);
        if(GUI_BEGIN_WINDOW(GUIID_PASS, &gui->guiState.windowPositions[2],
            &gui->guiState.windowSizes[2] ,0,  mw ))
        {

            if(peep->physics.drive.targetPathNodeOPtr != OFFSET_NULL)
            {
                LOCAL_STR(thinkingtxt, "Traveling..."); 
                GUI_LABEL(GUIID_PASS, (ge_int2)(0,0), (ge_int2)(gui->guiState.windowSizes[2].x,20), 0, thinkingtxt, (float3)(0.0,0,0) );
            }

            if(peep->stateBasic.orderPtr != OFFSET_NULL)
            {
                LOCAL_STR(descstr, "ORDER ------")
                CL_ITOA(peep->stateBasic.orderPtr, descstr+6, 6, 10 );
                GUI_LABEL(GUIID_PASS, (ge_int2)(0,20), (ge_int2)(100,50), 0, descstr, (float3)(0,0,0));
            }

            

            
            ge_int3 mapCoord = SHORT3_TO_INT3(peep->mapCoord);
            mapCoord.z--;

            MapTile downTile = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, mapCoord);
            USE_POINTER Machine* machine = MachineGetFromMapCoord(ALL_CORE_PARAMS_PASS, mapCoord);


            if(downTile != MapTile_NONE)
            {
                LOCAL_STR(dig, "Drill Down"); 
                if(GUI_BUTTON(GUIID_PASS, (ge_int2)(0,50), (ge_int2)(100,50), GuiFlags_Beveled, (float3)(0.4,0.4,0.4), dig, NULL, NULL ))
                {
                    if(gui->passType == GuiStatePassType_Synced)
                    {
                        PeepDrill(ALL_CORE_PARAMS_PASS, peep);
                    }
                }

                if(machine != NULL)
                {
                    
                    USE_POINTER MachineDesc* machDesc;
                    OFFSET_TO_PTR(gameState->machineDescriptions, machine->MachineDescPtr, machDesc);

                    if(machDesc->type == MachineTypes_COMMAND_CENTER)
                    {
                        LOCAL_STR(str, "Link"); 
                        if(GUI_BUTTON(GUIID_PASS, (ge_int2)(0,50+50), (ge_int2)(gui->guiState.windowSizes[2].x,50),GuiFlags_Beveled,
                        (float3)(0.4,0.4,0.4), str, NULL, NULL ))
                        {    
                            if(gui->passType == GuiStatePassType_Synced)
                            {

                                USE_POINTER Order* machineRootOrder;
                                OFFSET_TO_PTR(gameState->orders,machine->rootOrderPtr,  machineRootOrder);

                                //make path to the first machine order

                                USE_POINTER Order* newOrder;
                                offsetPtr newOrderPtr = Order_GetNewOrder(ALL_CORE_PARAMS_PASS);
                                OFFSET_TO_PTR(gameState->orders, newOrderPtr, newOrder);
                                newOrder->valid = true;
                                newOrder->ptr = newOrderPtr;
                                newOrder->pendingDelete = false;
                                newOrder->mapDest_Coord = machineRootOrder->mapDest_Coord;
                                newOrder->prevExecutionOrder = OFFSET_NULL;
                                newOrder->nextExecutionOrder = machine->rootOrderPtr;
                                newOrder->pathToDestPtr = AStarPathStepsNextFreePathNode(&gameState->paths);
                                newOrder->aStarJobPtr = AStarJob_EnqueueJob(ALL_CORE_PARAMS_PASS);

                                newOrder->refCount++;


                                USE_POINTER AStarJob* job;
                                OFFSET_TO_PTR(gameState->mapSearchJobQueue, newOrder->aStarJobPtr, job);
                                job->startLoc = GE_INT3_WHOLE_Q16(peep->posMap_Q16);
                                job->endLoc = machineRootOrder->mapDest_Coord;
                                job->pathPtr = newOrder->pathToDestPtr;


                                Peep_AssignOrder(ALL_CORE_PARAMS_PASS, peep, newOrderPtr);
                            }
                        }
                    }
                    else
                    {

                
                        LOCAL_STR(transferStr, "Push Down"); 
                        if(GUI_BUTTON(GUIID_PASS, (ge_int2)(0,50+50), (ge_int2)(gui->guiState.windowSizes[2].x,50),GuiFlags_Beveled,
                        (float3)(0.4,0.4,0.4), transferStr, NULL, NULL ))
                        {    
                            if(gui->passType == GuiStatePassType_Synced)
                            {
                                PeepPush(ALL_CORE_PARAMS_PASS, peep);
                            }
                        }
                        LOCAL_STR(transferStr2, "Pull Up"); 
                        if(GUI_BUTTON(GUIID_PASS, (ge_int2)(0,50+50+50), (ge_int2)(gui->guiState.windowSizes[2].x,50),
                        GuiFlags_Beveled, (float3)(0.4,0.4,0.4), transferStr2, NULL, NULL ))
                        {
                            if(gui->passType == GuiStatePassType_Synced)
                            {
                                PeepPull(ALL_CORE_PARAMS_PASS, peep);
                            }
                        }

                    }

                }

            }
            InventoryGui(GUIID_PASS, (ge_int2)(0,50),gui->guiState.windowSizes[2], 0, &peep->inventory);

            GUI_END_WINDOW(GUIID_PASS);
        }
    }
}

float ANIMATION_BLINK(ALL_CORE_PARAMS)
{
    return (sin(gameStateActions->tickIdx*0.2f)+1)*0.5;
}


void CLIENT_PushTool( PARAM_GLOBAL_POINTER SynchronizedClientState* client, EditorTools tool){
    client->curTool_1 = client->curTool;
    client->curTool = tool;
}

void CLIENT_PopTool( PARAM_GLOBAL_POINTER SynchronizedClientState* client)
{
    client->curTool = client->curTool_1;
}

bool OrderEntryGui(GUIID_DEF_ALL, PARAM_GLOBAL_POINTER Order* order,
 PARAM_GLOBAL_POINTER SynchronizedClientState* client)
{
    GUI_COMMON_WIDGET_START();
    if(!goodStart)
        return false;

    if(order->valid)
    {
        GUI_DrawRectangle(ALL_CORE_PARAMS_PASS,
         gui, pos.x, pos.y, origSize.x-50, origSize.y, COLOR_DARKGRAY,
          gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE, false);

        LOCAL_STR(descstr, "ORDER -----------")
        CL_ITOA(order->ptr, descstr+6, 6, 10 );
        GUI_LABEL(GUIID_PASS, origPos,(ge_int2)(origSize.x-50, 20), 0, descstr, COLOR_GRAY);
        
        // LOCAL_STR(nxtstr, "NEXT  -----------")
        // CL_ITOA(order->nextExecutionOrder, nxtstr+6, 6, 10 );
        // GUI_LABEL(GUIID_PASS, origPos + (ge_int2)(0,20), origSize - (ge_int2)(50,1), 0, nxtstr, (float3)(0,0,0));
        
        // LOCAL_STR(prevstr, "PREV  -----------")
        // CL_ITOA(order->prevExecutionOrder, prevstr+6, 6, 10 );
        // GUI_LABEL(GUIID_PASS, origPos + (ge_int2)(0,40), origSize - (ge_int2)(50,1), 0, prevstr, (float3)(0,0,0));
        

        LOCAL_STR(locStr, "SET TARGET")
        LOCAL_STR(locStr2, "TARGETING..")
        LOCAL_STR(locStr3, "ASSIGNED")





        char* str;
        float3 color = COLOR_ORANGE;
        if(client->curEditingOrderPtr == order->ptr && client->curEditingOrder_targeting)
        {
            str = locStr2;  
            color = COLOR_RED * ANIMATION_BLINK(ALL_CORE_PARAMS_PASS);

            if(client->tileTargetFound )
            {
                order->mapDest_Coord = client->tileTargetMapCoord;
                order->destinationSet = true;

                order->dirtyPathing = true;

                if(order->nextExecutionOrder != OFFSET_NULL)
                {
                    USE_POINTER Order* nextOrder;
                    OFFSET_TO_PTR(gameState->orders, order->nextExecutionOrder, nextOrder);
                    nextOrder->dirtyPathing = true;
                }
                



                client->curEditingOrderPtr = OFFSET_NULL;
                client->curEditingOrder_targeting = false;
                client->tileTargetFound = false;
            }
        }
        else if(order->destinationSet)
        {
            str = locStr3;  
            color = COLOR_GREEN;
        }
        else
        {
            str = locStr;
        }
        

        USE_POINTER bool* toggle = &gui->fakeDummyBool;
        gui->fakeDummyBool = order->selectingOrderLocation;
        if(gui->passType == GuiStatePassType_Synced)
            toggle = &order->selectingOrderLocation;

        if(GUI_BUTTON(GUIID_PASS, origPos + (ge_int2)(0,20), (ge_int2)(100,50), GuiFlags_Beveled, color, str, NULL, NULL))
        {
            if(gui->passType == GuiStatePassType_Synced && gui->hoverWidget == gui->nextId-2)
            {
                client->curEditingOrderPtr = order->ptr;
                
                if(!client->curEditingOrder_targeting)
                {

                    printf("Entering Tile Select Mode");

                    CLIENT_PushTool(client, EditorTools_TileTargetSelect);
                    client->curEditingOrder_targeting = true;
                    client->tileTargetFound = false;
                }
                else
                {
                    CLIENT_PopTool(client);
                    client->curEditingOrder_targeting = false;
                }


            }
        }
        

        LOCAL_STR(drillStr, "DRILL")
        LOCAL_STR(pushStr, "PUSH")
        LOCAL_STR(pullStr, "PULL")


        if(order->action == OrderAction_MINE)
            gui->fakeDummyBool = true;
        else
            gui->fakeDummyBool = false;

        toggle = &gui->fakeDummyBool;
        if(GUI_BUTTON(GUIID_PASS, origPos + (ge_int2)(100,20), (ge_int2)(50,50), GuiFlags_Beveled, COLOR_GRAY, drillStr, NULL, toggle))
        {
            if(gui->passType == GuiStatePassType_Synced)
                order->action = OrderAction_MINE;
        }

        if(order->action == OrderAction_DROPOFF_MACHINE)
            gui->fakeDummyBool = true;
        else
            gui->fakeDummyBool = false;

        toggle = &gui->fakeDummyBool;
        if(GUI_BUTTON(GUIID_PASS, origPos + (ge_int2)(150,20), (ge_int2)(50,50), GuiFlags_Beveled, COLOR_GRAY, pushStr, NULL, toggle))
        {
            if(gui->passType == GuiStatePassType_Synced)
                order->action = OrderAction_DROPOFF_MACHINE;
        }

        
        if(order->action == OrderAction_PICKUP_MACHINE)
            gui->fakeDummyBool = true;
        else
            gui->fakeDummyBool = false;

        toggle = &gui->fakeDummyBool;
        if(GUI_BUTTON(GUIID_PASS, origPos + (ge_int2)(200,20), (ge_int2)(50,50), GuiFlags_Beveled, COLOR_GRAY, pullStr, NULL, toggle))
        {
            if(gui->passType == GuiStatePassType_Synced)
                order->action = OrderAction_PICKUP_MACHINE;
        }


        LOCAL_STR(delStr, "DELETE")
        if(GUI_BUTTON(GUIID_PASS, origPos + (ge_int2)(origSize.x-75,origSize.y/3), (ge_int2)(75,origSize.y/3), GuiFlags_Beveled, COLOR_RED, delStr, NULL, NULL))
        {
            if(gui->passType == GuiStatePassType_Synced)
            {
                printf("deleting order by button\n");
                Order_DeleteOrder(ALL_CORE_PARAMS_PASS, order->ptr);
            }
        }
        LOCAL_STR(upStr, "UP")
        if(GUI_BUTTON(GUIID_PASS, origPos + (ge_int2)(origSize.x-75,0), (ge_int2)(75,origSize.y/3), GuiFlags_Beveled, COLOR_ORANGE, upStr, NULL, NULL))
        {
            if(gui->passType == GuiStatePassType_Synced)
            {
                USE_POINTER Order* prevOrder;
                OFFSET_TO_PTR(gameState->orders, order->prevExecutionOrder, prevOrder);

                USE_POINTER Order* nextOrder;
                OFFSET_TO_PTR(gameState->orders, order->nextExecutionOrder, nextOrder);
                
                if(nextOrder == prevOrder && nextOrder != NULL)
                {
                    return false;//list of 2
                }
                order->dirtyPathing = true;

                if(prevOrder != NULL)
                {
                    order->prevExecutionOrder = prevOrder->prevExecutionOrder;
                    prevOrder->nextExecutionOrder = order->nextExecutionOrder;
                    order->nextExecutionOrder = prevOrder->ptr;
                    

                    if(prevOrder->prevExecutionOrder != OFFSET_NULL && prevOrder->prevExecutionOrder != order->ptr)
                    {
                        USE_POINTER Order* prevPrevOrder;
                        OFFSET_TO_PTR(gameState->orders, prevOrder->prevExecutionOrder, prevPrevOrder);

                        prevPrevOrder->nextExecutionOrder = order->ptr;
                        prevPrevOrder->dirtyPathing = true;
                    }
                    prevOrder->prevExecutionOrder = order->ptr;
                    prevOrder->dirtyPathing = true;

                    

                    if(nextOrder != NULL)
                    {
                        nextOrder->prevExecutionOrder = prevOrder->ptr;
                        nextOrder->dirtyPathing = true;
                    }
                }
            }
            printf("order moved Up.\n");
        }
        LOCAL_STR(downStr, "DOWN")
        if(GUI_BUTTON(GUIID_PASS, origPos + (ge_int2)(origSize.x-75,2*origSize.y/3), (ge_int2)(75,origSize.y/3), GuiFlags_Beveled, COLOR_ORANGE, downStr, NULL, NULL))
        {
            if(gui->passType == GuiStatePassType_Synced)
            {
                USE_POINTER Order* prevOrder;
                OFFSET_TO_PTR(gameState->orders, order->prevExecutionOrder, prevOrder);

                USE_POINTER Order* nextOrder;
                OFFSET_TO_PTR(gameState->orders, order->nextExecutionOrder, nextOrder);

                if(nextOrder == prevOrder && nextOrder != NULL)
                {
                    return false;//list of 2
                }
                order->dirtyPathing = true;
                if(nextOrder != NULL)
                {

                    nextOrder->prevExecutionOrder = order->prevExecutionOrder;
                    order->prevExecutionOrder = nextOrder->ptr;
                    order->nextExecutionOrder = nextOrder->nextExecutionOrder;
                    

                    if(nextOrder->nextExecutionOrder != OFFSET_NULL && nextOrder->nextExecutionOrder != order->ptr)
                    {

                        USE_POINTER Order* nextNextOrder;
                        OFFSET_TO_PTR(gameState->orders, nextOrder->nextExecutionOrder, nextNextOrder);

                        nextNextOrder->prevExecutionOrder = order->ptr;
                        nextNextOrder->dirtyPathing = true;
                    }
                    nextOrder->nextExecutionOrder = order->ptr;
                    nextOrder->dirtyPathing = true;

                    if(prevOrder != NULL)
                    {
                        prevOrder->nextExecutionOrder = nextOrder->ptr;
                        prevOrder->dirtyPathing = true;
                    }
                }
            }
        }
        return true;

    }
    return false;
}

void OrderListGui(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER SyncedGui* gui, PARAM_GLOBAL_POINTER SynchronizedClientState* client)
{

        LOCAL_STR(orderWinStr, "Orders")
        if(GUI_BEGIN_WINDOW(GUIID_PASS, &gui->guiState.windowPositions[3],
        &gui->guiState.windowSizes[3] ,0,  orderWinStr ))
        {
            LOCAL_STR(addstr, "ADD")
            if(GUI_BUTTON(GUIID_PASS, (ge_int2)(0,0), (ge_int2)(50,50), GuiFlags_Beveled, COLOR_GREEN, addstr, NULL, NULL))
            {

            }

            const int entryHeight = 100;
            int j = 0;
            for(int i = 0; i < MAX_ORDERS; i++)
            {
                USE_POINTER Order* order;
                OFFSET_TO_PTR(gameState->orders, i, order);
                OrderEntryGui(GUIID_PASS,(ge_int2)(0,(j+1)*entryHeight), (ge_int2)(gui->guiState.windowSizes[3].x, entryHeight), 0,  order, client);
                if(order->valid)
                    j++;
                
            }
            GUI_END_WINDOW(GUIID_PASS);
        }
}



void CommandCenterMachineGui(GUIID_DEF_ALL, PARAM_GLOBAL_POINTER SynchronizedClientState* client, PARAM_GLOBAL_POINTER Machine* machine)
{
    GUI_COMMON_WIDGET_START()
    if(!goodStart)
        return ;

    USE_POINTER Order* currentRootOrder;
    OFFSET_TO_PTR(gameState->orders, machine->rootOrderPtr, currentRootOrder);
    if(currentRootOrder == NULL || !currentRootOrder->valid)
    {
        machine->rootOrderPtr = OFFSET_NULL;
        OFFSET_TO_PTR(gameState->orders, machine->rootOrderPtr, currentRootOrder);
    }



    LOCAL_STR(rstr, "Refresh")
    if(GUI_BUTTON(GUIID_PASS, (ge_int2)(100,0), (ge_int2)(100,50), GuiFlags_Beveled, COLOR_RED, rstr, NULL, NULL))
    {
            offsetPtr curOrderPtr = machine->rootOrderPtr;

            while(curOrderPtr != OFFSET_NULL)
            {
                USE_POINTER Order* curOrder;
                OFFSET_TO_PTR(gameState->orders, curOrderPtr, curOrder);

                curOrder->dirtyPathing = true;

                if(curOrder->nextExecutionOrder == machine->rootOrderPtr)
                    break;
                
                curOrderPtr = curOrder->nextExecutionOrder;
            }
    }



    LOCAL_STR(addstr, "New Order")
    if(GUI_BUTTON(GUIID_PASS, (ge_int2)(0,0), (ge_int2)(100,50), GuiFlags_Beveled, COLOR_GREEN, addstr, NULL, NULL))
    {
        if(gui->passType == GuiStatePassType_Synced)
        {
            offsetPtr newOrderPTr = Order_GetNewOrder(ALL_CORE_PARAMS_PASS);
            USE_POINTER Order* newOrder;
            OFFSET_TO_PTR(gameState->orders, newOrderPTr, newOrder);


            if(currentRootOrder == NULL || !currentRootOrder->valid)
            {
                printf("new standalone order\n");
                machine->rootOrderPtr = newOrderPTr;
                newOrder->nextExecutionOrder = newOrderPTr;
                newOrder->prevExecutionOrder = newOrderPTr;
            }
            else
            {
                offsetPtr endOrderPtr = currentRootOrder->prevExecutionOrder;
                USE_POINTER Order* endOrder;
                OFFSET_TO_PTR(gameState->orders, endOrderPtr, endOrder);

                endOrder->nextExecutionOrder = newOrderPTr;
                endOrder->dirtyPathing = true;
                currentRootOrder->prevExecutionOrder = newOrderPTr;
                currentRootOrder->dirtyPathing = true;
                
                newOrder->nextExecutionOrder = machine->rootOrderPtr;
                newOrder->prevExecutionOrder = endOrder->ptr;
            }

            

            newOrder->refCount = 1;



        }   
    }
    int numOrders = 0;
    if(machine->rootOrderPtr != OFFSET_NULL)
    {
        const int entryHeight = 100;
        const int entryGap = 10;
        int totalHeight = machine->orderLen*(entryHeight+entryGap);

        if(GUI_SCROLLBOX_BEGIN(GUIID_PASS, (ge_int2)(0,50), (ge_int2)(origSize.x, origSize.y - 50), 0, (ge_int2)(1000,  totalHeight), &gui->guiState.scrollBoxes_x[0], &gui->guiState.scrollBoxes_y[0] ))
        {
            offsetPtr firstOrder = machine->rootOrderPtr;
            offsetPtr curOrder = machine->rootOrderPtr;

            int j = 0;
            while( curOrder != OFFSET_NULL )   
            {
                USE_POINTER Order* order;
                OFFSET_TO_PTR(gameState->orders, curOrder, order);

                OrderEntryGui(GUIID_PASS, (ge_int2)(0,(j)*(entryHeight+entryGap)), (ge_int2)(origSize.x, entryHeight), 0,  order, client);
            
                numOrders++;

                if(order->valid)
                    j++;
                else
                    break;

                if(order->nextExecutionOrder == firstOrder)
                    break;
                curOrder = order->nextExecutionOrder;
            
            }

            GUI_SCROLLBOX_END(GUIID_PASS);
        }
            
        machine->orderLen = numOrders;
    }

}



void MachineGui(ALL_CORE_PARAMS, 
PARAM_GLOBAL_POINTER SyncedGui* gui,
 PARAM_GLOBAL_POINTER SynchronizedClientState* client)
{
    if(client->selectedMachine != OFFSET_NULL)
    {

        USE_POINTER Machine* mach;
        OFFSET_TO_PTR(gameState->machines, client->selectedMachine, mach);
        CL_CHECK_NULL(mach);

        USE_POINTER MachineDesc* desc;
        OFFSET_TO_PTR(gameState->machineDescriptions, mach->MachineDescPtr, desc);
        CL_CHECK_NULL(desc);

        USE_POINTER MachineRecipe* recip;
        OFFSET_TO_PTR(gameState->machineRecipes, mach->recipePtr, recip);

        LOCAL_STRL(mw, "Machine ------", mwlen); 
        CL_ITOA(client->selectedMachine, (mw)+8,mwlen-8, 10);


        
        gui->guiState.windowSizes[1] = (ge_int2)(600,600);


        if(GUI_BEGIN_WINDOW(GUIID_PASS, &gui->guiState.windowPositions[1],
            &gui->guiState.windowSizes[1],0,  mw ))
        {
            

            if(desc->type == MachineTypes_COMMAND_CENTER)
            {
                CommandCenterMachineGui(GUIID_PASS, (ge_int2)(0,0), gui->guiState.windowSizes[1],0, client, mach);
            }
            else
            {
                LOCAL_STRL(thinkingtxt2, "---", thinkingtxtLen); 
                float perc = ((float)mach->tickProgess/desc->processingTime);
                char* p = CL_ITOA(perc*100, thinkingtxt2, thinkingtxtLen, 10 );
                *(p+2) = '%'; if(thinkingtxt2[1] == '\0') thinkingtxt2[1] = ' ';
                GUI_LABEL(GUIID_PASS, (ge_int2)(0,0), (ge_int2)(perc*gui->guiState.windowSizes[1].x,50), 0, thinkingtxt2, (float3)(0,0.7,0) );

                
                int downDummy;
                LOCAL_STR(stateStrStart, "Start"); 
                LOCAL_STR(stateStrStop, "Stop"); 
                char* stateStr = stateStrStart;
                float3 btnColor;

                if( mach->state == MachineState_Running )
                {
                    stateStr = stateStrStop;
                    btnColor = (float3)(1.0,0.0,0.0);
                }
                else if( mach->state == MachineState_Idle )
                {
                    stateStr = stateStrStart;
                    btnColor = (float3)(0.0,0.7,0.0);
                }

                if(GUI_BUTTON(GUIID_PASS, (ge_int2)(0,50), (ge_int2)(50,50), GuiFlags_Beveled, btnColor, stateStr, &downDummy, NULL))
                {
                    if(gui->passType == GuiStatePassType_Synced)
                    {    
                        if( mach->state == MachineState_Running)
                        {
                            mach->state = MachineState_Idle;
                        }
                        else if( mach->state == MachineState_Idle)
                        {
                            mach->state = MachineState_Running;
                        }

                        
                    }
                }
                if(GUI_BUTTON(GUIID_PASS, (ge_int2)(50,50), (ge_int2)(50,50), GuiFlags_Beveled,(float3)(0,0,1.0), NULL, &downDummy, NULL))
                {
                    for(int i = 0; i < 8; i++)
                    {
                        if(recip->inputTypes[i] != ItemType_INVALID_ITEM)
                            mach->inventoryIn.counts[recip->inputTypes[i]]+=20;
                    }
                }


                InventoryGui(GUIID_PASS,  (ge_int2)(0,0), gui->guiState.windowSizes[1],0,&mach->inventoryIn );    
                InventoryGui(GUIID_PASS,  (ge_int2)(0,100), gui->guiState.windowSizes[1],0,&mach->inventoryOut);            
            }



           
            GUI_END_WINDOW(GUIID_PASS);
        }
    }
}


void GuiCode(ALL_CORE_PARAMS, 
PARAM_GLOBAL_POINTER SynchronizedClientState* client, 
PARAM_GLOBAL_POINTER ClientAction* clientAction, 
PARAM_GLOBAL_POINTER SyncedGui* gui, 
bool guiIsLocalClient, 
ge_int2 mouseLoc,
int mouseState,
int cliId)
{


    GUI_RESET(ALL_CORE_PARAMS_PASS, gui, mouseLoc, mouseState, gui->passType, guiIsLocalClient);

    int downDummy;
    char btntxt[9] = "CLICK ME"; 
    btntxt[8] = '\0';
    

    //handle hidden tools
    if(client->curTool == EditorTools_TileTargetSelect)
    {
        for(int i = 0; i < NUM_EDITOR_MENU_TABS; i++)
            gui->guiState.menuToggles[0] = false;
    }


    LOCAL_STR(noneTxt, "SELECT");
    if(GUI_BUTTON(GUIID_PASS, (ge_int2){0 ,0}, (ge_int2){100, 50}, GuiFlags_Beveled, GUI_BUTTON_COLOR_DEF, noneTxt, &downDummy, &(gui->guiState.menuToggles[0])) == 1)
    {
        client->curTool = EditorTools_Select;
        GUI_UpdateToggleGroup(gui->guiState.menuToggles, NUM_EDITOR_MENU_TABS, 0);
    }
    LOCAL_STR(deleteTxt, "DELETE");
    if(GUI_BUTTON(GUIID_PASS, (ge_int2){100 ,0}, (ge_int2){100, 50},GuiFlags_Beveled, GUI_BUTTON_COLOR_DEF, deleteTxt, &downDummy, &(gui->guiState.menuToggles[1])) == 1)
    {
        //printf("delete mode.");
        client->curTool = EditorTools_Delete;
        GUI_UpdateToggleGroup(gui->guiState.menuToggles, NUM_EDITOR_MENU_TABS, 1);
    }

    LOCAL_STR(createTxt, "CREATE\nCRUSHER");
    if(GUI_BUTTON(GUIID_PASS, (ge_int2){200 ,0}, (ge_int2){100, 50}, GuiFlags_Beveled, GUI_BUTTON_COLOR_DEF, createTxt, &downDummy, &(gui->guiState.menuToggles[2])) == 1)
    {
        //  printf("create mode");
        client->curTool = EditorTools_Create;
        client->curToolMachine = MachineTypes_CRUSHER;
        GUI_UpdateToggleGroup(gui->guiState.menuToggles, NUM_EDITOR_MENU_TABS, 2);
    }

    LOCAL_STR(createTxt3, "CREATE\nSMELTER");
    if(GUI_BUTTON(GUIID_PASS, (ge_int2){300 ,0}, (ge_int2){100, 50}, GuiFlags_Beveled, GUI_BUTTON_COLOR_DEF, createTxt3, &downDummy, &(gui->guiState.menuToggles[3])) == 1)
    {
        // printf("create mode");
        client->curTool = EditorTools_Create;
        client->curToolMachine = MachineTypes_SMELTER;

        GUI_UpdateToggleGroup(gui->guiState.menuToggles, NUM_EDITOR_MENU_TABS, 3);
    }

    LOCAL_STR(createTxt2, "CREATE\nCOMMAND CENTER");
    if(GUI_BUTTON(GUIID_PASS, (ge_int2){400 ,0}, (ge_int2){100, 50}, GuiFlags_Beveled, GUI_BUTTON_COLOR_DEF, createTxt2, &downDummy, &(gui->guiState.menuToggles[4])) == 1)
    {
        // printf("create mode");
        client->curTool = EditorTools_Create;
        client->curToolMachine = MachineTypes_COMMAND_CENTER;

        GUI_UpdateToggleGroup(gui->guiState.menuToggles, NUM_EDITOR_MENU_TABS, 4);
    }


    LOCAL_STRL(labeltxt, "DEEP", labeltxtLen); 
    GUI_LABEL(GUIID_PASS, (ge_int2){0 ,50}, (ge_int2){80 ,50},0, labeltxt, (float3)(0.3,0.3,0.3));


    if(GUI_SLIDER_INT_VERTICAL(GUIID_PASS,  (ge_int2){0 ,100}, (ge_int2){80, 800}, 0, &client->mapZView, 0, MAPDEPTH))
    {

    }

    LOCAL_STRL(labeltxt2, "BIRDS\nEYE", labeltxt2Len); 
    GUI_LABEL(GUIID_PASS, (ge_int2){0 ,900}, (ge_int2){80 ,50}, 0, labeltxt2, (float3)(0.3,0.3,0.3));
        

    LOCAL_STRL(robotSelWindowStr, "Selected Robots", robotSelWindowStrLen); 
    if(GUI_BEGIN_WINDOW(GUIID_PASS,  &gui->guiState.windowPositions[0],
    &gui->guiState.windowSizes[0],0,  robotSelWindowStr ))
    {
        if(GUI_SCROLLBOX_BEGIN(GUIID_PASS, (ge_int2){0,0},
        (ge_int2){10,10},
        GuiFlags_FillParent,
        (ge_int2){1000,1000}, &gui->guiState.menuScrollx, &gui->guiState.menuScrolly))
        {
            //iterate selected peeps
            USE_POINTER Peep* p;
            OFFSET_TO_PTR(gameState->peeps, client->selectedPeepsLastIdx, p);
                
            int i = 0;
            while(p != NULL)
            {

                LOCAL_STRL(header, "Miner: ", headerLen); 
                LOCAL_STRL(peeptxt, "Select", peeptxtLen); 
                GUI_LABEL(GUIID_PASS, (ge_int2){0 ,50*i}, (ge_int2){50, 50},0, header, (float3)(0.3,0.3,0.3));
        
                if(GUI_BUTTON(GUIID_PASS, (ge_int2){50 ,50*i}, (ge_int2){50, 50},GuiFlags_Beveled,GUI_BUTTON_COLOR_DEF,  peeptxt, &downDummy, NULL))
                {
                    client->selectedPeepPrimary = p->ptr;
                }

                i++;    
                OFFSET_TO_PTR(gameState->peeps, p->prevSelectionPeepPtr[cliId], p);
            }
            GUI_SCROLLBOX_END(GUIID_PASS);
        }
        
        GUI_END_WINDOW(GUIID_PASS);
    }

    //hover stats
    if((gui->passType == GuiStatePassType_NoLogic) && (GUI_MOUSE_ON_GUI(gui) == 0))
    {

        ge_int2 world_Q16;
        world_Q16.x = gameStateActions->mouseLocWorldx_Q16;
        world_Q16.y = gameStateActions->mouseLocWorldy_Q16;

        //world to map
        //get tile etc.
        ge_int3 mapcoord_whole;
        int occluded;
        GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world_Q16, &mapcoord_whole, &occluded, client->mapZView+1);



        MapTile tileup = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, mapcoord_whole + (ge_int3)(0,0,1));
        MapTile tile = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, mapcoord_whole);
        MapTile tiledown = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, mapcoord_whole + (ge_int3)(0,0,-1));
        



        LOCAL_STRL(xtxt, "", xtxtLen); 
        //CL_ITOA(tile, xtxt, xtxtLen, 10 );
        //GUI_LABEL(GUIID_PASS, (ge_int2){300,200}, (ge_int2){100, 50}, xtxt, (float3)(0.3,0.3,0.3));
        const int widgx = 80;
        const int widgy = 800;
        

        GUI_LABEL(GUIID_PASS, (ge_int2)(widgx-5,widgy-50-5) , (ge_int2){50+10, 150+10}, 0, xtxt, (float3)(0.3,0.3,0.3));
        float2 uv = TileToUV((TileUnion){tileup});
        GUI_IMAGE(GUIID_PASS, (ge_int2)(widgx,widgy-50) , (ge_int2){50, 50}, 0, uv, uv + MAP_TILE_UV_WIDTH_FLOAT2, (float3)(1,1,1));

        uv = TileToUV((TileUnion){tile});
        GUI_IMAGE(GUIID_PASS, (ge_int2)(widgx,widgy) , (ge_int2){50, 50}, 0, uv, uv + MAP_TILE_UV_WIDTH_FLOAT2, (float3)(1,1,1));

        uv = TileToUV((TileUnion){tiledown});

        GUI_IMAGE(GUIID_PASS, (ge_int2)(widgx,widgy+50) , (ge_int2){50, 50},0,  uv, uv + MAP_TILE_UV_WIDTH_FLOAT2, (float3)(1,1,1));

        
    }


    if(gameState->mapSearchers[0].state == AStarPathFindingProgress_Searching)
    {
        LOCAL_STRL(thinkingtxt, "FINDING PATH..", thinkingtxtLen); 
        GUI_LABEL(GUIID_PASS, (ge_int2)(400,100), (ge_int2)(150,50), 0, thinkingtxt, (float3)(1.0,0,0) );
    }



    //selected machine
    MachineGui(ALL_CORE_PARAMS_PASS,  gui, client);
    
    //selected single peep
    PeepCommandGui(ALL_CORE_PARAMS_PASS, gui, client);

    //order list
    OrderListGui(ALL_CORE_PARAMS_PASS, gui, client);


    if(gui->passType == GuiStatePassType_Synced)
        printf("cli: %d, mapz: %d\n", cliId, client->mapZView);
    else{
        //  printf("(fakepass) cli: %d, mapz: %d\n", cliId, client->mapZView);
    }


    //selection box
    GUI_RESET_POST(ALL_CORE_PARAMS_PASS,  gui);




}




__kernel void game_hover_gui(ALL_CORE_PARAMS)
{

    int cliId = gameStateActions->clientId;
    USE_POINTER SynchronizedClientState* client =  ThisClient(ALL_CORE_PARAMS_PASS);
    USE_POINTER ClientAction* clientAction = &gameStateActions->clientActions[cliId].action;
    USE_POINTER ActionTracking* actionTracking = &gameStateActions->clientActions[cliId].tracking;
    USE_POINTER SyncedGui* gui = &gameState->fakePassGui;
    gui->passType = GuiStatePassType_NoLogic;
    ge_int2 mouseLoc = (ge_int2){gameStateActions->mouseLocx, gameStateActions->mouseLocy };
    int mouseState = gameStateActions->mouseState;
    bool guiIsLocalClient = true;


    GuiCode(ALL_CORE_PARAMS_PASS, 
        client, 
        clientAction, 
        gui, 
        guiIsLocalClient, 
        mouseLoc, 
        mouseState,  
        cliId);
}




__kernel void game_apply_actions(ALL_CORE_PARAMS)
{
    //apply turns
    for (int32_t a = 0; a < gameStateActions->numActions; a++)
    {

        int b = a;
        GuiStatePassType guiPass = GuiStatePassType_Synced;


        USE_POINTER ClientAction* clientAction = &gameStateActions->clientActions[b].action;
        USE_POINTER ActionTracking* actionTracking = &gameStateActions->clientActions[b].tracking;
        int cliId = actionTracking->clientId;
        USE_POINTER SynchronizedClientState* client = &gameState->clientStates[cliId];
        USE_POINTER SyncedGui* gui = &gameState->clientStates[cliId].gui;
        

        //detect new clients
        if(gameState->numClients < cliId+1)
        {
            printf("New Client Connected!\n");
            gameState->numClients = cliId+1;
        }

        ge_int2 mouseLoc;
        int mouseState;
        bool guiIsLocalClient = false;
        
        gui->passType = GuiStatePassType_Synced;
        mouseLoc.x = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_X];
        mouseLoc.y = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_Y];
        mouseState = clientAction->intParameters[CAC_MouseStateChange_Param_BUTTON_BITS];

        if(client == ThisClient(ALL_CORE_PARAMS_PASS))
            guiIsLocalClient = true;

        PrintMouseState( mouseState);
        printf("IsLocalClient: %d\n", guiIsLocalClient);
        printf("a = %d\n", a);

        GuiCode(ALL_CORE_PARAMS_PASS, 
        client, 
        clientAction, 
        gui, 
        guiIsLocalClient, 
        mouseLoc, 
        mouseState, 
        cliId);

        if(guiPass == GuiStatePassType_NoLogic)
            continue;

        
        if (clientAction->actionCode == ClientActionCode_MouseStateChange)
        {
            
            printf("Processing Action From Client: %d\n", cliId);
            int buttons = clientAction->intParameters[CAC_MouseStateChange_Param_BUTTON_BITS];



            
            //end selection
            if(BITGET_MF(buttons, MouseButtonBits_PrimaryPressed) && (GUI_MOUSE_ON_GUI(gui) == 0))
            {

                printf("Starting Drag Selection..\n");
                client->mouseGUIBegin.x = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_X];
                client->mouseGUIBegin.y = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_Y];
                client->mouseWorldBegin_Q16.x = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16];
                client->mouseWorldBegin_Q16.y = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16];

            }
            else if(BITGET(buttons, MouseButtonBits_PrimaryReleased) && (gui->draggedOff == 0) && client->curTool == EditorTools_Select)
            {
                printf("Ending Drag Selection..\n");
                
                client->mouseGUIEnd.x = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_X];
                client->mouseGUIEnd.y = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_Y];
                client->mouseWorldEnd_Q16.x = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16];
                client->mouseWorldEnd_Q16.y = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16];
                
                //sort the selection box
                int nex = max(client->mouseWorldEnd_Q16.x, client->mouseWorldBegin_Q16.x);
                int ney  = max(client->mouseWorldEnd_Q16.y, client->mouseWorldBegin_Q16.y);

                int nsx  = min(client->mouseWorldEnd_Q16.x, client->mouseWorldBegin_Q16.x);
                int nsy  = min(client->mouseWorldEnd_Q16.y, client->mouseWorldBegin_Q16.y);
                client->mouseWorldEnd_Q16.x = nex;
                client->mouseWorldEnd_Q16.y = ney;
                client->mouseWorldBegin_Q16.x = nsx;
                client->mouseWorldBegin_Q16.y = nsy;


                {
                    client->selectedPeepsLastIdx = OFFSET_NULL;
                    client->selectedPeepPrimary = OFFSET_NULL;
                    int selectionCount = 0;
                    for (cl_uint pi = 0; pi < MAX_PEEPS; pi++)
                    {


                        USE_POINTER Peep* p = &gameState->peeps[pi];
                        p->prevSelectionPeepPtr[cliId] = OFFSET_NULL;
                        p->nextSelectionPeepPtr[cliId] = OFFSET_NULL;


                        if (p->stateBasic.faction == actionTracking->clientId)
                        {
                            if ((p->physics.base.pos_Q16.x > client->mouseWorldBegin_Q16.x)
                            && (p->physics.base.pos_Q16.x < client->mouseWorldEnd_Q16.x))
                            {

                                if ((p->physics.base.pos_Q16.y < client->mouseWorldEnd_Q16.y)
                                    && (p->physics.base.pos_Q16.y > client->mouseWorldBegin_Q16.y))
                                {
                                    if (PeepMapVisiblity(ALL_CORE_PARAMS_PASS, p, client->mapZView))
                                    {

                                        if (client->selectedPeepsLastIdx != OFFSET_NULL)
                                        {
                                            gameState->peeps[client->selectedPeepsLastIdx].nextSelectionPeepPtr[cliId] = pi;
                                            p->prevSelectionPeepPtr[cliId] = client->selectedPeepsLastIdx;
                                            p->nextSelectionPeepPtr[cliId] = OFFSET_NULL;
                                        }
                                        client->selectedPeepsLastIdx = pi;
                                        client->selectedPeepPrimary = pi;
                                        selectionCount++;
                                        PrintSelectionPeepStats(ALL_CORE_PARAMS_PASS, p);

                                    }

                                }
                            }
                        }

                    }

                    if(selectionCount != 1)
                    {
                        client->selectedPeepPrimary = OFFSET_NULL;
                    }

                }

            }

            
            //command to location
            if(BITGET(buttons, MouseButtonBits_SecondaryReleased) && (GUI_MOUSE_ON_GUI(gui) == 0) 
            && (gui->draggedOff == 0))
            {
                cl_uint curPeepIdx = client->selectedPeepsLastIdx;
                ge_int3 mapcoord;
                ge_int2 world2D;
                AStarPathFindingProgress pathFindProgress;

                offsetPtr pathOPtr;
                if (curPeepIdx != OFFSET_NULL)
                {
                    //Do an AStarSearch_IDA
                    USE_POINTER Peep* curPeep = &gameState->peeps[curPeepIdx];
                    world2D.x = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16];
                    world2D.y = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16];
                    int occluded;

                    GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world2D, &mapcoord, &occluded, client->mapZView);
                    mapcoord.z++;
                   
                    ge_int3 firstPeepMapCoord = GE_INT3_WHOLE_Q16(curPeep->posMap_Q16);




                    ge_int3 start;
                    start = firstPeepMapCoord;
                    
                    printf("start: ");Print_GE_INT3(start);
                    printf("end: ");  Print_GE_INT3(mapcoord);

                    //create order imeddiate, and queue up path creation.
                    USE_POINTER Order* newMainOrder;
                    offsetPtr mainPathOrderPtr = Order_GetNewOrder(ALL_CORE_PARAMS_PASS);
                    OFFSET_TO_PTR(gameState->orders, mainPathOrderPtr, newMainOrder);
                    newMainOrder->valid = true;
                    newMainOrder->ptr = mainPathOrderPtr;
                    newMainOrder->pendingDelete = false;
                    newMainOrder->mapDest_Coord = mapcoord;
                    newMainOrder->prevExecutionOrder = OFFSET_NULL;
                    newMainOrder->nextExecutionOrder = OFFSET_NULL;
                    newMainOrder->pathToDestPtr = AStarPathStepsNextFreePathNode(&gameState->paths);
                    newMainOrder->aStarJobPtr = AStarJob_EnqueueJob(ALL_CORE_PARAMS_PASS);
                    

                    USE_POINTER AStarJob* job;
                    OFFSET_TO_PTR(gameState->mapSearchJobQueue, newMainOrder->aStarJobPtr, job);
                    job->startLoc = start;
                    job->endLoc = mapcoord;
                    job->pathPtr = newMainOrder->pathToDestPtr;










                    while (curPeepIdx != OFFSET_NULL)
                    {
                        USE_POINTER Peep* curPeep = &gameState->peeps[curPeepIdx];
                        

                        start = GE_INT3_WHOLE_Q16(curPeep->posMap_Q16);
                        
                        //create order for local paths
                        USE_POINTER Order* newOrder;
                        offsetPtr newOrderPtr = Order_GetNewOrder(ALL_CORE_PARAMS_PASS);
                        OFFSET_TO_PTR(gameState->orders, newOrderPtr, newOrder);
                        newOrder->valid = true;
                        newOrder->ptr = newOrderPtr;
                        newOrder->pendingDelete = false;
                        newOrder->mapDest_Coord = firstPeepMapCoord;
                        newOrder->prevExecutionOrder = OFFSET_NULL;
                        newOrder->nextExecutionOrder = mainPathOrderPtr;
                        newOrder->pathToDestPtr = AStarPathStepsNextFreePathNode(&gameState->paths);
                        newOrder->aStarJobPtr = AStarJob_EnqueueJob(ALL_CORE_PARAMS_PASS);

                        newOrder->refCount++;
                        newMainOrder->refCount++;

                        USE_POINTER AStarJob* job;
                        OFFSET_TO_PTR(gameState->mapSearchJobQueue, newOrder->aStarJobPtr, job);
                        job->startLoc = start;
                        job->endLoc = firstPeepMapCoord;
                        job->pathPtr = newOrder->pathToDestPtr;


                        Peep_AssignOrder(ALL_CORE_PARAMS_PASS, curPeep, newOrderPtr);
                        
                        curPeepIdx = curPeep->prevSelectionPeepPtr[cliId];
                    }

                }


            }
            
            //delete
            if(BITGET(buttons, MouseButtonBits_PrimaryReleased) && (GUI_MOUSE_ON_GUI(gui) == 0) 
            && (gui->draggedOff == 0) && client->curTool == EditorTools_Delete)
            {
                ge_int2 world2DMouse;
                world2DMouse.x = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16];
                world2DMouse.y = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16];



                ge_int3 mapCoord;
                int occluded;
                GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world2DMouse, &mapCoord, &occluded, client->mapZView+1);
                
                Print_GE_INT3(mapCoord);
                MapDeleteTile(ALL_CORE_PARAMS_PASS, mapCoord);
               

            }

            //create
            if(BITGET(buttons, MouseButtonBits_PrimaryReleased) && (GUI_MOUSE_ON_GUI(gui) == 0) 
            && (gui->draggedOff == 0) && client->curTool == EditorTools_Create)
            {
                ge_int2 world2DMouse;
                world2DMouse.x = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16];
                world2DMouse.y = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16];

                ge_int3 mapCoord;
                int occluded;
                GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world2DMouse, &mapCoord, &occluded, client->mapZView+1);
                

                Print_GE_INT3(mapCoord);
                if (mapCoord.z >= 0 && mapCoord.z < MAPDEPTH-1) 
                {
                    ge_int3 mapCoordSpawn;
                    mapCoordSpawn.x = mapCoord.x;
                    mapCoordSpawn.y = mapCoord.y;
                    mapCoordSpawn.z = mapCoord.z+1;

                    USE_POINTER cl_uint* tileData = &gameState->map.levels[mapCoordSpawn.z].data[mapCoordSpawn.x][mapCoordSpawn.y]; 

                    if(MapDataGetTile(*tileData) == MapTile_NONE)
                    {

                        offsetPtr machinePtr = Machine_CreateMachine(ALL_CORE_PARAMS_PASS);
                        gameState->map.levels[mapCoordSpawn.z].machinePtr[mapCoordSpawn.x][mapCoordSpawn.y] = machinePtr;

                        USE_POINTER Machine* machine;
                        OFFSET_TO_PTR(gameState->machines, machinePtr, machine);
                        CL_CHECK_NULL(machine);

                        machine->valid = true;
                        machine->mapTilePtr = VECTOR3_CAST(mapCoordSpawn, offsetPtrShort3);
                        machine->MachineDescPtr = client->curToolMachine;
                        machine->recipePtr = gameState->validMachineRecipes[client->curToolMachine][0];


                        USE_POINTER MachineDesc* machDesc;
                        OFFSET_TO_PTR(gameState->machineDescriptions, machine->MachineDescPtr, machDesc);
                        CL_CHECK_NULL(machDesc);

                        *tileData = machDesc->tile;
                        BITSET(*tileData, MapTileFlags_Explored);
                    }
                    


                    MapBuildTileView3Area(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y);
                    MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y);
                    MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x + 1, mapCoord.y);
                    MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x - 1, mapCoord.y);
                    MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y + 1);
                    MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y - 1);
                }

            }

            //select tile
            if(BITGET(buttons, MouseButtonBits_PrimaryReleased) && (GUI_MOUSE_ON_GUI(gui) == 0) 
            && (gui->draggedOff == 0) && (client->curTool == EditorTools_Select || client->curTool == EditorTools_TileTargetSelect))
            {
                ge_int2 world2DMouse;
                world2DMouse.x = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16];
                world2DMouse.y = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16];

                ge_int3 mapCoord;
                int occluded;
                GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world2DMouse, &mapCoord, &occluded, client->mapZView+1);
                

                
                client->selectedMapCoord = INT3_TO_SHORT3(mapCoord);

                offsetPtr machinePtr = gameState->map.levels[mapCoord.z].machinePtr[mapCoord.x][mapCoord.y];



                if(machinePtr != OFFSET_NULL)
                {
                    USE_POINTER Machine* machine;
                    OFFSET_TO_PTR(gameState->machines, machinePtr, machine);
                    printf("machine selected\n");
                }

                if(client->curTool == EditorTools_Select)
                    client->selectedMachine = machinePtr;

                if(client->curTool == EditorTools_TileTargetSelect)
                {
                    printf("target selected.\n");
                    client->tileTargetMapCoord = mapCoord;
                    client->tileTargetMapCoord.z++;
                    client->tileTargetFound = true;
                }

            }




        }



       
   
    }

    gameStateActions->numActions = 0;
}

ge_int3 MapTileWholeToMapTileCenterQ16(int x, int y, int z)
{
    ge_int3 mapCoordsTileCenter_Q16 = (ge_int3){TO_Q16(x) + (TO_Q16(1) >> 1),
    TO_Q16(y) + (TO_Q16(1) >> 1) ,
    TO_Q16(z) + (TO_Q16(1) >> 1) };
    return mapCoordsTileCenter_Q16;
}

void MapCreateSlope(ALL_CORE_PARAMS, int x, int y)
{
    ge_int3 world_Q16;
    ge_int3 mapCoords2D_Q16 = MapTileWholeToMapTileCenterQ16(x, y, 0);


    MapToWorld(mapCoords2D_Q16, &world_Q16);
    ge_int2 world2d_Q16 = (ge_int2){world_Q16.x, world_Q16.y };

    ge_int3 mapCoordWhole;//top tile
    int occluded;
    GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world2d_Q16,
        &mapCoordWhole, &occluded, MAPDEPTH - 1);

    USE_POINTER cl_uint* tileData = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoordWhole);
    //do 3x3 kernel test


    //offsets[22] = (ge_int3){ 1, 1, 0 };
    //offsets[23] = (ge_int3){ -1, 1, 0 };
    //offsets[24] = (ge_int3){ 1, -1, 0 };
    //offsets[25] = (ge_int3){ -1, -1, 0 };

    USE_POINTER cl_uint* data22 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, INT3_ADD(mapCoordWhole, staticData->directionalOffsets[22]));
    MapTile tile22 = MapDataGetTile(*data22);
    if (tile22 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerBTMRIGHT);
    }
    USE_POINTER cl_uint* data24 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, INT3_ADD(mapCoordWhole, staticData->directionalOffsets[24]));
    MapTile tile24 = MapDataGetTile(*data24);
    if (tile24 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerTPRIGHT);
    }
    USE_POINTER cl_uint* data23 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, INT3_ADD(mapCoordWhole, staticData->directionalOffsets[23]));
    MapTile tile23 = MapDataGetTile(*data23);
    if (tile23 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerBTMLEFT);
    }
    USE_POINTER cl_uint* data25 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, INT3_ADD(mapCoordWhole, staticData->directionalOffsets[25]));
    MapTile tile25 = MapDataGetTile(*data25);
    if (tile25 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerTPLEFT);
    }








    //offsets[0] = (ge_int3){ 1, 0, 0 };
    //offsets[1] = (ge_int3){ -1, 0, 0 };
    //offsets[2] = (ge_int3){ 0, -1, 0 };
    //offsets[3] = (ge_int3){ 0, 1, 0 };
    //offsets[4] = (ge_int3){ 0, 0, 1 };
    //offsets[5] = (ge_int3){ 0, 0, -1 };



    USE_POINTER cl_uint* data0 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, INT3_ADD(mapCoordWhole, staticData->directionalOffsets[0]));
    MapTile tile0 = MapDataGetTile(*data0);
    if (tile0 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerBTMRIGHT);
        BITSET(*tileData, MapTileFlags_LowCornerTPRIGHT);
    }
    USE_POINTER cl_uint* data1 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, INT3_ADD(mapCoordWhole, staticData->directionalOffsets[1]));
    MapTile tile1 = MapDataGetTile(*data1);
    if (tile1 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerBTMLEFT);
        BITSET(*tileData, MapTileFlags_LowCornerTPLEFT);
    }
    USE_POINTER cl_uint* data2 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, INT3_ADD(mapCoordWhole, staticData->directionalOffsets[2]));
    MapTile tile2 = MapDataGetTile(*data2);
    if (tile2 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerTPRIGHT);
        BITSET(*tileData, MapTileFlags_LowCornerTPLEFT);
    }
    USE_POINTER cl_uint* data3 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, INT3_ADD(mapCoordWhole, staticData->directionalOffsets[3]));
    MapTile tile3 = MapDataGetTile(*data3);
    if (tile3 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerBTMRIGHT);
        BITSET(*tileData, MapTileFlags_LowCornerBTMLEFT);
    }


    
    if(MapDataLowCornerCount(*tileData) == 4)
        MapDataSetTile(tileData, MapTile_NONE);

    #ifdef ALL_EXPLORED
        BITSET(*tileData, MapTileFlags_Explored);
    #endif

}

void MapCreate(ALL_CORE_PARAMS, int x, int y)
{
    //printf("Creating Map..\n");

    int i = 0;

            cl_int perlin_z_Q16 = cl_perlin_2d_Q16(TO_Q16(x), TO_Q16(y), TO_Q16(1) >> 6, 8, 0) ;

            #ifdef FLATMAP
            perlin_z_Q16 = TO_Q16(1) >> 1;
            #endif

           
            for (int z = 0; z < MAPDEPTH; z++)
            {
                int zPerc_Q16 = DIV_PAD_Q16(TO_Q16(z), TO_Q16(MAPDEPTH));
                int depthFromSurface = perlin_z_Q16 - zPerc_Q16;
                MapTile tileType = MapTile_NONE;
                if (zPerc_Q16 < perlin_z_Q16)
                {
                    tileType = MapTile_Rock;

                    if (RandomRange(x * y * z, 0, 20) == 1)
                    {
                        tileType = MapTile_IronOre;
                    }
                    else if (RandomRange(x * y * z + 1, 0, 20) == 1)
                    {
                        tileType = MapTile_GoldOre;
                    }
                    else if (RandomRange(x * y * z + 2, 0, 100) == 1)
                    {
                        tileType = MapTile_DiamondOre;
                    }

                    if (z == 0)
                    {
                        tileType = MapTile_Lava;
                    }
                }
                else
                {
                    tileType = MapTile_NONE;
                }

                //int depthPerc_Q16 = TO_Q16(2) + perlin_z_Q16 - zPerc_Q16;
                //depthPerc_Q16 = DIV_PAD_Q16(depthPerc_Q16, TO_Q16(3));
                ////PrintQ16(depthPerc_Q16*100);

                //if (depthPerc_Q16 * 100 > TO_Q16(90))
                //{
                //    tileType = MapTile_DiamondOre;
                //}
                //else if (depthPerc_Q16 * 100 > TO_Q16(80))
                //{
                //    tileType = MapTile_CopperOre;
                //}
                //else if (depthPerc_Q16 * 100 > TO_Q16(75))
                //{
                //    tileType = MapTile_Rock;
                //}
                //else if (depthPerc_Q16 * 100 > TO_Q16(70))
                //{
                //    tileType = MapTile_Dirt;
                //}
                //else if (depthPerc_Q16 * 100 > TO_Q16(65))
                //{
                //    tileType = MapTile_DarkGrass;
                //}

                gameState->map.levels[z].machinePtr[x][y] = OFFSET_NULL;

                USE_POINTER cl_uint* data = &gameState->map.levels[z].data[x][y];
                *data = tileType;
                #ifdef ALL_EXPLORED
                BITSET(*data, MapTileFlags_Explored);
                #endif

                cl_uint cpy = *data;
                BITBANK_SET_SUBNUMBER_UINT(&cpy, MapTileFlags_RotBit1, 2, RandomRange(x*y,0,4));
                *data = cpy;

                i++;
            }





}
void MapCreate2(ALL_CORE_PARAMS, int x, int y)
{
    //MapCreateSlope(ALL_CORE_PARAMS_PASS, x, y);
    MapBuildTileView(ALL_CORE_PARAMS_PASS, x, y);
    MapUpdateShadow(ALL_CORE_PARAMS_PASS, x, y);

}

void StartupTests()
{
  printf("StartupTests Tests------------------------------------------------------:\n");
  if(0){
  printf("Speed Tests:\n");

    int s = 0;
    for (cl_ulong i = 0; i < 1000; i++)
    {
        ge_int3 a = (ge_int3){ TO_Q16(i), TO_Q16(2), TO_Q16(i*2) };
        ge_int3 b = (ge_int3){ TO_Q16(i*2), TO_Q16(i), TO_Q16(i) };

        ge_int3 c = GE_INT3_MUL_Q16(a, b);
        s += c.x + c.y + c.z;
    }

  }
  if(0)
  {
    fixedPointTests();
  }

  if(0)
  {

    printf("Triangle Tests\n");

    ge_int3 point = (ge_int3){TO_Q16(1), TO_Q16(0), TO_Q16(1) };
    Triangle3DHeavy tri;
    tri.base.verts_Q16[0] = (ge_int3){ TO_Q16(-1), TO_Q16(-1), TO_Q16(0) };
    tri.base.verts_Q16[1] = (ge_int3){ TO_Q16( 1), TO_Q16(-1), TO_Q16(0) };
    tri.base.verts_Q16[2] = (ge_int3){ TO_Q16(-1), TO_Q16( 1), TO_Q16(0) };

    Triangle3DMakeHeavy(&tri);

    int dist;
    ge_int3 closestPoint = Triangle3DHeavy_ClosestPoint(&tri, point, &dist);
    printf("closest point: ");
    Print_GE_INT3_Q16(closestPoint);
    printf("Dist: ");
    PrintQ16(dist);
  }


  if(0)
  {
    printf("Convex Hull Tests:\n");


    ConvexHull hull;    
    cl_uint tileData = 1;
    MapTileConvexHull_From_TileData(&hull, &tileData);

    printf("convex hull tris:\n");
    for (int i = 0; i < 14; i++)
    {
        Print_GE_INT3_Q16(hull.triangles[i].base.verts_Q16[0]);
        Print_GE_INT3_Q16(hull.triangles[i].base.verts_Q16[1]);
        Print_GE_INT3_Q16(hull.triangles[i].base.verts_Q16[2]);
    }


    ge_int3 p = (ge_int3){TO_Q16(0),TO_Q16(0),TO_Q16(2)};
    ge_int3 nearestPoint = MapTileConvexHull_ClosestPointToPoint(&hull, p);

    printf("nearest point: ");
    Print_GE_INT3_Q16(nearestPoint);

    p = (ge_int3){TO_Q16(0),TO_Q16(0),TO_Q16(0)};
    cl_uchar inside = MapTileConvexHull_PointInside(&hull, p);
    printf("should be inside(1): %d\n", inside);

    p = (ge_int3){TO_Q16(0),TO_Q16(1),TO_Q16(0)};
    inside = MapTileConvexHull_PointInside(&hull, p);
    printf("should be outside(0): %d\n", inside);
  }
    printf("End Tests-----------------------------------------------------------------\n");

}








void MapExplorerSpawn(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER MapExplorerAgent* agent)
{
    ge_int3 randomTileLoc;
    randomTileLoc.x = RandomRange((int)agent, 0, MAPDIM);
    randomTileLoc.y = RandomRange((int)agent, 0, MAPDIM);
    randomTileLoc.z = RandomRange((int)agent, 0, MAPDEPTH);


    USE_POINTER cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, randomTileLoc);
    if(MapDataGetTile(*data) == MapTile_NONE && BITGET(*data, MapTileFlags_Explored))
    {
        
    }
}

void AStarPathStepsInit(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER AStarPathSteps* steps)
{
    for (int i = 0; i < ASTARPATHSTEPSSIZE; i++)
    {
        AStarInitPathNode(&gameState->paths.pathNodes[i]);
    }

    //paths initialize
    gameState->paths.nextListIdx = 0;
    for(int i = 0; i < ASTAR_MAX_PATHS; i++)
        gameState->paths.pathStarts[i] = OFFSET_NULL;

}

void CLIENT_InitClientState(PARAM_GLOBAL_POINTER SynchronizedClientState* client)
{
    client->selectedMachine = OFFSET_NULL;
    client->selectedPeepPrimary = OFFSET_NULL;

}


void CLIENT_InitClientStates(ALL_CORE_PARAMS)
{
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        USE_POINTER SynchronizedClientState* client = &gameState->clientStates[i];
        CLIENT_InitClientState(client);
        USE_POINTER SyncedGui* gui = &client->gui;

        GuiState_Init(&gui->guiState);
    }
    GuiState_Init(&gameState->fakePassGui.guiState);
}



void MakeItemStrings(PARAM_GLOBAL_POINTER char* strings)
{
    //char a[ITEMTYPE_STRING_MAX_LENGTH] = "Iron Dust\0";
    //strings[0] = a;

    //ItemTypeStrings[ITEMTYPE_STRING_MAX_LENGTH][ItemTypes_NUMITEMS];

   // LOCAL_STR(a, "Iron Dust");


    //strings[1] = '\0';
    //strings[1*ITEMTYPE_STRING_MAX_LENGTH] = "Iron Dust\0";
   // strings[2*ITEMTYPE_STRING_MAX_LENGTH] = "Iron Bar\0";
    //strings[3*ITEMTYPE_STRING_MAX_LENGTH] = "Rock Dust\0";
}


__kernel void game_init_single(ALL_CORE_PARAMS)
{
    printf("Game Initializing...\n");


    printf("ASTARBFS Size: %d\n", sizeof(AStarSearch_BFS));

    printf("Initializing StaticData Buffers..\n");
    MakeCardinalDirectionOffsets(&staticData->directionalOffsets[0]);
   // MakeItemStrings(staticData->ItemTypeStrings);



    printf("Initializing GUI..\n");
    GUI_INIT_STYLE(ALL_CORE_PARAMS_PASS);

    printf("Initializing Default Client States..\n");
    CLIENT_InitClientStates(ALL_CORE_PARAMS_PASS);



    printf("Startup Tests..\n");
    StartupTests();


    printf("Creating Machines Types\n");
    Machine_InitDescriptions(ALL_CORE_PARAMS_PASS);
    printf("Creating Machine Recipes\n");
    Machine_InitRecipes(ALL_CORE_PARAMS_PASS);
    printf("Mapping Item Tiles\n");   
    InitItemTypeTiles(ALL_CORE_PARAMS_PASS);

    gameState->numClients = 1;
    gameStateActions->pauseState = 0;
    ThisClient(ALL_CORE_PARAMS_PASS)->mapZView = MAPDEPTH-1;
    ThisClient(ALL_CORE_PARAMS_PASS)->mapZView_1 = 0;

    for (int secx = 0; secx < SQRT_MAXSECTORS; secx++)
    {
        for (int secy = 0; secy < SQRT_MAXSECTORS; secy++)
        {
            gameState->sectors[secx][secy].ptr.x = secx;
            gameState->sectors[secx][secy].ptr.y = secy;
            gameState->sectors[secx][secy].lock = 0;
            gameState->sectors[secx][secy].empty = true; 
            for(int j = 0; j < MAX_PEEPS_PER_SECTOR; j++)
            {
                gameState->sectors[secx][secy].peepPtrs[j] = OFFSET_NULL;
            }
        }
    }
    printf("Sectors Initialized.\n");




    AStarSearch_BFS_Instantiate(&gameState->mapSearchers[0]);
}

__kernel void game_init_multi(ALL_CORE_PARAMS)
{
    int globalid = get_global_id(0);
    int localid = get_local_id(0);

    MapCreate(ALL_CORE_PARAMS_PASS, globalid % MAPDIM, globalid / MAPDIM);

}
__kernel void game_init_multi2(ALL_CORE_PARAMS)
{
    int globalid = get_global_id(0);
    int localid = get_local_id(0);

    MapCreate2(ALL_CORE_PARAMS_PASS, globalid % MAPDIM, globalid / MAPDIM);
}
__kernel void game_init_single2(ALL_CORE_PARAMS)
{
    AStarPathStepsInit(ALL_CORE_PARAMS_PASS, &gameState->paths);



    printf("AStarTests:\n");
    //printf("AStarTests1:\n");
    //AStarSearch_BFS_Instantiate(&gameState->mapSearchers[0]);
    ////test AStarHeap
    //for (int x = 0; x < MAPDIM; x++)
    //{
    //    for (int y = 0; y < MAPDIM; y++)
    //    {
    //        AStarNode* node = &gameState->mapSearchers[0].details[x][y][0];
    //        node->h_Q16 = x * y;
    //        node->g_Q16 = x * y;
    //        AStarOpenHeapInsert(&gameState->mapSearchers[0], node);
    //    }
    //}

    //do
    //{
    //    AStarNode* node = AStarOpenHeapRemove(&gameState->mapSearchers[0]);
    //    AStarPrintNodeStats(node);
    //} while (gameState->mapSearchers[0].openHeapSize);

    printf("AStarTests2:\n");
    //test AStar

    // ge_short3 start = (ge_short3){ 0,0,MAPDEPTH - 2 };
    // ge_short3 end = (ge_short3){ MAPDIM - 1,MAPDIM - 1,1 };
    // AStarSearch_BFS_Routine(ALL_CORE_PARAMS_PASS, &gameState->mapSearchers[0], start, end, CL_INTMAX);

    printf("initializing peeps..\n");
    const int spread = 500;
    for (cl_uint p = 0; p < MAX_PEEPS; p++)
    {
        gameState->peeps[p].ptr = p;

        gameState->peeps[p].physics.base.pos_Q16.x = RandomRange(p, -spread << 16, spread << 16);
        gameState->peeps[p].physics.base.pos_Q16.y = RandomRange(p + 1, -spread << 16, spread << 16);


        ge_int3 mapcoord;
        ge_int2 world2D;
        world2D.x = gameState->peeps[p].physics.base.pos_Q16.x;
        world2D.y = gameState->peeps[p].physics.base.pos_Q16.y;
        int occluded;

        GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world2D, &mapcoord, &occluded, MAPDEPTH - 1);
        //printf("%d\n", mapcoord.z);
        mapcoord.z += 2;
        mapcoord = GE_INT3_TO_Q16(mapcoord);

        ge_int3 worldCoord;
        MapToWorld(mapcoord, &worldCoord);
                
        gameState->peeps[p].physics.base.pos_Q16.z = worldCoord.z;

        ge_int3 pmap_Q16;
        WorldToMap(gameState->peeps[p].physics.base.pos_Q16, &pmap_Q16);

        gameState->peeps[p].posMap_Q16 = pmap_Q16;
        gameState->peeps[p].lastGoodPosMap_Q16 = gameState->peeps[p].posMap_Q16;


        gameState->peeps[p].mapCoord = VECTOR3_CAST(GE_INT3_WHOLE_Q16(gameState->peeps[p].posMap_Q16), offsetPtrShort3);
        gameState->peeps[p].mapCoord_1 = OFFSET_NULL_SHORT_3D;


        gameState->peeps[p].physics.shape.radius_Q16 = TO_Q16(1);
        BITCLEAR(gameState->peeps[p].stateBasic.bitflags0, PeepState_BitFlags_deathState);
        BITSET(gameState->peeps[p].stateBasic.bitflags0, PeepState_BitFlags_valid);
        BITSET(gameState->peeps[p].stateBasic.bitflags0, PeepState_BitFlags_visible);
        gameState->peeps[p].stateBasic.health = 10;
        gameState->peeps[p].stateBasic.deathState = 0;
        gameState->peeps[p].stateBasic.buriedGlitchState = 0;
        gameState->peeps[p].stateBasic.orderPtr = OFFSET_NULL;

      
        

        gameState->peeps[p].physics.base.v_Q16 = (ge_int3){ 0,0,0 };
        gameState->peeps[p].physics.base.vel_add_Q16 = (ge_int3){ 0,0,0 };
        gameState->peeps[p].physics.base.pos_post_Q16 = (ge_int3){ 0,0,0 };

        gameState->peeps[p].sectorPtr = OFFSET_NULL_2D;

        gameState->peeps[p].minDistPeepPtr = OFFSET_NULL;
        gameState->peeps[p].minDistPeep_Q16 = (1 << 30);
        gameState->peeps[p].physics.drive.target_x_Q16 = gameState->peeps[p].physics.base.pos_Q16.x;
        gameState->peeps[p].physics.drive.target_y_Q16 = gameState->peeps[p].physics.base.pos_Q16.y;
        gameState->peeps[p].physics.drive.targetPathNodeOPtr = OFFSET_NULL;


        gameState->peeps[p].stateBasic.faction = RandomRange(p,0,4);


        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            gameState->clientStates[i].selectedPeepsLastIdx = OFFSET_NULL;


            CL_CHECKED_ARRAY_SET(gameState->peeps[p].nextSelectionPeepPtr, MAX_CLIENTS, i, OFFSET_NULL)
            CL_CHECKED_ARRAY_SET(gameState->peeps[p].prevSelectionPeepPtr, MAX_CLIENTS, i, OFFSET_NULL)
        }

    }

    printf("Peeps Initialized.\n");






    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        USE_POINTER Particle* p = &gameState->particles[i];

        p->pos.x = FloatToQMP32((float)RandomRange(i, -spread, spread));
        p->pos.y = FloatToQMP32((float)RandomRange(i + 1, -spread, spread));

        p->vel.x = FloatToQMP32(((float)RandomRange(i, -1000, 1000)) * 0.001f);
        p->vel.y = FloatToQMP32(((float)RandomRange(i + 1, -1000, 1000)) * 0.001f);
    }


}



void UpdateMapExplorer(ALL_CORE_PARAMS,  PARAM_GLOBAL_POINTER MapExplorerAgent* agent)
{



}


void PeepDraw(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep)
{
    float3 drawColor;
    float drawPosX = (float)((float)peep->physics.base.pos_Q16.x / (1 << 16));
    float drawPosY = (float)((float)peep->physics.base.pos_Q16.y / (1 << 16));

    float brightFactor = 0.6f;
    if (peep->stateBasic.faction == 0)
    {
        drawColor.x = 0.0f;
        drawColor.y = 1.0f;
        drawColor.z = 1.0f;
    }
    else if(peep->stateBasic.faction == 1)
    {
        drawColor.x = 1.0f;
        drawColor.y = 1.0f;
        drawColor.z = 0.0f;
    }
    else if (peep->stateBasic.faction == 2)
    {
        drawColor.x = 0.0f;
        drawColor.y = 1.0f;
        drawColor.z = 0.0f;
    }
    else if (peep->stateBasic.faction == 3)
    {
        drawColor.x = 1.0f;
        drawColor.y = 0.0f;
        drawColor.z = 1.0f;
    }

    if (gameState->clientStates[gameStateActions->clientId].peepRenderSupport[peep->ptr].render_selectedByClient)
    {
        brightFactor = 1.0f;
        gameState->clientStates[gameStateActions->clientId].peepRenderSupport[peep->ptr].render_selectedByClient = 0;
    }
    if ( BITGET(peep->stateBasic.bitflags0, PeepState_BitFlags_deathState) )
    {
        brightFactor = 0.6f;
        drawColor.x = 0.5f;
        drawColor.y = 0.5f;
        drawColor.z = 0.5f;
    }
    if (!BITGET(peep->stateBasic.bitflags0, PeepState_BitFlags_visible))
    {
        drawPosX = 99999;
        drawPosY = 99999;
    }


    if(peep->physics.base.pos_Q16.z < 0)
    {   brightFactor = 1.0f;
        drawColor.x = 1.0f;
        drawColor.y = 0.0f;
        drawColor.z = 0.0f;
    }

    if(peep->stateBasic.buriedGlitchState > 0)
    { 
       
        brightFactor = 1.0f;
        drawColor.x = 0.0f;
        drawColor.y = 1.0f;
        drawColor.z = 0.0f;
    }

    peepVBOBuffer[peep->ptr * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 0] = drawPosX;
    peepVBOBuffer[peep->ptr * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 1] = drawPosY;

    peepVBOBuffer[peep->ptr * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 2] = drawColor.x * brightFactor;
    peepVBOBuffer[peep->ptr * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 3] = drawColor.y * brightFactor;
    peepVBOBuffer[peep->ptr * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 4] = drawColor.z * brightFactor;

    peepVBOBuffer[peep->ptr * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 5] = peep->physics.base.CS_angle_rad;
}

void ParticleDraw(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Particle* particle, cl_uint ptr)
{
    float3 drawColor;
    float drawPosX = (float)((float)particle->pos.x.number / (1 << particle->pos.x.q));
    float drawPosY = (float)((float)particle->pos.y.number / (1 << particle->pos.y.q));


    drawColor.x = 1.0f;
    drawColor.y = 1.0f;
    drawColor.z = 1.0f;
  

    float brightFactor = 1.0f;

    particleVBOBuffer[ptr * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 0] = drawPosX;
    particleVBOBuffer[ptr * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 1] = drawPosY;

    particleVBOBuffer[ptr * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 2] = drawColor.x * brightFactor;
    particleVBOBuffer[ptr * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 3] = drawColor.y * brightFactor;
    particleVBOBuffer[ptr * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 4] = drawColor.z * brightFactor;

    particleVBOBuffer[ptr * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 5] = 0;
}

__kernel void game_updatepre1(ALL_CORE_PARAMS)
{


    // Get the index of the current element to be processed
    int globalid = get_global_id(0);
    int localid = get_local_id(0);



    cl_ulong chunkSize = (MAX_PEEPS) / GAME_UPDATE_WORKITEMS;
    for (cl_ulong i = 0; i < chunkSize+1; i++)
    {
        cl_ulong idx = globalid+GAME_UPDATE_WORKITEMS*i;
        if (idx < MAX_PEEPS) {
            USE_POINTER Peep* p = &gameState->peeps[idx]; 
            PeepPreUpdate2(ALL_CORE_PARAMS_PASS, p);
        }
    }


    if(globalid == 0)
    {
        
        LINES_ClearAll(ALL_CORE_PARAMS_PASS);


        if (ThisClient(ALL_CORE_PARAMS_PASS)->mapZView != ThisClient(ALL_CORE_PARAMS_PASS)->mapZView_1)
        {
            ThisClient(ALL_CORE_PARAMS_PASS)->updateMap = true;
            ThisClient(ALL_CORE_PARAMS_PASS)->mapZView_1 = ThisClient(ALL_CORE_PARAMS_PASS)->mapZView;
        }
    }
}


__kernel void game_update(ALL_CORE_PARAMS)
{

    // Get the index of the current element to be processed
    int globalid = get_global_id(0);
    int localid = get_local_id(0);
    

    cl_ulong chunkSize = (MAX_PEEPS) / GAME_UPDATE_WORKITEMS;
    for (cl_ulong i = 0; i < chunkSize+1; i++)
    {
        cl_ulong idx = globalid+GAME_UPDATE_WORKITEMS*i;
        if (idx < MAX_PEEPS) {

            USE_POINTER Peep* p = &gameState->peeps[idx]; 
            PeepUpdate(ALL_CORE_PARAMS_PASS, p);
            PeepDraw(ALL_CORE_PARAMS_PASS, p);
        }
    }

    chunkSize = (MAX_MACHINES) / GAME_UPDATE_WORKITEMS;
    for (cl_ulong i = 0; i < chunkSize+1; i++)
    {
        cl_ulong idx = globalid+GAME_UPDATE_WORKITEMS*i;
        if ( idx < MAX_MACHINES)
        {
            USE_POINTER Machine* m = &gameState->machines[idx]; 
            MachineUpdate(ALL_CORE_PARAMS_PASS, m);
        }
    }

    chunkSize = (MAX_PARTICLES) / GAME_UPDATE_WORKITEMS;
    for (cl_ulong i = 0; i < chunkSize+1; i++)
    {
        cl_ulong idx = globalid+GAME_UPDATE_WORKITEMS*i;
        if (idx < MAX_PARTICLES) {
            USE_POINTER Particle* p = &gameState->particles[idx];
            ParticleUpdate(ALL_CORE_PARAMS_PASS, p);
            ParticleDraw(ALL_CORE_PARAMS_PASS, p, idx);
        }
    }
    



    //update map view
    if (ThisClient(ALL_CORE_PARAMS_PASS)->updateMap)
    {
        
        cl_ulong chunkSize = (MAPDIM * MAPDIM) / GAME_UPDATE_WORKITEMS;

        for (cl_ulong i = 0; i < chunkSize+1; i++)
        {
            cl_ulong xyIdx = globalid+GAME_UPDATE_WORKITEMS*i;

            
            if (xyIdx < (MAPDIM * MAPDIM))
            {
                MapBuildTileView(ALL_CORE_PARAMS_PASS, xyIdx % MAPDIM, xyIdx / MAPDIM);
                MapUpdateShadow(ALL_CORE_PARAMS_PASS, xyIdx % MAPDIM, xyIdx / MAPDIM);
            }
        }
    }

    //update map explorers
    USE_POINTER MapExplorerAgent* explorer = &gameState->explorerAgents[globalid];
    UpdateMapExplorer(ALL_CORE_PARAMS_PASS, explorer);


}
__kernel void game_update2(ALL_CORE_PARAMS)
{


    // Get the index of the current element to be processed
    int globalid = get_global_id(0);
    int localid = get_local_id(0);




    //reset searches
    USE_POINTER AStarSearch_BFS* search = &gameState->mapSearchers[0];
    if(search->state == AStarPathFindingProgress_ResetReady)
    {
        cl_ulong chunkSize = (MAPDIM * MAPDIM * MAPDEPTH) / GAME_UPDATE_WORKITEMS;
    
        for (cl_ulong i = 0; i < chunkSize+1; i++)
        {
            cl_long xyzIdx = globalid+GAME_UPDATE_WORKITEMS*i;
            
            if (xyzIdx < (MAPDIM * MAPDIM* MAPDEPTH))
            {

                cl_long z = xyzIdx % MAPDEPTH;
                cl_long y = (xyzIdx / MAPDEPTH) % MAPDIM;
                cl_long x = xyzIdx / (MAPDIM * MAPDEPTH); 




                if(x > MAPDIM)
                    printf("X>MAPDIM\n");
                if(y > MAPDIM)
                    printf("Y>MAPDIM\n");
                if(z > MAPDEPTH)
                    printf("Z>MAPDEPTH\n");

                if(xyzIdx == (MAPDIM * MAPDIM* MAPDEPTH)-1)
                    printf("RESETING SEARCH\n");

                AStarSearch_BFS_InstantiateParrallel(search, xyzIdx, x, y, z);
            }
        }
    }

    if(globalid == 0)
    if(ThisClient(ALL_CORE_PARAMS_PASS)->updateMap)
    {
        ThisClient(ALL_CORE_PARAMS_PASS)->updateMap = false;
    }

}


__kernel void game_post_update( ALL_CORE_PARAMS )
{
    // Get the index of the current element to be processed
    int globalid = get_global_id(0);
    int localid = get_local_id(0);


    if(globalid == 0)
    {
        //set selected peeps to highlight.
        cl_uint curPeepIdx = gameState->clientStates[gameStateActions->clientId].selectedPeepsLastIdx;
        PeepRenderSupport peepRenderSupport[MAX_PEEPS];
        while (curPeepIdx != OFFSET_NULL)
        {
            USE_POINTER Peep* p = &gameState->peeps[curPeepIdx];
            gameState->clientStates[gameStateActions->clientId].peepRenderSupport[curPeepIdx].render_selectedByClient = 1;
                    
            curPeepIdx = p->prevSelectionPeepPtr[gameStateActions->clientId];
        }

        //should be reset after gameupdate_2
        if(gameState->mapSearchers[0].state == AStarPathFindingProgress_ResetReady)
            gameState->mapSearchers[0].state = AStarPathFindingProgress_Ready;

        //update jobs
        AStarJob_UpdateJobs(ALL_CORE_PARAMS_PASS);






        //draw debug lines on paths
        #ifdef DEBUG_PATHS


        USE_POINTER AStarPathSteps* paths = &gameState->paths;

        for(int i = 0; i < ASTAR_MAX_PATHS; i++)
        {
            offsetPtr pathStartOPtr = paths->pathStarts[i];
            if(pathStartOPtr != OFFSET_NULL)
            {
                USE_POINTER AStarPathNode* node;
                OFFSET_TO_PTR(paths->pathNodes, pathStartOPtr, node)

                while(node != NULL && node->nextOPtr != OFFSET_NULL)
                {
                    USE_POINTER AStarPathNode* nodeNext;
                    OFFSET_TO_PTR(paths->pathNodes, node->nextOPtr, nodeNext);
                    CL_CHECK_NULL(nodeNext);

                    ge_int3 worldCoord_Q16;
                    MapToWorld(( node->mapCoord_Q16 ), &worldCoord_Q16);

                    ge_int3 worldCoordNext_Q16;
                    MapToWorld(( nodeNext->mapCoord_Q16 ), &worldCoordNext_Q16);

                    
                    float2 worldCoordsFloat = (float2)(FIXED2FLTQ16(worldCoord_Q16.x),FIXED2FLTQ16(worldCoord_Q16.y));
                    float2 worldCoordsNextFloat = (float2)(FIXED2FLTQ16(worldCoordNext_Q16.x),FIXED2FLTQ16(worldCoordNext_Q16.y));

                
                    LINES_DrawLineWorld(ALL_CORE_PARAMS_PASS, worldCoordsFloat, worldCoordsNextFloat, (float3)(RandomRange(i, 0, 1000)/1000.0f, RandomRange(i+1, 0, 1000)/1000.0f, 1.0f));


                    OFFSET_TO_PTR(paths->pathNodes, node->nextOPtr, node)
                }
            }
        }

        #endif
    }



    cl_ulong chunkSize = (MAX_ORDERS) / GAME_UPDATE_WORKITEMS;
    for (cl_ulong pi = 0; pi < chunkSize+1; pi++)
    {
        cl_ulong idx = globalid+(GAME_UPDATE_WORKITEMS)*pi;
        if (idx < MAX_ORDERS)
        {
            USE_POINTER Order* order;
            CL_CHECKED_ARRAY_GET_PTR(gameState->orders, MAX_ORDERS, idx, order)
            

            if(order->valid && order->pendingDelete)
            {
                if(order->nextExecutionOrder == OFFSET_NULL && order->prevExecutionOrder == OFFSET_NULL)
                {
                    printf("deleting expired and dangling order\n");
                    Order_DeleteOrder(ALL_CORE_PARAMS_PASS, idx);
                }
            }

            if(order->valid && order->refCount <= 0)
            {
                order->pendingDelete = true;
            }
        }
    }






}

__kernel void game_preupdate_1(ALL_CORE_PARAMS) {


    int globalid = get_global_id(0);

    if (globalid >= GAME_UPDATE_WORKITEMS)
        return;
   

    cl_ulong chunkSize = (MAX_PEEPS) / GAME_UPDATE_WORKITEMS;
    for (cl_ulong pi = 0; pi < chunkSize+1; pi++)
    {
        cl_ulong idx = globalid+(GAME_UPDATE_WORKITEMS)*pi;
        if (idx < MAX_PEEPS)
        {
            USE_POINTER Peep* peep;
            CL_CHECKED_ARRAY_GET_PTR(gameState->peeps, MAX_PEEPS, idx, peep)
            CL_CHECK_NULL(peep)
            
            cl_int x = ((peep->physics.base.pos_Q16.x >> 16) / (SECTOR_SIZE));
            cl_int y = ((peep->physics.base.pos_Q16.y >> 16) / (SECTOR_SIZE));

            peep->sectorPtr.x = x + SQRT_MAXSECTORS / 2;
            peep->sectorPtr.y = y + SQRT_MAXSECTORS / 2;

            USE_POINTER MapSector* sector;
            OFFSET_TO_PTR_2D(gameState->sectors, peep->sectorPtr, sector);
            sector->empty = false;
        }

    }


    //parrellel order Pre update



}


__kernel void game_preupdate_2(ALL_CORE_PARAMS) {


    // Get the index of the current element to be processed
    int globalid = get_global_id(0);

    if (globalid >= GAME_UPDATE_WORKITEMS)
        return;
   

    cl_uint chunkSize = (SQRT_MAXSECTORS*SQRT_MAXSECTORS) / GAME_UPDATE_WORKITEMS;



    for (cl_ulong pi = 0; pi < chunkSize+1; pi++)
    {
        cl_ulong idx = globalid+(GAME_UPDATE_WORKITEMS)*pi;
        if (idx < SQRT_MAXSECTORS*SQRT_MAXSECTORS)
        {
            offsetPtr2 xy;
            xy.x = idx % SQRT_MAXSECTORS;
            xy.y = idx / SQRT_MAXSECTORS;

            global volatile MapSector* mapSector;
            OFFSET_TO_PTR_2D(gameState->sectors, xy, mapSector);
            CL_CHECK_NULL(mapSector)

            if(mapSector->empty)
                continue;

            //clear non-present peeps
            for(int j = 0; j < MAX_PEEPS_PER_SECTOR; j++)
            {
                if(mapSector->peepPtrs[j] == OFFSET_NULL)
                    continue;
                USE_POINTER Peep* peep;
                OFFSET_TO_PTR(gameState->peeps, mapSector->peepPtrs[j], peep)

                if(!VECTOR2_EQUAL(peep->sectorPtr, xy))
                {
                    mapSector->peepPtrs[j] = OFFSET_NULL;
                }
            }



            for(int i = mapSector->chunkStart; (i < mapSector->chunkStart + MAX_PEEPS/16) && i < MAX_PEEPS; i++)
            {   
                USE_POINTER Peep* peep;
                CL_CHECKED_ARRAY_GET_PTR(gameState->peeps, MAX_PEEPS, i, peep)
                CL_CHECK_NULL(peep)
                
                if(VECTOR2_EQUAL(peep->sectorPtr, xy) == 0)
                    continue;

                //check if its already in the list
                if(mapSector->peepPtrs[peep->sectorListIdx] == i)
                    continue;

                peep->sectorListIdx = mapSector->ptrIterator;
                mapSector->peepPtrs[mapSector->ptrIterator] = i;
                mapSector->ptrIterator++;
                if(mapSector->ptrIterator >= MAX_PEEPS_PER_SECTOR)
                {
                    mapSector->ptrIterator = 0;
                }
            }


            mapSector->chunkStart += MAX_PEEPS/16;
            if(mapSector->chunkStart >= MAX_PEEPS)
                mapSector->chunkStart = 0;
        }
    }


}


__kernel void size_tests_kernel(PARAM_GLOBAL_POINTER SIZETESTSDATA* data)
{
    data->gameStateStructureSize = sizeof(GameState);
    data->staticDataStructSize = sizeof(StaticData);

    printf("SIZE_TESTS_KERNEL: sizeof(GameState): %ul\n", sizeof(GameState));
    printf("SIZE_TESTS_KERNEL: sizeof(StaticData): %ul\n", sizeof(StaticData));

}
