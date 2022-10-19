

#include "clCommon.h"


//#define FLATMAP
#define ALL_EXPLORED
//#define NO_ZSHADING
//#define PEEP_ALL_ALWAYS_VISIBLE
//#define PEEP_DISABLE_TILECORRECTIONS
#define PEEP_PATH_CROWD (4)

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
            return 1;
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
        //if(enterDirection.z < 0)
        //    return 0;



        ge_int3 downCoord = mapcoord;
        downCoord.z--;
        USE_POINTER cl_uint* downData = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, downCoord);
        if( MapDataGetTile(*downData) != MapTile_NONE)
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




void Machine_InitDescriptions(ALL_CORE_PARAMS)
{
    MachineDesc* m = &gameState->machineDescriptions[MachineTypes_CRUSHER];
    m->type = MachineTypes_CRUSHER;
    m->tile = MapTile_MACHINE_CRUSHER;
    m->numInputs = 1;
    m->numOutputs = 2;
    m->inputTypes[0] = ItemType_IRON_ORE;
    m->outputTypes[0] = ItemType_IRON_DUST;
    m->outputTypes[1] = ItemType_ROCK_DUST;

    m->inputRatio[0] = 1;
    m->outputRatio[0] = 1;
    m->outputRatio[1] = 5;
    m->processingTime = 30;




    m = &gameState->machineDescriptions[MachineTypes_SMELTER];
    m->type = MachineTypes_SMELTER;
    m->tile = MapTile_MACHINE_FURNACE;
    m->numInputs = 1;
    m->numOutputs = 1;
    m->inputTypes[0] = ItemType_IRON_DUST;
    m->outputTypes[1] = ItemType_IRON_BAR;

    m->inputRatio[0] = 5;
    m->outputRatio[0] = 1;

    m->processingTime = 100;




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

    return ptr;
}





























inline void AStarNodeInstantiate(PARAM_GLOBAL_POINTER AStarNode* node)
{
    
    node->g_Q16 = TO_Q16(0);
    node->h_Q16 = TO_Q16(0);
    node->nextOPtr = OFFSET_NULL_3D;  
    node->prevOPtr = OFFSET_NULL_3D;
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
                USE_POINTER AStarNode* node = &search->details[x][y][z];
                AStarNodeInstantiate(node);


                node->tileIdx = (ge_short3){x,y,z};

                search->closedMap[x][y][z] = 0;
                search->openMap[x][y][z] = 0;
            }
        }  
    }

    for(int i = 0; i < ASTARHEAPSIZE; i++)
    {
        search->openHeap_OPtrs[i] = OFFSET_NULL_3D;
    }
    
    search->startNodeOPtr = OFFSET_NULL_3D;
    search->endNodeOPtr = OFFSET_NULL_3D;
    
    search->openHeapSize = 0;
    search->pathOPtr = OFFSET_NULL;

}
void AStarSearch_BFS_InstantiateParrallel(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, cl_ulong idx, int x, int y, int z)
{

    USE_POINTER AStarNode* node = &search->details[x][y][z];
    AStarNodeInstantiate(node);
    node->tileIdx = (ge_short3){x,y,z};

    search->closedMap[x][y][z] = 0;
    search->openMap[x][y][z] = 0;



    if(idx < ASTARHEAPSIZE)
    {
        search->openHeap_OPtrs[idx] = OFFSET_NULL_3D;
       
    }
    
    
    search->startNodeOPtr = OFFSET_NULL_3D;
    search->endNodeOPtr = OFFSET_NULL_3D;
    search->openHeapSize = 0;
    search->pathOPtr = OFFSET_NULL;


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
        return 0;

    //must be adjacent to 
    bool adjacent = false;
    for(int i = 0; i < 26; i++)
    {
        ge_int3 worldCoord = staticData->directionalOffsets[i] + SHORT3_TO_INT3(node->tileIdx);
        MapTile tile = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, worldCoord);
        if(tile != MapTile_NONE)
        {
            adjacent = true;
            break;
        }
    }
    if(adjacent == false)
        return 0;


    if(delta.z > 0)
    {


      //  if(MapDataGetTile(*fromTileData) == MapTile_NONE)
     //       return 0;
        //else could be a ramp

    
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
    offsetPtr3 topOPtr = search->openHeap_OPtrs[index];
    OFFSET_TO_PTR_3D(search->details, topOPtr, top);


    while (index < search->openHeapSize / 2)
    {
        int leftChild = 2 * index + 1;
        int rightChild = leftChild + 1;

        USE_POINTER AStarNode* leftChildNode;
        OFFSET_TO_PTR_3D(search->details, search->openHeap_OPtrs[leftChild], leftChildNode)
        USE_POINTER AStarNode* rightChildNode;
        OFFSET_TO_PTR_3D(search->details, search->openHeap_OPtrs[rightChild], rightChildNode)

        if ((rightChild < search->openHeapSize) && (AStarOpenHeapKey(search, leftChildNode) > AStarOpenHeapKey(search, rightChildNode)))
            largerChild = rightChild;
        else
            largerChild = leftChild;

        USE_POINTER AStarNode* largerChildNode;

        OFFSET_TO_PTR_3D(search->details, search->openHeap_OPtrs[largerChild], largerChildNode)

        if (AStarOpenHeapKey(search, top) <= AStarOpenHeapKey(search, largerChildNode))
            break;

        search->openHeap_OPtrs[index] = search->openHeap_OPtrs[largerChild];
        index = largerChild;

        
    }
    
    search->openHeap_OPtrs[index] = topOPtr;
}

offsetPtr3 AStarOpenHeapRemove(PARAM_GLOBAL_POINTER AStarSearch_BFS* search)
{
    offsetPtr3 rootOPtr = search->openHeap_OPtrs[0];

    search->openHeap_OPtrs[0] = search->openHeap_OPtrs[search->openHeapSize-1];
    search->openHeapSize--;

 
    AStarOpenHeapTrickleDown(search, 0);

    return rootOPtr;
}

offsetPtr3 AStarRemoveFromOpen(PARAM_GLOBAL_POINTER AStarSearch_BFS* search)
{

    offsetPtr3 nodeOPtr = AStarOpenHeapRemove(search);
    

    USE_POINTER AStarNode* node;
    OFFSET_TO_PTR_3D(search->details, nodeOPtr, node);

    search->openMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z] = 0;
    return nodeOPtr;
}
void AStarAddToClosed(PARAM_GLOBAL_POINTER AStarSearch_BFS* search,PARAM_GLOBAL_POINTER AStarNode* node)
{
    search->closedMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z] = 1;
}
cl_uchar AStarNodeInClosed(PARAM_GLOBAL_POINTER AStarSearch_BFS* search,PARAM_GLOBAL_POINTER AStarNode* node)
{
    return search->closedMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z];
}

cl_uchar AStarNodeInOpen(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, PARAM_GLOBAL_POINTER AStarNode* node)
{
    return search->openMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z];
}


void AStarOpenHeapTrickleUp(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, cl_int index)
{
    cl_int prev = (index - 1) / 2;
    offsetPtr3 bottomOPtr = search->openHeap_OPtrs[index];

    USE_POINTER AStarNode* bottomNode;
    OFFSET_TO_PTR_3D(search->details, bottomOPtr, bottomNode);

    USE_POINTER AStarNode* prevNode;
    OFFSET_TO_PTR_3D(search->details, search->openHeap_OPtrs[prev], prevNode);

    while (index > 0 && AStarOpenHeapKey(search, prevNode) > AStarOpenHeapKey(search, bottomNode))
    {
        search->openHeap_OPtrs[index] = search->openHeap_OPtrs[prev];
        index = prev;
        prev = (prev - 1) / 2;
        OFFSET_TO_PTR_3D(search->details, search->openHeap_OPtrs[prev], prevNode);
    }
    search->openHeap_OPtrs[index] = bottomOPtr;
}


void AStarOpenHeapInsert(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, offsetPtr3 nodeOPtr)
{
    search->openHeap_OPtrs[search->openHeapSize] = nodeOPtr;
    AStarOpenHeapTrickleUp(search, search->openHeapSize);
    search->openHeapSize++;
    if (search->openHeapSize > ASTARHEAPSIZE)
        printf("ERROR: AStarHeap Size Greater than ASTARHEAPSIZE!\n");

}
void AStarAddToOpen(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, offsetPtr3 nodeOPtr)
{
    AStarOpenHeapInsert(search, nodeOPtr);

    USE_POINTER AStarNode* node;
    OFFSET_TO_PTR_3D(search->details, nodeOPtr, node);
    search->openMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z] = 1;
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
cl_int AStarNodeDistanceHuristic_IDA(PARAM_GLOBAL_POINTER AStarSearch_IDA* search, ge_short3 locA, ge_short3 locB)
{
    return TO_Q16(abs(locA.x - locB.x) + abs(locA.y - locB.y) + abs(locA.z - locB.z));
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
void AStarPrintSearchPathTo(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, ge_int3 destTile)
{
    USE_POINTER AStarNode* curNode = &search->details[destTile.x][destTile.y][destTile.z];
    while (curNode != NULL)
    {
        AStarPrintNodeStats(curNode);
        OFFSET_TO_PTR_3D(search->details, curNode->prevOPtr, curNode);
    }
}
void AStarPrintSearchPathFrom(PARAM_GLOBAL_POINTER AStarSearch_BFS* search, ge_int3 startTile)
{
    USE_POINTER AStarNode* curNode = &search->details[startTile.x][startTile.y][startTile.z];
    while (curNode != NULL)
    {
        AStarPrintNodeStats(curNode);
        OFFSET_TO_PTR_3D(search->details, curNode->nextOPtr, curNode);
    }
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


offsetPtr AStarFormPathSteps(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER AStarSearch_BFS* search, PARAM_GLOBAL_POINTER AStarPathSteps* steps)
{
    //grab a unused Node from pathNodes, and start building the list .
    offsetPtr3 curNodeOPtr =  search->startNodeOPtr;
    USE_POINTER AStarNode* curNode;
    OFFSET_TO_PTR_3D(search->details, curNodeOPtr, curNode);
    CL_CHECK_NULL(curNode);

    offsetPtr startNodeOPtr = OFFSET_NULL;
    USE_POINTER AStarPathNode* pNP = NULL;
    int i = 0;
    while (curNode != NULL)
    { 

        int index = AStarPathStepsNextFreePathNode(&gameState->paths);

        USE_POINTER AStarPathNode* pN = &gameState->paths.pathNodes[index];

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

        if (!VECTOR3_EQUAL(curNode->nextOPtr, OFFSET_NULL_3D)) 
        {

            OFFSET_TO_PTR_3D(search->details, curNode->nextOPtr, curNode);

            if(0)
            {
                //iterate until joint in path.
                ge_int3 delta;
                USE_POINTER AStarNode* n2 = curNode;
                do
                {
                    OFFSET_TO_PTR_3D(search->details, n2->nextOPtr, n2);

                    if (n2 != NULL) {
                        delta = INT3_ADD(SHORT3_TO_INT3(n2->tileIdx), INT3_NEG(holdTileCoord));
                    }
                    else
                        delta = (ge_int3){ 0,0,0 };

                } while ((n2 != NULL) && (GE_INT3_WHACHAMACOLIT1_ENTRY(delta) == 1));

                if (n2 != NULL) 
                {
                    USE_POINTER AStarNode* n2Prev;
                    OFFSET_TO_PTR_3D(search->details, n2->prevOPtr, n2Prev);

                    if (curNode != n2Prev)
                        curNode = n2Prev;
                    else
                        curNode = n2;
                }
                else
                {
                    OFFSET_TO_PTR_3D(search->details, search->endNodeOPtr, curNode);
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

    steps->pathStarts[steps->nextPathStartIdx] = startNodeOPtr;
    steps->nextPathStartIdx++;

    if(steps->nextPathStartIdx >= ASTAR_MAX_PATHS)
    {
        steps->nextPathStartIdx = ASTAR_MAX_PATHS-1;
        printf("Max Paths Reached!\n");
    }

  //  OFFSET_TO_PTR(steps->pathNodes, startNodeOPtr, curNode2);
   // curNode2->prevOPtr = OFFSET_NULL;

    return startNodeOPtr;
}



offsetPtr AStarFormPathSteps_IDA(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER AStarSearch_IDA* search, PARAM_GLOBAL_POINTER AStarPathSteps* steps)
{
    //grab a unused Node from pathNodes, and start building the list .

    USE_POINTER AStarNode_IDA* curNode;
    OFFSET_TO_PTR(search->path, 0, curNode);

    offsetPtr startNodeOPtr = OFFSET_NULL;
    USE_POINTER AStarPathNode* pNP = NULL;
    int i = 0;
    while (curNode != NULL && i <= search->pathEndIdx)
    { 
        int index = AStarPathStepsNextFreePathNode(&gameState->paths);

        USE_POINTER AStarPathNode* pN = &gameState->paths.pathNodes[index];

        if (i == 0)
            startNodeOPtr = index;

        ge_int3 holdTileCoord = SHORT3_TO_INT3(  curNode->tileLoc  );

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


        i++;
        OFFSET_TO_PTR(search->path, i, curNode);
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

    steps->pathStarts[steps->nextPathStartIdx] = startNodeOPtr;
    steps->nextPathStartIdx++;

    if(steps->nextPathStartIdx >= ASTAR_MAX_PATHS)
    {
        steps->nextPathStartIdx = ASTAR_MAX_PATHS-1;
        printf("Max Paths Reached!\n");
    }

    return startNodeOPtr;
}


AStarPathFindingProgress AStarSearch_BFS_Continue(ALL_CORE_PARAMS,PARAM_GLOBAL_POINTER AStarSearch_BFS* search, int iterations)
{
    USE_POINTER AStarNode* startNode;
    USE_POINTER AStarNode* targetNode;
    OFFSET_TO_PTR_3D(search->details, search->startNodeOPtr,startNode);
    OFFSET_TO_PTR_3D(search->details, search->endNodeOPtr,targetNode);


    //printf("AStarSearch_BFS_Continue..openHeapSize: %d\n", search->openHeapSize);
    while (search->openHeapSize > 0 && iterations > 0)
    {
        //printf("AStarSearch_BFS_Continue iterating..%d\n", iterations);
        //find node in open with lowest f cost
        offsetPtr3 currentOPtr = AStarRemoveFromOpen(search);


        USE_POINTER AStarNode* current;
        OFFSET_TO_PTR_3D(search->details, currentOPtr, current);

        //printf("G: "); PrintQ16(current->g_Q16); printf(" H: "); PrintQ16(current->h_Q16);

        AStarAddToClosed(search, current);//visited
        if (VECTOR3_EQUAL(SHORT3_TO_INT3( current->tileIdx ), search->endNodeOPtr) )
        {
            printf("AStarSearch_BFS_Continue AStarPathFindingProgress_Finished\n");
            search->state = AStarPathFindingProgress_Finished;
            
            USE_POINTER AStarNode* endNode;
            OFFSET_TO_PTR_3D(search->details, search->endNodeOPtr, endNode);
            CL_CHECK_NULL(endNode);

            endNode->nextOPtr = OFFSET_NULL_3D;
            startNode->prevOPtr = OFFSET_NULL_3D;

            //form next links
            USE_POINTER AStarNode* curNode = targetNode;
            offsetPtr3 curNodeOPtr = search->endNodeOPtr;

            while (curNode != NULL)
            {
                USE_POINTER AStarNode* p;
                OFFSET_TO_PTR_3D(search->details, curNode->prevOPtr, p);

                if(p != NULL)
                    p->nextOPtr = curNodeOPtr;

                curNodeOPtr = curNode->prevOPtr;
                curNode = p;
            }

            //form a simplified path
            search->pathOPtr = AStarFormPathSteps(ALL_CORE_PARAMS_PASS , search, &gameState->paths);


            return search->state;//found dest
        }
        
        



        //5 neighbors
        for (int i = 0; i <= 5; i++)
        { 
            ge_int3 prospectiveTileCoord;
            ge_int3 dir = staticData->directionalOffsets[i];
            prospectiveTileCoord.x = current->tileIdx.x + dir.x;
            prospectiveTileCoord.y = current->tileIdx.y + dir.y;
            prospectiveTileCoord.z = current->tileIdx.z + dir.z;
 
            if (MapTileCoordValid(prospectiveTileCoord, 1)==0)
            {
                continue;
            }


            
            //if lateral dyagonol, check adjacents a for traversability as well. if all traverible - diagonoal is traversible.
            // if(GE_INT3_ENTRY_COUNT(dir) == 2 && dir.z == 0)
            // {
            //     ge_int3 dirNoX = dir;
            //     dirNoX.x=0;
            //     ge_int3 dirNoY = dir;
            //     dirNoX.y=0;

            //     ge_int3 XCheckCoord;
            //     ge_int3 YCheckCoord;


            //     offsetPtr3 XCheckNodeOPtr = (offsetPtr3){XCheckCoord.x,XCheckCoord.y,XCheckCoord.z};
            //     AStarNode* XCheckNode;
            //     OFFSET_TO_PTR_3D(search->details, XCheckNodeOPtr, XCheckNode);
            //     if ((AStarNode2NodeTraversible(ALL_CORE_PARAMS_PASS,  XCheckNode, current) == 0))
            //     {
            //         continue;
            //     }

            //     offsetPtr3 YCheckNodeOPtr = (offsetPtr3){YCheckCoord.x,YCheckCoord.y,YCheckCoord.z};
            //     AStarNode* YCheckNode;
            //     OFFSET_TO_PTR_3D(search->details, YCheckNodeOPtr, YCheckNode);
            //     if ((AStarNode2NodeTraversible(ALL_CORE_PARAMS_PASS,  YCheckNode, current) == 0))
            //     {
            //         continue;
            //     }
            // }

            offsetPtr3 prospectiveNodeOPtr = (offsetPtr3){prospectiveTileCoord.x,prospectiveTileCoord.y,prospectiveTileCoord.z};
            USE_POINTER AStarNode* prospectiveNode;
            OFFSET_TO_PTR_3D(search->details, prospectiveNodeOPtr, prospectiveNode);

            if ((AStarNode2NodeTraversible(ALL_CORE_PARAMS_PASS,  prospectiveNode, current) == 0) || (AStarNodeInClosed(search, prospectiveNode)))
            {
                continue;
            }




            int totalMoveCost = current->g_Q16 + AStarNodeDistanceHuristic(search, current, prospectiveNode);
           // PrintQ16(totalMoveCost); PrintQ16(current->g_Q16); PrintQ16(prospectiveNode->g_Q16);
            if (((totalMoveCost < prospectiveNode->g_Q16) || AStarNodeInOpen(search, prospectiveNode) == 0) )
            {
                

                prospectiveNode->g_Q16 = totalMoveCost;
                prospectiveNode->h_Q16 = AStarNodeDistanceHuristic(search, prospectiveNode, targetNode);
                
                prospectiveNode->prevOPtr = currentOPtr;

                //printf("G: "); PrintQ16(prospectiveNode->g_Q16); printf("H: ");  PrintQ16(prospectiveNode->h_Q16);
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

cl_uchar AStarSearch_BFS_Routine(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER AStarSearch_BFS* search, ge_int3 startTile, ge_int3 destTile, int startIterations)
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

    printf("starting search\n");
    search->state = AStarPathFindingProgress_Searching;
    search->startNodeOPtr = (offsetPtr3){startTile.x,startTile.y,startTile.z};
    search->endNodeOPtr = (offsetPtr3){destTile.x,destTile.y,destTile.z};
    
    USE_POINTER AStarNode* startNode;
    USE_POINTER AStarNode* targetNode;
    OFFSET_TO_PTR_3D(search->details, search->startNodeOPtr,startNode);
    OFFSET_TO_PTR_3D(search->details, search->endNodeOPtr,targetNode);

    //add start to openList
    startNode->h_Q16 = AStarNodeDistanceHuristic(search, startNode, targetNode);
    AStarAddToOpen(search, search->startNodeOPtr );


    return AStarSearch_BFS_Continue(ALL_CORE_PARAMS_PASS, search, startIterations);
}
bool AStarSearch_IDA_Node_In_Path(PARAM_GLOBAL_POINTER AStarSearch_IDA* search, AStarNode_IDA* node)
{
    for(int i = 0; i < search->pathEndIdx; i++)
    {
        if(VECTOR3_EQUAL(search->path[i].tileLoc, node->tileLoc))
            return true;
    }
    return false;
}
bool AStarSearch_IDA_Loc_In_Path(PARAM_GLOBAL_POINTER AStarSearch_IDA* search, ge_short3 loc)
{
    for(int i = 0; i < search->pathEndIdx; i++)
    {
        if(VECTOR3_EQUAL(search->path[i].tileLoc, loc))
            return true;
    }
    return false;
}
void AStarSearch_IDA_InitNode(PARAM_GLOBAL_POINTER AStarSearch_IDA* search, PARAM_GLOBAL_POINTER AStarNode_IDA* node)
{
    node->tileLoc = (ge_short3)(-1,-1,-1);
    for(int i = 0; i <= 25; i++)
        node->searchedSuccessors[i] = false;
}

ge_short3 AStarSearch_IDA_NodeGrabNextBestSuccessor(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER AStarSearch_IDA* search, PARAM_GLOBAL_POINTER AStarNode_IDA* node,  int* hCost,  bool* failed)
{

    int minCost = INT_MAX;
    int minCosti = 0;
    int i;
    *failed = true;
    for (i = 0; i <= 3; i++)
    { 
        if(node->searchedSuccessors[i])
            continue;

        ge_short3 prospectiveTileCoord;
        ge_int3 dir = staticData->directionalOffsets[i];
        prospectiveTileCoord.x = node->tileLoc.x + dir.x;
        prospectiveTileCoord.y = node->tileLoc.y + dir.y;
        prospectiveTileCoord.z = node->tileLoc.z + dir.z;

        if (MapTileCoordValid(SHORT3_TO_INT3( prospectiveTileCoord),1)==0)
        {
            continue;
        }

        USE_POINTER cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, SHORT3_TO_INT3( prospectiveTileCoord));
        MapTile tile = MapDataGetTile(*data);
        if (tile != MapTile_NONE)
        {
            continue;
        }



        int cost = AStarNodeDistanceHuristic_IDA(search, prospectiveTileCoord, search->endLoc);
        if(cost < minCost)
        {
            minCost = cost;
            minCosti = i;
        }
        *failed = false;
    }



    *hCost = minCost;
    node->searchedSuccessors[minCosti] = true;
    return  node->tileLoc + INT3_TO_SHORT3( staticData->directionalOffsets[minCosti] );
}

int AStarSearch_IDA_Search(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER AStarSearch_IDA* search)
{
    int pathRootsStack[ASTARSEARCH_IDA_PATHMAXSIZE];
    int rootStackIdx=0;

    cl_long iterations = 0;
    while(true)
    {
        USE_POINTER AStarNode_IDA* node = &search->path[search->pathEndIdx];
        int h = AStarNodeDistanceHuristic_IDA(search, node->tileLoc, search->endLoc);
        int f = node->gCost + h;
        
        printf("\nsearching..");Print_GE_SHORT3(node->tileLoc);
        iterations++;
        if(f > search->bound)
        {
            //cost is too much - pop.
            search->pathEndIdx--; 
            printf("pop\n");
            if(search->pathEndIdx <=0) 
            {
                printf("The cost is too much.\n");
                search->pathEndIdx = 0;
                search->state = AStarPathFindingProgress_Failed;
                return 0;
            }
                
            

            continue;
        }
            

        if(VECTOR3_EQUAL(node->tileLoc, search->endLoc))
        {
            printf("AStarSearch_IDA_Search Found Goal\n");
            printf("Iterations: %d\n", iterations);
            search->state = AStarPathFindingProgress_Finished;
            search->pathOPtr = AStarFormPathSteps_IDA(ALL_CORE_PARAMS_PASS , search, &gameState->paths);
            return 0;//found
        }
            

        int min = INT_MAX;


        //choose successor
        int hcost;
        bool failed;
        ge_short3 sLoc = AStarSearch_IDA_NodeGrabNextBestSuccessor(ALL_CORE_PARAMS_PASS, search,  node, &hcost, &failed);
        if(failed)
        {
            //pop.
            search->pathEndIdx--; 
            printf("pop\n");
            if(search->pathEndIdx <=0) 
            {
                printf("The cost is too much.\n");
                printf("Iterations: %d\n", iterations);
                search->pathEndIdx = 0;
                search->state = AStarPathFindingProgress_Failed;
                return 0;
            }
            continue;
        }


        int costDiff = h - hcost;//cost of transition
        

        if(!AStarSearch_IDA_Loc_In_Path(search, sLoc))
        {
            //Push
            search->pathEndIdx++;
            //AStarSearch_IDA_InitNode(search, &search->path[search->pathEndIdx]);
            search->path[search->pathEndIdx].tileLoc = (sLoc);
            search->path[search->pathEndIdx].gCost = node->gCost + costDiff;
            printf("push\n");
        }
        //endloop
    }

    search->state = AStarPathFindingProgress_Failed;
    return 0;
}


AStarPathFindingProgress AStarSearch_IDA_Continue(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER AStarSearch_IDA* search, int iterations)
{
    search->t = AStarSearch_IDA_Search(ALL_CORE_PARAMS_PASS, search);
    if(search->t == 0)
    {
        //path found
        search->state = AStarPathFindingProgress_Finished;
        printf("AStarSearch_IDA_Continue Path Found\n");
    }
    if(search->t == 10000)
    {
        //not found
        search->state = AStarPathFindingProgress_Failed;
        printf("AStarSearch_IDA_Continue Path Failed\n");
    }
    search->bound = search->t ;
}




cl_uchar AStarSearch_IDA_Routine(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER AStarSearch_IDA* search, ge_short3 startTile, ge_short3 destTile, int startIterations)
{
    if (MapTileCoordValid(SHORT3_TO_INT3( startTile ),1) == 0)
    {
        printf("AStarSearch_IDA_Routine: Start MapCoord Not Valid.\n");
        return 0;
    }
    if (MapTileCoordValid(SHORT3_TO_INT3(destTile),1) == 0)
    {
        printf("AStarSearch_IDA_Routine: Dest MapCoord Not Valid.\n");
        return 0;
    }
    if (MapTileCoordStandInValid(ALL_CORE_PARAMS_PASS, SHORT3_TO_INT3(startTile)) == 0)
    {
        printf("AStarSearch_IDA_Routine: Start Stand In Invalid.\n");
        return 0;
    }
    if (MapTileCoordStandInValid(ALL_CORE_PARAMS_PASS, SHORT3_TO_INT3(destTile))==0)
    {
        printf("AStarSearch_IDA_Routine: Dest Stand In Invalid.\n");
        return 0;
    }

    printf("AStarSearch_IDA_Routine: starting search\n");
    search->state = AStarPathFindingProgress_Searching;


    search->bound = AStarNodeDistanceHuristic_IDA(search, startTile, destTile);
    search->path[0].tileLoc = startTile;
    search->pathEndIdx=0;
    search->startLoc = startTile;
    search->endLoc = destTile;

    return AStarSearch_IDA_Continue(ALL_CORE_PARAMS_PASS, search, startIterations);
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



void MapTileConvexHull_From_TileData( ConvexHull* hull,  cl_int* tileData)
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
   cl_int* out_tile_data)
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
    cl_int tileDatas[26];


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
  
            MapTileConvexHull_From_TileData(&hull, &tileDatas[i]);
            ge_int3 peepPosLocalToHull_Q16 = INT3_SUB(futurePos, tileCenters_Q16[i]);

            peepPosLocalToHull_Q16 = GE_INT3_DIV_Q16(peepPosLocalToHull_Q16, (ge_int3) {
                TO_Q16(MAP_TILE_SIZE), TO_Q16(MAP_TILE_SIZE)
                    , TO_Q16(MAP_TILE_SIZE)
            });

            nearestPoint = MapTileConvexHull_ClosestPointToPoint(&hull, peepPosLocalToHull_Q16);
            insideSolidRegion = MapTileConvexHull_PointInside(&hull, peepPosLocalToHull_Q16);

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

    if (WHOLE_Q16(len) < 2)//within range of current target
    {


        if (peep->physics.drive.targetPathNodeOPtr == OFFSET_NULL)
        {
            //final node reached.
            peep->comms.message_TargetReached_pending = 255;//send the message
            
        }
        else
        {
            //advance if theres room
            USE_POINTER AStarPathNode* targetPathNode;
            OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.targetPathNodeOPtr,targetPathNode);


            USE_POINTER AStarPathNode* prevpathNode;
            OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.prevPathNodeOPtr,prevpathNode);

            //advance
            peep->physics.drive.prevPathNodeOPtr = peep->physics.drive.targetPathNodeOPtr;        
            peep->physics.drive.targetPathNodeOPtr = targetPathNode->nextOPtr;

            
            if (peep->physics.drive.targetPathNodeOPtr != OFFSET_NULL) 
            {
                USE_POINTER AStarPathNode* targetPathNode;
                OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.targetPathNodeOPtr,targetPathNode);
                

                ge_int3 nextTarget_Q16 = targetPathNode->mapCoord_Q16;
                MapToWorld(nextTarget_Q16, &nextTarget_Q16);

                peep->physics.drive.target_x_Q16 = nextTarget_Q16.x;
                peep->physics.drive.target_y_Q16 = nextTarget_Q16.y;
                peep->physics.drive.target_z_Q16 = nextTarget_Q16.z;
            }
            
        }
        
    }

   

    //advacne if theres room
    USE_POINTER AStarPathNode* targetPathNode;
    OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.targetPathNodeOPtr,targetPathNode);
   

    // Print_GE_INT3_Q16(peep->physics.base.pos_Q16);
    if(targetPathNode != NULL)
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
        //else
        {
         //   printf("pc: %d, ",peepCnt);
        }
    }


    peep->physics.base.CS_angle_rad = atan2(((float)(d.x))/(1<<16), ((float)(d.y)) / (1 << 16));




}

void WalkAndFight(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER Peep* peep)
{



    //if search is done - start the peep on path
    if(peep->stateBasic.aStarSearchPtr != OFFSET_NULL)
    {
        USE_POINTER AStarSearch_BFS* search = &gameState->mapSearchers[0];
        if(search->state == AStarPathFindingProgress_Finished)
        {
            peep->physics.drive.targetPathNodeOPtr = search->pathOPtr;
            peep->physics.drive.prevPathNodeOPtr = OFFSET_NULL;
            //AStarPathNode* nxtPathNode;
            //OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.targetPathNodeOPtr, nxtPathNode);
            

           // printf("path assining to %d\n", search->pathOPtr);
            peep->stateBasic.aStarSearchPtr = OFFSET_NULL;
        }
        
    }


    if(peep->physics.drive.targetPathNodeOPtr != OFFSET_NULL)
    {


        //drive to the next path node
        USE_POINTER AStarPathNode* nxtPathNode;
        OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.targetPathNodeOPtr, nxtPathNode);


            ge_int3 worldloc;
            MapToWorld(nxtPathNode->mapCoord_Q16, &worldloc);
            peep->physics.drive.target_x_Q16 = worldloc.x;
            peep->physics.drive.target_y_Q16 = worldloc.y;
            peep->physics.drive.target_z_Q16 = worldloc.z;
            peep->physics.drive.drivingToTarget = 1;

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
        peep->physics.drive.drivingToTarget = 0;
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
    cl_int tileData;
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
    if (!VECTOR3_EQUAL(maptilecoords, maptilecoords_prev) || (ThisClient(ALL_CORE_PARAMS_PASS)->mapZView_1 != ThisClient(ALL_CORE_PARAMS_PASS)->mapZView))
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
    cl_int tileData;
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
    if(gameState->debugLinesIdx >= MAX_LINES)
        printf("out of debug line space!\n");
}
void LINES_ClearAll(ALL_CORE_PARAMS)
{
    for(int i = gameState->debugLinesIdx; i >=0; i--)
        linesVBO[gameState->debugLinesIdx] = 0.0f;

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
    float4 screenPosStart4 = Matrix_Float4_Times_Vec4(&gameStateActions->viewMatrix, worldPosStart4);
    float2 screenPosStart2 = (float2)(screenPosStart4.x, screenPosStart4.y);

    float4 worldPosEnd4 = (float4)(worldPosEnd.x, worldPosEnd.y, 0.0f, 1.0f);
    float4 screenPosEnd4 = Matrix_Float4_Times_Vec4(&gameStateActions->viewMatrix, worldPosEnd4);
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
    float4 worldPos = Matrix_Float4_Times_Vec4(&gameStateActions->viewMatrix_Inv, v);

    printf("%f, %f, %f, %f\n", worldPos.x, worldPos.y, worldPos.z, worldPos.w);

    return (ge_int2){0,0};
}

float2 MapTileToUV(MapTile tile)
{

    //duplicate of geomMapTile.glsl code.
    float2 uv;
    uv.x = ((uint)tile & 15u) / 16.0;
    uv.y = (((uint)tile >> 4u) & 15u) / 16.0;    
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

__kernel void game_apply_actions(ALL_CORE_PARAMS)
{

    cl_uint curPeepIdx = gameState->clientStates[gameStateActions->clientId].selectedPeepsLastIdx;
    PeepRenderSupport peepRenderSupport[MAX_PEEPS];
    while (curPeepIdx != OFFSET_NULL)
    {
        USE_POINTER Peep* p = &gameState->peeps[curPeepIdx];
        gameState->clientStates[gameStateActions->clientId].peepRenderSupport[curPeepIdx].render_selectedByClient = 1;

        curPeepIdx = p->prevSelectionPeepPtr[gameStateActions->clientId];
    }


    //apply turns
    for (int32_t a = 0; a < gameStateActions->numActions+1; a++)
    {
        int b = a;
        GuiStatePassType guiPass = GuiStatePassType_Synced;
        if(a == gameStateActions->numActions)
        {
            b = 0;//'local' client
            guiPass = GuiStatePassType_NoLogic;
        }

        USE_POINTER ClientAction* clientAction = &gameStateActions->clientActions[b].action;
        USE_POINTER ActionTracking* actionTracking = &gameStateActions->clientActions[b].tracking;
        int cliId = actionTracking->clientId;
        USE_POINTER SynchronizedClientState* client = &gameState->clientStates[cliId];
        USE_POINTER SyncedGui* gui = &gameState->clientStates[cliId].gui;
        



        ge_int2 mouseLoc;
        int mouseState;
        bool guiIsLocalClient = false;
        if(guiPass == GuiStatePassType_NoLogic)//redirect pointers above so they reflect the local client only.
        {
            guiPass = GuiStatePassType_NoLogic;
            mouseLoc = (ge_int2){gameStateActions->mouseLocx, gameStateActions->mouseLocy };
            mouseState = gameStateActions->mouseState;
            client = ThisClient(ALL_CORE_PARAMS_PASS);
            cliId = gameStateActions->clientId;
            clientAction= &gameStateActions->clientActions[cliId].action;
            actionTracking= &gameStateActions->clientActions[cliId].tracking;
            
            gui = &gameState->fakePassGui;
            guiIsLocalClient = true;
            
        }
        else
        {
            guiPass = GuiStatePassType_Synced;
            mouseLoc.x = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_X];
            mouseLoc.y = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_Y];
            mouseState = clientAction->intParameters[CAC_MouseStateChange_Param_BUTTON_BITS];

            if(client == ThisClient(ALL_CORE_PARAMS_PASS))
                guiIsLocalClient = true;

            PrintMouseState( mouseState);
            printf("IsLocalClient: %d\n", guiIsLocalClient);
        }



        GUI_RESET(ALL_CORE_PARAMS_PASS, gui, mouseLoc, mouseState, guiPass, guiIsLocalClient);

        int downDummy;
        char btntxt[9] = "CLICK ME"; 
        btntxt[8] = '\0';
        
        


        LOCAL_STR(noneTxt, "NONE");
        if(GUI_BUTTON(GUIID_PASS, (ge_int2){0 ,0}, (ge_int2){100, 50}, 0, noneTxt, &downDummy, &(gui->guiState.menuToggles[0])) == 1)
        {
            client->curTool = EditorTools_None;
            GUI_UpdateToggleGroup(gui->guiState.menuToggles, 4, 0);
        }


        LOCAL_STR(deleteTxt, "DELETE");
        if(GUI_BUTTON(GUIID_PASS, (ge_int2){100 ,0}, (ge_int2){100, 50},0, deleteTxt, &downDummy, &(gui->guiState.menuToggles[1])) == 1)
        {
            //printf("delete mode.");
            client->curTool = EditorTools_Delete;
            GUI_UpdateToggleGroup(gui->guiState.menuToggles, 4, 1);
        }

        LOCAL_STR(createTxt, "CREATE\nCRUSHER");
        if(GUI_BUTTON(GUIID_PASS, (ge_int2){200 ,0}, (ge_int2){100, 50}, 0, createTxt, &downDummy, &(gui->guiState.menuToggles[2])) == 1)
        {
          //  printf("create mode");
            client->curTool = EditorTools_Create;
            client->curToolMachine = MachineTypes_CRUSHER;
            GUI_UpdateToggleGroup(gui->guiState.menuToggles, 4, 2);
        }

        LOCAL_STR(createTxt2, "CREATE\nSMELTER");
        if(GUI_BUTTON(GUIID_PASS, (ge_int2){300 ,0}, (ge_int2){100, 50}, 0, createTxt2, &downDummy, &(gui->guiState.menuToggles[3])) == 1)
        {
           // printf("create mode");
            client->curTool = EditorTools_Create;
            client->curToolMachine = MachineTypes_SMELTER;

            GUI_UpdateToggleGroup(gui->guiState.menuToggles, 4, 3);
        }




        LOCAL_STRL(labeltxt, "DEEP", labeltxtLen); 
        GUI_LABEL(GUIID_PASS, (ge_int2){0 ,50}, (ge_int2){80 ,50},0, labeltxt, (float3)(0.3,0.3,0.3));


        GUI_SLIDER_INT_VERTICAL(GUIID_PASS,  (ge_int2){0 ,100}, (ge_int2){80, 800},0, &client->mapZView, 0, MAPDEPTH);

        LOCAL_STRL(labeltxt2, "BIRDS\nEYE", labeltxt2Len); 
        GUI_LABEL(GUIID_PASS, (ge_int2){0 ,900}, (ge_int2){80 ,50}, 0, labeltxt2, (float3)(0.3,0.3,0.3));
            
        

        LOCAL_STRL(robotSelWindowStr, "Selected Robots", robotSelWindowStrLen); 
        GUI_BEGIN_WINDOW(GUIID_PASS, (ge_int2){100,100},
        (ge_int2){200,200},0,  robotSelWindowStr, &gui->guiState.windowPositions[0],&gui->guiState.windowSizes[0] );

        GUI_SCROLLBOX_BEGIN(GUIID_PASS, (ge_int2){0,0},
        (ge_int2){10,10},
        GuiFlags_FillParent,
        (ge_int2){1000,1000}, &gui->guiState.menuScrollx, &gui->guiState.menuScrolly);
            //iterate selected peeps
            USE_POINTER Peep* p;
            OFFSET_TO_PTR(gameState->peeps, client->selectedPeepsLastIdx, p);
                  
            int i = 0;
            while(p != NULL)
            {

                LOCAL_STRL(header, "Miner: ", headerLen); 
                LOCAL_STRL(peeptxt, "------------", peeptxtLen); 
                CL_ITOA(p->physics.drive.targetPathNodeOPtr, peeptxt, peeptxtLen, 10 );
                GUI_LABEL(GUIID_PASS, (ge_int2){0 ,50*i}, (ge_int2){50, 50},0, header, (float3)(0.3,0.3,0.3));
        
                GUI_BUTTON(GUIID_PASS, (ge_int2){50 ,50*i}, (ge_int2){50, 50},0, peeptxt, &downDummy, NULL);

                i++;    
                OFFSET_TO_PTR(gameState->peeps, p->prevSelectionPeepPtr[cliId], p);
            }
        GUI_SCROLLBOX_END(GUIID_PASS);
        
        GUI_END_WINDOW(GUIID_PASS);
        

        //hover stats
        if(guiPass == GuiStatePassType_NoLogic)
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
            float2 uv = MapTileToUV(tileup);
            GUI_IMAGE(GUIID_PASS, (ge_int2)(widgx,widgy-50) , (ge_int2){50, 50}, 0, uv, uv + MAP_TILE_UV_WIDTH_FLOAT2, (float3)(1,1,1));

            uv = MapTileToUV(tile);
            GUI_IMAGE(GUIID_PASS, (ge_int2)(widgx,widgy) , (ge_int2){50, 50}, 0, uv, uv + MAP_TILE_UV_WIDTH_FLOAT2, (float3)(1,1,1));

            uv = MapTileToUV(tiledown);
            GUI_IMAGE(GUIID_PASS, (ge_int2)(widgx,widgy+50) , (ge_int2){50, 50},0,  uv, uv + MAP_TILE_UV_WIDTH_FLOAT2, (float3)(1,1,1));

        }

        if(gameState->mapSearchers[0].state == AStarPathFindingProgress_Searching)
        {
            LOCAL_STRL(thinkingtxt, "FINDING PATH..", thinkingtxtLen); 
            GUI_LABEL(GUIID_PASS, (ge_int2)(400,100), (ge_int2)(150,50), 0, thinkingtxt, (float3)(1.0,0,0) );
        }







        if(guiPass == GuiStatePassType_Synced)
            printf("cli: %d, mapz: %d\n", cliId, client->mapZView);
        else{
          //  printf("(fakepass) cli: %d, mapz: %d\n", cliId, client->mapZView);
        }


        //selection box
        GUI_RESET_POST(ALL_CORE_PARAMS_PASS,  gui);





        if(guiPass == GuiStatePassType_NoLogic)
            continue;

        
        if (clientAction->actionCode == ClientActionCode_MouseStateChange)
        {
            
            printf("Processing Action From Client: %d\n", cliId);
            int buttons = clientAction->intParameters[CAC_MouseStateChange_Param_BUTTON_BITS];



            
            //end selection
            if(BITGET_MF(buttons, MouseButtonBits_PrimaryPressed))
            {

                if(GUI_MOUSE_ON_GUI(gui) == 0)
                {
                    printf("Starting Drag Selection..\n");
                    client->mouseGUIBegin.x = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_X];
                    client->mouseGUIBegin.y = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_Y];
                    client->mouseWorldBegin_Q16.x = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16];
                    client->mouseWorldBegin_Q16.y = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16];

                    client->mouseOnGUiBegin = 0;
                }
                else
                {
                    client->mouseOnGUiBegin = 1;
                }
            }
            else if(BITGET(buttons, MouseButtonBits_PrimaryReleased) && (gui->draggedOff == 0))
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
                    for (cl_uint pi = 0; pi < MAX_PEEPS; pi++)
                    {
                        USE_POINTER Peep* p = &gameState->peeps[pi];
                        p->prevSelectionPeepPtr[cliId] = OFFSET_NULL;
                        p->nextSelectionPeepPtr[cliId] = OFFSET_NULL;


                        if (p->stateBasic.faction == actionTracking->clientId)
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

                                    PrintSelectionPeepStats(ALL_CORE_PARAMS_PASS, p);

                                }

                            }
                        }
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

                    ge_int3 start = GE_INT3_WHOLE_Q16(curPeep->posMap_Q16);
                    mapcoord.z++;
                    ge_int3 end = mapcoord;
                    printf("start: ");Print_GE_INT3(start);
                    printf("end: ");Print_GE_INT3(end);

                    //AStarSearch_BFS_Instantiate(&gameState->mapSearchers[0]);


                    ge_int3 worldloc;
                    MapToWorld(GE_INT3_TO_Q16(end), &worldloc);


                    if(gameState->mapSearchers[0].state == AStarPathFindingProgress_Ready)
                    {
                        printf("Starting Search for selected peeps..\n");
                        AStarPathFindingProgress prog = AStarSearch_BFS_Routine(ALL_CORE_PARAMS_PASS, &gameState->mapSearchers[0], (start), (end), 0);

                        if(prog == AStarPathFindingProgress_Searching)
                        { 
                            while (curPeepIdx != OFFSET_NULL)
                            {
                                USE_POINTER Peep* curPeep = &gameState->peeps[curPeepIdx];

                                //tmp
                                curPeep->stateBasic.aStarSearchPtr = 0;

                                //tmp
                                //curPeep->physics.drive.target_x_Q16 = worldloc.x;
                                //curPeep->physics.drive.target_y_Q16 = worldloc.y;
                                //curPeep->physics.drive.target_z_Q16 = worldloc.z;
                                //curPeep->physics.drive.drivingToTarget = 1;



                                curPeepIdx = curPeep->prevSelectionPeepPtr[cliId];

                            }
                        }
                    }
                    else
                    {
                        printf("path finder busy..%d\n", gameState->mapSearchers[0].state);
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
                if (mapCoord.z > 0) 
                {
                    gameState->map.levels[mapCoord.z].data[mapCoord.x][mapCoord.y] = MapTile_NONE;
                    
                    MapBuildTileView3Area(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y);
                    MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y);
                    MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x + 1, mapCoord.y);
                    MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x - 1, mapCoord.y);
                    MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y + 1);
                    MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y - 1);
                }

            }

            //create
            if(BITGET(buttons, MouseButtonBits_PrimaryReleased) && (GUI_MOUSE_ON_GUI(gui) == 0) && (gui->draggedOff == 0) && client->curTool == EditorTools_Create)
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

                        Machine* machine;
                        OFFSET_TO_PTR(gameState->machines, machinePtr, machine);
                        CL_CHECK_NULL(machine);

                        machine->valid = true;
                        machine->mapTilePtr = VECTOR3_CAST(mapCoordSpawn, offsetPtrShort3);
                        machine->MachineDescPtr = client->curToolMachine;
                        MachineDesc* machDesc;
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
                        tileType = MapTile_CopperOre;
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
    cl_int tileData = 1;
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



void CLIENT_InitClientStates(ALL_CORE_PARAMS)
{
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        SynchronizedClientState* client = &gameState->clientStates[i];
        SyncedGui* gui = &client->gui;

        GuiState_Init(&gui->guiState);
    }
    GuiState_Init(&gameState->fakePassGui.guiState);
}

__kernel void game_init_single(ALL_CORE_PARAMS)
{
    printf("Game Initializing...\n");




    printf("Initializing StaticData Buffers..\n");
    MakeCardinalDirectionOffsets(&staticData->directionalOffsets[0]);

    printf("Initializing GUI..\n");
    GUI_INIT_STYLE(ALL_CORE_PARAMS_PASS);

    printf("Initializing Default Client States..\n");
    CLIENT_InitClientStates(ALL_CORE_PARAMS_PASS);



    printf("Startup Tests..\n");
    StartupTests();


    printf("Creating Machines Types\n");
    Machine_InitDescriptions(ALL_CORE_PARAMS_PASS);

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
        gameState->peeps[p].stateBasic.aStarSearchPtr = OFFSET_NULL;

        gameState->peeps[p].physics.base.v_Q16 = (ge_int3){ 0,0,0 };
        gameState->peeps[p].physics.base.vel_add_Q16 = (ge_int3){ 0,0,0 };
        gameState->peeps[p].physics.base.pos_post_Q16 = (ge_int3){ 0,0,0 };

        gameState->peeps[p].sectorPtr = OFFSET_NULL_2D;

        gameState->peeps[p].minDistPeepPtr = OFFSET_NULL;
        gameState->peeps[p].minDistPeep_Q16 = (1 << 30);
        gameState->peeps[p].physics.drive.target_x_Q16 = gameState->peeps[p].physics.base.pos_Q16.x;
        gameState->peeps[p].physics.drive.target_y_Q16 = gameState->peeps[p].physics.base.pos_Q16.y;
        gameState->peeps[p].physics.drive.drivingToTarget = 0;
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
    if (globalid < MAX_PEEPS) {
        USE_POINTER Peep* p = &gameState->peeps[globalid]; 
        PeepPreUpdate2(ALL_CORE_PARAMS_PASS, p);
    }

    //reset some things
    LINES_ClearAll(ALL_CORE_PARAMS_PASS);


    

    
}


__kernel void game_update(ALL_CORE_PARAMS)
{

    // Get the index of the current element to be processed
    int globalid = get_global_id(0);
    int localid = get_local_id(0);
    

    if (globalid < MAX_PEEPS) {
        USE_POINTER Peep* p = &gameState->peeps[globalid]; 
        PeepUpdate(ALL_CORE_PARAMS_PASS, p);
        PeepDraw(ALL_CORE_PARAMS_PASS, p);
    }

    if (globalid < MAX_PARTICLES) {
        USE_POINTER Particle* p = &gameState->particles[globalid];
        ParticleUpdate(ALL_CORE_PARAMS_PASS, p);
        ParticleDraw(ALL_CORE_PARAMS_PASS, p, globalid);
    }
    
    //update map view
    if (ThisClient(ALL_CORE_PARAMS_PASS)->mapZView != ThisClient(ALL_CORE_PARAMS_PASS)->mapZView_1)
    {
        cl_uint chunkSize = (MAPDIM * MAPDIM) / GAME_UPDATE_WORKITEMS;

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
    if((search->state == AStarPathFindingProgress_Failed) || (search->state == AStarPathFindingProgress_Finished))
    {
        cl_ulong chunks = (MAPDIM * MAPDIM * MAPDEPTH) / GAME_UPDATE_WORKITEMS;
    
        for (cl_ulong i = 0; i < chunks+1; i++)
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

                
                AStarSearch_BFS_InstantiateParrallel(search, xyzIdx, x, y, z);
            }
        }
    }



}


__kernel void game_post_update_single( ALL_CORE_PARAMS )
{



    ThisClient(ALL_CORE_PARAMS_PASS)->mapZView_1 = ThisClient(ALL_CORE_PARAMS_PASS)->mapZView;
    

    USE_POINTER AStarSearch_BFS* search = &gameState->mapSearchers[0];
    //update AStarPath Searchers
    if(search->state == AStarPathFindingProgress_Searching)
    {
        AStarSearch_BFS_Continue(ALL_CORE_PARAMS_PASS, search, 100);
    }
    else if(search->state != AStarPathFindingProgress_Ready)
    {
       // printf("going to ready..");
        search->state = AStarPathFindingProgress_Ready;
    }







    //draw debug lines on paths
    USE_POINTER AStarPathSteps* paths = &gameState->paths;
    
    //get a path
    int pathIdx = 0;

    offsetPtr pathStartOPtr = paths->pathStarts[pathIdx];
    while(pathStartOPtr != OFFSET_NULL)
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

         
            LINES_DrawLineWorld(ALL_CORE_PARAMS_PASS, worldCoordsFloat, worldCoordsNextFloat, (float3)(RandomRange(pathIdx, 0, 1000)/1000.0f, RandomRange(pathIdx+1, 0, 1000)/1000.0f, 1.0f));


            OFFSET_TO_PTR(paths->pathNodes, node->nextOPtr, node)
        }

        pathIdx++;
        if(pathIdx >= ASTAR_MAX_PATHS)
            pathIdx = 0;

        pathStartOPtr = paths->pathStarts[pathIdx];
    }

}

__kernel void game_preupdate_1(ALL_CORE_PARAMS) {


    int globalid = get_global_id(0);

    if (globalid >= GAME_UPDATE_WORKITEMS)
        return;
   

    cl_uint chunkSize = (MAX_PEEPS) / GAME_UPDATE_WORKITEMS;

    

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

            MapSector* sector;
            OFFSET_TO_PTR_2D(gameState->sectors, peep->sectorPtr, sector);
            sector->empty = false;
        }

    }

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

