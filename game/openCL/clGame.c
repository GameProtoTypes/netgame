

#include "clCommon.h"



#define RAYGUI_STANDALONE
#define RAYGUI_IMPLEMENTATION




#define PEEP_ALL_ALWAYS_VISIBLE
//#define PEEP_DISABLE_TILECORRECTIONS






SynchronizedClientState* ThisClient(ALL_CORE_PARAMS)
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
inline void MapDataSetTile(cl_uint* tileData, MapTile tile) {
    BITBANK_SET_SUBNUMBER_UINT(tileData, 0, 8, tile);
}

inline int MapTileGetRotation(cl_uint tileData) {
    return BITBANK_GET_SUBNUMBER_UINT(tileData, 8, 2);
}

cl_uint* MapGetDataPointerFromCoord(ALL_CORE_PARAMS, ge_int3 mapcoord)
{
    return &(gameState->map.levels[(mapcoord).z].data[(mapcoord).x][(mapcoord).y]);
}

MapTile MapGetTileFromCoord(ALL_CORE_PARAMS, ge_int3 mapcoord)
{
    return MapDataGetTile(gameState->map.levels[(mapcoord).z].data[(mapcoord).x][(mapcoord).y]);
}

cl_uchar MapRidgeType(ALL_CORE_PARAMS, ge_int3 mapCoords, ge_int3 enterDir)
{
    cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoords);

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

cl_uchar MapDataHas2LowAdjacentCorners(cl_uint* data)
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
    cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoords);

    return MapDataHas2LowAdjacentCorners(data);
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

cl_uchar MapHasLowCorner(ALL_CORE_PARAMS, ge_int3 mapCoords)
{
    cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoords);

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
    cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoords);

    if (MapDataGetTile(*data) == MapTile_NONE)
        return 0;

    return MapDataLowCornerCount(*data);
}

cl_uchar MapDataXLevel(cl_uint* data)
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
    cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoords);

    return MapDataXLevel(data);
}

cl_uchar MapTileCoordStandInValid(ALL_CORE_PARAMS, ge_int3 mapcoord)
{
    cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapcoord);
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
    cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapcoord);
    MapTile tile = MapDataGetTile(*data);
    if (tile == MapTile_NONE)
    {   

        if(enterDirection.z < 0)
            return 0;



        ge_int3 downCoord = mapcoord;
        downCoord.z--;
        cl_uint* downData = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, downCoord);
        if( MapDataGetTile(*downData) != MapTile_NONE)
            return 1;





    }
    else if(MapDataLowCornerCount(*data) == 0)//cant enter full blocks
    {
        return 0;
    }
    else if(enterDirection.z == 0)//entering partial block from the side
    {
        cl_uchar ridgeType = MapRidgeType(ALL_CORE_PARAMS_PASS, mapcoord, enterDirection);
        if (ridgeType == 0)
            return 1;
    }
    else if(enterDirection.z > 0)//entering partial block from below
    {
        ge_int3 dirNoZ = enterDirection;
        dirNoZ.z = 0;
        cl_uchar ridgeType = MapRidgeType(ALL_CORE_PARAMS_PASS, mapcoord, dirNoZ);
        if (ridgeType == 0)//running up continued ramp case.
            return 1;

    }
    else if(enterDirection.z < 0)//entering partial block from above
    {
        if(MapDataLowCornerCount(*data) > 0)//could be a ramp 
            return 1;
    }
    return 0;


}
void AStarNodeInstantiate(AStarNode* node)
{
    node->g_Q16 = TO_Q16(0);
    node->h_Q16 = TO_Q16(0);
    node->nextOPtr = OFFSET_NULL_3D;  
    node->prevOPtr = OFFSET_NULL_3D;
    node->tileIdx.x = -1;
    node->tileIdx.y = -1;
    node->tileIdx.z = -1;

}
void AStarInitPathNode(AStarPathNode* node)
{
    node->mapCoord_Q16 = (ge_int3){ 0,0,0 };
    node->nextOPtr = OFFSET_NULL;
    node->prevOPtr = OFFSET_NULL;
}
void AStarSearchInstantiate(AStarSearch* search)
{
    for (int x = 0; x < MAPDIM; x++)
    {
        for (int y = 0; y < MAPDIM; y++)
        {
            for (int z = 0; z < MAPDEPTH; z++)
            {
                AStarNode* node = &search->details[x][y][z];
                AStarNodeInstantiate(node);
                node->tileIdx = (ge_short3){x,y,z};

                search->closedMap[x][y][z] = 0;
                search->openMap[x][x][x] = 0;
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
}

cl_uchar MapTileCoordValid(ge_int3 mapcoord)
{
    if (mapcoord.x >= 0 && mapcoord.y >= 0 && mapcoord.z >= 0 && mapcoord.x < MAPDIM && mapcoord.y < MAPDIM && mapcoord.z < MAPDEPTH)
    {
        return 1;
    }
    return 0;
}
cl_uchar AStarNodeValid(AStarNode* node)
{
    return MapTileCoordValid(SHORT3_TO_INT3(node->tileIdx));
}
cl_uchar AStarNode2NodeTraversible(ALL_CORE_PARAMS, AStarNode* node, AStarNode* prevNode)
{  

    cl_uint* fromTileData = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, SHORT3_TO_INT3(prevNode->tileIdx));
    cl_uint* toTileData = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, SHORT3_TO_INT3(node->tileIdx));

    ge_int3 delta = INT3_SUB(SHORT3_TO_INT3(node->tileIdx), SHORT3_TO_INT3( prevNode->tileIdx ));
    if (MapTileCoordEnterable(ALL_CORE_PARAMS_PASS, SHORT3_TO_INT3(node->tileIdx), delta) == 0)
        return 0;

    if(delta.z > 0)
    {
        printf("UP.");

        if(MapDataGetTile(*fromTileData) == MapTile_NONE)
            return 0;
        //else could be a ramp

    
    }


    return 1;




}

void MakeCardinalDirectionOffsets(ge_int3* offsets)
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
int AStarOpenHeapKey(AStarSearch* search, AStarNode* node)
{
    //f
    return node->g_Q16 + node->h_Q16;
}

void AStarOpenHeapTrickleDown(AStarSearch* search, cl_int index)
{
    cl_int largerChild;
    AStarNode* top;
    offsetPtr3 topOPtr = search->openHeap_OPtrs[index];
    OFFSET_TO_PTR_3D(search->details, topOPtr, top);


    while (index < search->openHeapSize / 2)
    {
        int leftChild = 2 * index + 1;
        int rightChild = leftChild + 1;

        AStarNode* leftChildNode;
        OFFSET_TO_PTR_3D(search->details, search->openHeap_OPtrs[leftChild], leftChildNode)
        AStarNode* rightChildNode;
        OFFSET_TO_PTR_3D(search->details, search->openHeap_OPtrs[rightChild], rightChildNode)

        if ((rightChild < search->openHeapSize) && (AStarOpenHeapKey(search, leftChildNode) > AStarOpenHeapKey(search, rightChildNode)))
            largerChild = rightChild;
        else
            largerChild = leftChild;

        AStarNode* largerChildNode;

        OFFSET_TO_PTR_3D(search->details, search->openHeap_OPtrs[largerChild], largerChildNode)

        if (AStarOpenHeapKey(search, top) <= AStarOpenHeapKey(search, largerChildNode))
            break;

        search->openHeap_OPtrs[index] = search->openHeap_OPtrs[largerChild];
        index = largerChild;

        
    }
    
    search->openHeap_OPtrs[index] = topOPtr;
}

offsetPtr3 AStarOpenHeapRemove(AStarSearch* search)
{
    offsetPtr3 rootOPtr = search->openHeap_OPtrs[0];

    search->openHeap_OPtrs[0] = search->openHeap_OPtrs[search->openHeapSize-1];
    search->openHeapSize--;

 
    AStarOpenHeapTrickleDown(search, 0);

    return rootOPtr;
}

offsetPtr3 AStarRemoveFromOpen(AStarSearch* search)
{

    offsetPtr3 nodeOPtr = AStarOpenHeapRemove(search);
    

    AStarNode* node;
    OFFSET_TO_PTR_3D(search->details, nodeOPtr, node);

    search->openMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z] = 0;
    return nodeOPtr;
}
void AStarAddToClosed( AStarSearch* search, AStarNode* node)
{
    search->closedMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z] = 1;
}
cl_uchar AStarNodeInClosed(AStarSearch* search, AStarNode* node)
{
    return search->closedMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z];
}

cl_uchar AStarNodeInOpen(AStarSearch* search, AStarNode* node)
{
    return search->openMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z];
}


void AStarOpenHeapTrickleUp(AStarSearch* search, cl_int index)
{
    cl_int prev = (index - 1) / 2;
    offsetPtr3 bottomOPtr = search->openHeap_OPtrs[index];

    AStarNode* bottomNode;
    OFFSET_TO_PTR_3D(search->details, bottomOPtr, bottomNode);

    AStarNode* prevNode;
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


void AStarOpenHeapInsert(AStarSearch* search, offsetPtr3 nodeOPtr)
{
    search->openHeap_OPtrs[search->openHeapSize] = nodeOPtr;
    AStarOpenHeapTrickleUp(search, search->openHeapSize);
    search->openHeapSize++;
    if (search->openHeapSize > ASTARHEAPSIZE)
        printf("ERROR: AStarHeap Size Greater than ASTARHEAPSIZE!\n");


}
void AStarAddToOpen(AStarSearch* search, offsetPtr3 nodeOPtr)
{
    AStarOpenHeapInsert(search, nodeOPtr);

    AStarNode* node;
    OFFSET_TO_PTR_3D(search->details, nodeOPtr, node);
    search->openMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z] = 1;
}







void AStarRemoveFromClosed(AStarSearch* search, AStarNode* node)
{
    search->closedMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z] = 0;
}

cl_int AStarNodeDistanceHuristic(AStarSearch* search, AStarNode* nodeA, AStarNode* nodeB)
{
    return TO_Q16(abs(nodeA->tileIdx.x - nodeB->tileIdx.x) + abs(nodeA->tileIdx.y - nodeB->tileIdx.y) + abs(nodeA->tileIdx.z - nodeB->tileIdx.z));
}

void AStarPrintNodeStats(AStarNode* node)
{
    printf("Node: Loc: ");
    Print_GE_SHORT3(node->tileIdx);
    printf(" H: %f, G: %f\n", FIXED2FLTQ16(node->h_Q16), FIXED2FLTQ16(node->g_Q16));
}
void AStarPrintPathNodeStats(AStarPathNode* node)
{
    printf("Node: Loc: ");
    Print_GE_INT3_Q16(node->mapCoord_Q16);
}
void AStarPrintSearchPathTo(AStarSearch* search, ge_int3 destTile)
{
    AStarNode* curNode = &search->details[destTile.x][destTile.y][destTile.z];
    while (curNode != NULL)
    {
        AStarPrintNodeStats(curNode);
        OFFSET_TO_PTR_3D(search->details, curNode->prevOPtr, curNode);
    }
}
void AStarPrintSearchPathFrom(AStarSearch* search, ge_int3 startTile)
{
    AStarNode* curNode = &search->details[startTile.x][startTile.y][startTile.z];
    while (curNode != NULL)
    {
        AStarPrintNodeStats(curNode);
        OFFSET_TO_PTR_3D(search->details, curNode->nextOPtr, curNode);
    }
}

void AStarPrintPath(AStarPathSteps* paths, offsetPtr startNodeOPtr)
{
    AStarPathNode* curNode;
    OFFSET_TO_PTR(paths->pathNodes, startNodeOPtr, curNode);
    while (curNode != NULL)
    {
        AStarPrintPathNodeStats(curNode);
        OFFSET_TO_PTR(paths->pathNodes, curNode->nextOPtr, curNode);
    }
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


cl_uchar AStarSearchRoutine(ALL_CORE_PARAMS, AStarSearch* search, ge_int3 startTile, ge_int3 destTile, int maxIterations)
{
    if (MapTileCoordValid(startTile) == 0)
    {
        printf("AStarSearchRoutine: Start MapCoord Not Valid.\n");
        return 0;
    }
    if (MapTileCoordValid(destTile) == 0)
    {
        printf("AStarSearchRoutine: Dest MapCoord Not Valid.\n");
        return 0;
    }
    if (MapTileCoordStandInValid(ALL_CORE_PARAMS_PASS, startTile) == 0)
    {
        printf("AStarSearchRoutine: Start Stand In Invalid.\n");
        return 0;
    }
    if (MapTileCoordStandInValid(ALL_CORE_PARAMS_PASS, destTile)==0)
    {
        printf("AStarSearchRoutine: Dest Stand In Invalid.\n");
        return 0;
    }

    printf("starting search\n");

    AStarNode* startNode = &search->details[startTile.x][startTile.y][startTile.z];
    AStarNode* targetNode = &search->details[destTile.x][destTile.y][destTile.z];

    offsetPtr3 targetNodeOPtr = (offsetPtr3){destTile.x,destTile.y,destTile.z};

    search->startNodeOPtr = (offsetPtr3){startTile.x,startTile.y,startTile.z};
    
    //add start to openList
    startNode->h_Q16 = AStarNodeDistanceHuristic(search, startNode, targetNode);
    AStarAddToOpen(search, search->startNodeOPtr );


    cl_uchar foundDest = 0;
    int iterationCount = maxIterations;
    while (search->openHeapSize > 0 && iterationCount > 0)
    {
        //find node in open with lowest f cost
        offsetPtr3 currentOPtr = AStarRemoveFromOpen(search);


        AStarNode* current;
        OFFSET_TO_PTR_3D(search->details, currentOPtr, current);



        if (VECTOR3_EQUAL(SHORT3_TO_INT3( current->tileIdx ), destTile) )
        {
            printf("Goal Found\n");
            search->endNodeOPtr = targetNodeOPtr;

            AStarNode* endNode;
            OFFSET_TO_PTR_3D(search->details, search->endNodeOPtr, endNode);
            endNode->nextOPtr = OFFSET_NULL_3D;
            startNode->prevOPtr = OFFSET_NULL_3D;

            //form next links
            AStarNode* curNode = targetNode;
            offsetPtr3 curNodeOPtr = targetNodeOPtr;

            while (curNode != NULL)
            {
                AStarNode* p;
                OFFSET_TO_PTR_3D(search->details, curNode->prevOPtr, p);

                if(p != NULL)
                    p->nextOPtr = curNodeOPtr;

                curNodeOPtr = curNode->prevOPtr;
                curNode = p;
            }


            return 1;//found dest
        }
        
        //AStarAddToClosed(search, current);



        //5 neighbors
        for (int i = 0; i <= 25; i++)
        { 
            ge_int3 prospectiveTileCoord;
            ge_int3 dir = staticData->directionalOffsets[i];
            prospectiveTileCoord.x = current->tileIdx.x + dir.x;
            prospectiveTileCoord.y = current->tileIdx.y + dir.y;
            prospectiveTileCoord.z = current->tileIdx.z + dir.z;
 
            if (MapTileCoordValid(prospectiveTileCoord)==0)
            {
                continue;
            }


            
            //if lateral dyagonol, check adjacents a for traversability as well. if all traverible - diagonoal is traversible.
            if(GE_INT3_SINGLE_ENTRY(dir) == 0 && dir.z == 0)
            {


                ge_int3 dirNoX = dir;
                dirNoX.x=0;
                ge_int3 dirNoY = dir;
                dirNoX.y=0;


                ge_int3 XCheckCoord;
                ge_int3 YCheckCoord;


                offsetPtr3 XCheckNodeOPtr = (offsetPtr3){XCheckCoord.x,XCheckCoord.y,XCheckCoord.z};
                AStarNode* XCheckNode;
                OFFSET_TO_PTR_3D(search->details, XCheckNodeOPtr, XCheckNode);
                if ((AStarNode2NodeTraversible(ALL_CORE_PARAMS_PASS,  XCheckNode, current) == 0))
                {
                    continue;
                }

                offsetPtr3 YCheckNodeOPtr = (offsetPtr3){YCheckCoord.x,YCheckCoord.y,YCheckCoord.z};
                AStarNode* YCheckNode;
                OFFSET_TO_PTR_3D(search->details, YCheckNodeOPtr, YCheckNode);
                if ((AStarNode2NodeTraversible(ALL_CORE_PARAMS_PASS,  YCheckNode, current) == 0))
                {
                    continue;
                }
            }






            offsetPtr3 prospectiveNodeOPtr = (offsetPtr3){prospectiveTileCoord.x,prospectiveTileCoord.y,prospectiveTileCoord.z};
            AStarNode* prospectiveNode;
            OFFSET_TO_PTR_3D(search->details, prospectiveNodeOPtr, prospectiveNode);

            if ((AStarNode2NodeTraversible(ALL_CORE_PARAMS_PASS,  prospectiveNode, current) == 0))
            {
                continue;
            }




            int totalMoveCost = current->g_Q16 + AStarNodeDistanceHuristic(search, current, prospectiveNode);

            if ((totalMoveCost < prospectiveNode->g_Q16) || prospectiveNode->g_Q16 == 0)
            {
                prospectiveNode->g_Q16 = totalMoveCost;
                prospectiveNode->h_Q16 = AStarNodeDistanceHuristic(search, prospectiveNode, targetNode);
                
                prospectiveNode->prevOPtr = currentOPtr;

               // printf("G: "); PrintQ16(prospectiveNode->g_Q16); printf("H: ");  PrintQ16(prospectiveNode->h_Q16);
                AStarAddToOpen(search, prospectiveNodeOPtr);
            }
        }

        iterationCount--;
    }

    
    printf("FAIL %d\n", search->openHeapSize);
    return foundDest;
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



int AStarPathStepsNextFreePathNode(AStarPathSteps* list)
{
    int ptr = list->nextListIdx;
    while (list->pathNodes[ptr].nextOPtr != OFFSET_NULL)
    {
        ptr++;
        if (ptr >= ASTARPATHSTEPSSIZE)
            ptr = 0;
    }
    list->nextListIdx = ptr+1;
    return ptr;
}

//get the last path node from a node in a path
offsetPtr AStarPathNode_LastPathNode(AStarPathSteps* steps, offsetPtr pathNodeOPtr)
{
    offsetPtr curNodeOPtr = pathNodeOPtr;
    AStarPathNode* curNode;
    OFFSET_TO_PTR(steps->pathNodes, curNodeOPtr, curNode);


    while(curNode->nextOPtr != OFFSET_NULL)
    {
        OFFSET_TO_PTR(steps->pathNodes, curNode->nextOPtr, curNode);
        curNodeOPtr = curNode->nextOPtr;
    }
    return curNodeOPtr;
}

offsetPtr AStarFormPathSteps(ALL_CORE_PARAMS, AStarSearch* search, AStarPathSteps* steps)
{
    //grab a unused Node from pathNodes, and start building the list .
    offsetPtr3 curNodeOPtr =  search->startNodeOPtr;
    AStarNode* curNode;
    OFFSET_TO_PTR_3D(search->details, curNodeOPtr, curNode);

    offsetPtr startNodeOPtr = OFFSET_NULL;
    AStarPathNode* pNP = NULL;
    int i = 0;
    while (curNode != NULL)
    { 

        int index = AStarPathStepsNextFreePathNode(&gameState->paths);

        AStarPathNode* pN = &gameState->paths.pathNodes[index];

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
            //iterate until joint in path.
            ge_int3 delta;
            AStarNode* n2 = curNode;
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
                AStarNode* n2Prev;
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
        else
            curNode = NULL;


        i++;

    }
    pNP->nextOPtr = OFFSET_NULL;



    //form prev links
    AStarPathNode* curNode2;
    offsetPtr curNode2OPtr = startNodeOPtr;
    OFFSET_TO_PTR(steps->pathNodes, startNodeOPtr, curNode2);

    while (curNode2 != NULL)
    {

        AStarPathNode* p;
        OFFSET_TO_PTR(steps->pathNodes, curNode2->nextOPtr, p);


        if (p != NULL)
            p->prevOPtr = curNode2OPtr;


        curNode2 = p;
        if(curNode2 != NULL)
            curNode2OPtr = curNode2->nextOPtr;
        
    }


    return startNodeOPtr;
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

ge_int3 Triangle3D_ToBaryCentric(Triangle3DHeavy* triangle, ge_int3 point)
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

ge_int3 Triangle3DHeavy_ClosestPoint(Triangle3DHeavy* triangle, ge_int3 point_Q16, int* dist_Q16)
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



void Triangle3DMakeHeavy(Triangle3DHeavy* triangle)
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
void Triangle3D_Make2Face(Triangle3DHeavy* triangle1, Triangle3DHeavy* triangle2, ge_int3* fourCorners)
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



void MapTileConvexHull_From_TileData(ConvexHull* hull, cl_int* tileData)
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


void PeepPeepPhysics(ALL_CORE_PARAMS, Peep* peep, Peep* otherPeep)
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

void PeepToPeepInteraction(ALL_CORE_PARAMS, Peep* peep, Peep* otherPeep)
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


void PeepGetMapTile(ALL_CORE_PARAMS, Peep* peep, ge_int3 offset, MapTile* out_map_tile, ge_int3* out_tile_world_pos_center_Q16, ge_int3* out_map_tile_coord_whole, cl_int* out_tile_data)
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

void RegionCollision(cl_int* out_pen_Q16, cl_int radius_Q16, cl_int W, cl_int lr)
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




ge_int3 MapTileConvexHull_ClosestPointToPoint(ConvexHull* hull, ge_int3 point_Q16)
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

cl_uchar MapTileConvexHull_PointInside(ConvexHull* hull, ge_int3 point)
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

void PeepMapTileCollisions(ALL_CORE_PARAMS, Peep* peep)
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
    peep->physics.base.v_Q16.z += (TO_Q16(-1) >> 3);
}


void PeepDrivePhysics(ALL_CORE_PARAMS, Peep* peep)
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
        if (peep->physics.drive.drivingToTarget)
        {

            if (peep->physics.drive.nextPathNodeOPtr == OFFSET_NULL)
            {
                //final node reached.
                peep->comms.message_TargetReached_pending = 255;//send the message
            }
            else
            {

                AStarPathNode* nxtpathNode;
                OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.nextPathNodeOPtr,nxtpathNode);

            
                peep->physics.drive.nextPathNodeOPtr = nxtpathNode->nextOPtr;
                if (peep->physics.drive.nextPathNodeOPtr != OFFSET_NULL) 
                {
                    AStarPathNode* nxtpathNode;
                    OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.nextPathNodeOPtr,nxtpathNode);


                    ge_int3 nextTarget_Q16 = nxtpathNode->mapCoord_Q16;
                    MapToWorld(nextTarget_Q16, &nextTarget_Q16);

                    peep->physics.drive.target_x_Q16 = nextTarget_Q16.x;
                    peep->physics.drive.target_y_Q16 = nextTarget_Q16.y;
                    peep->physics.drive.target_z_Q16 = nextTarget_Q16.z;
                }
            }
        }
    }
   
    if (peep->physics.drive.drivingToTarget)
    {
       // Print_GE_INT3_Q16(peep->physics.base.pos_Q16);

        targetVelocity.x = d.x >> 2;
        targetVelocity.y = d.y >> 2;
        targetVelocity.z = d.z >> 2;

        ge_int3 error = INT3_SUB( targetVelocity, peep->physics.base.v_Q16 );

        peep->physics.base.vel_add_Q16.x += error.x;
        peep->physics.base.vel_add_Q16.y += error.y;
        peep->physics.base.vel_add_Q16.z += error.z;


        peep->physics.base.CS_angle_rad = atan2(((float)(d.x))/(1<<16), ((float)(d.y)) / (1 << 16));
    }



}

void WalkAndFight(ALL_CORE_PARAMS, Peep* peep)
{


    PeepDrivePhysics(ALL_CORE_PARAMS_PASS, peep);



    PeepMapTileCollisions(ALL_CORE_PARAMS_PASS, peep);




}


void PeepAssignToSector_Detach(ALL_CORE_PARAMS, Peep* peep)
{

    cl_int x = ((peep->physics.base.pos_Q16.x >> 16) / (SECTOR_SIZE));
    cl_int y = ((peep->physics.base.pos_Q16.y >> 16) / (SECTOR_SIZE));

    //clamp
    if (x < -SQRT_MAXSECTORS / 2)
        x = -SQRT_MAXSECTORS / 2;
    if (y < -SQRT_MAXSECTORS / 2)
        y = -SQRT_MAXSECTORS / 2;
    if (x >= SQRT_MAXSECTORS / 2)
        x = SQRT_MAXSECTORS / 2 - 1;
    if (y >= SQRT_MAXSECTORS / 2)
        y = SQRT_MAXSECTORS / 2 - 1;


    MapSector* newSector = &(gameState->sectors[x + SQRT_MAXSECTORS / 2][y + SQRT_MAXSECTORS / 2]);
    CL_CHECK_NULL(newSector)

    MapSector* curSector;
    OFFSET_TO_PTR_2D(gameState->sectors, peep->mapSectorPtr, curSector);

    if ((curSector != newSector))
    {

        if (curSector != NULL)
        {

            //remove peep from old sector
            if ((peep->prevSectorPeepPtr != OFFSET_NULL))
            {
                gameState->peeps[peep->prevSectorPeepPtr].nextSectorPeepPtr = peep->nextSectorPeepPtr;
            }
            if ((peep->nextSectorPeepPtr != OFFSET_NULL))
            {
                gameState->peeps[peep->nextSectorPeepPtr].prevSectorPeepPtr = peep->prevSectorPeepPtr;
            }


            if (curSector->lastPeepPtr == peep->ptr) {

                if (peep->prevSectorPeepPtr != OFFSET_NULL)
                    curSector->lastPeepPtr = gameState->peeps[peep->prevSectorPeepPtr].ptr;
                else
                    curSector->lastPeepPtr = OFFSET_NULL;
                
                if (curSector->lastPeepPtr != OFFSET_NULL)
                    gameState->peeps[curSector->lastPeepPtr].nextSectorPeepPtr = OFFSET_NULL;
            }

            //completely detach
            peep->nextSectorPeepPtr = OFFSET_NULL;
            peep->prevSectorPeepPtr = OFFSET_NULL;

        }

        //assign new sector for next stage
        peep->mapSector_pendingPtr = newSector->ptr;
    }

}
void PeepAssignToSector_Insert(ALL_CORE_PARAMS, Peep* peep)
{
    //assign new sector
    if (!CL_VECTOR2_EQUAL(peep->mapSectorPtr, peep->mapSector_pendingPtr))
    {
        peep->mapSectorPtr = peep->mapSector_pendingPtr;

        MapSector* mapSector;
        OFFSET_TO_PTR_2D(gameState->sectors, peep->mapSectorPtr, mapSector)


        //put peep in the sector.  extend list
        if (mapSector->lastPeepPtr != OFFSET_NULL)
        {
            gameState->peeps[mapSector->lastPeepPtr].nextSectorPeepPtr = peep->ptr;
            peep->prevSectorPeepPtr = gameState->peeps[mapSector->lastPeepPtr].ptr;
        }
        mapSector->lastPeepPtr = peep->ptr;
    }
}
void PeepPreUpdate1(ALL_CORE_PARAMS, Peep* peep)
{

}

void PeepPreUpdate2(Peep* peep)
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

    peep->physics.base.pos_post_Q16.z = 0;
    peep->physics.base.pos_post_Q16.y = 0;
    peep->physics.base.pos_post_Q16.x = 0;

    peep->physics.base.v_Q16.x = 0;
    peep->physics.base.v_Q16.y = 0;
    peep->physics.base.v_Q16.z = 0;

    if (peep->stateBasic.health <= 0)
        peep->stateBasic.deathState = 1;


    //peep comms
    if (peep->comms.message_TargetReached_pending)
    {
        peep->physics.drive.drivingToTarget = 0;
        peep->comms.message_TargetReached = peep->comms.message_TargetReached_pending;
        peep->comms.message_TargetReached--;//message fade
    }


}

int PeepMapVisiblity(ALL_CORE_PARAMS, Peep* peep, int mapZViewLevel)
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
    MapTile ctile;
    cl_int tileData;
    offset.x = 0;
    offset.y = 0;
    offset.z = 0;
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, offset, &ctile, &tilePWorldCen, &tileMapCoordWhole, &tileData);


    while (ctile == MapTile_NONE && tileMapCoordWhole.z < MAPDEPTH)
    {
        tileMapCoordWhole.z++;

        ctile = gameState->map.levels[tileMapCoordWhole.z].data[tileMapCoordWhole.x][tileMapCoordWhole.y];
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


void PeepUpdate(ALL_CORE_PARAMS, Peep* peep)
{

    peep->minDistPeep_Q16 = (1 << 30);
    peep->minDistPeepPtr = OFFSET_NULL;

    MapSector* cursector;
    OFFSET_TO_PTR_2D(gameState->sectors, peep->mapSectorPtr, cursector);

    //traverse sector
    int minx = cursector->ptr.x - 1; if (minx == 0xFFFFFFFF) minx = 0;
    int miny = cursector->ptr.y - 1; if (miny == 0xFFFFFFFF) miny = 0;

    int maxx = cursector->ptr.x + 1; if (maxx >= SQRT_MAXSECTORS) maxx = SQRT_MAXSECTORS-1;
    int maxy = cursector->ptr.y + 1; if (maxy >= SQRT_MAXSECTORS) maxy = SQRT_MAXSECTORS-1;
    
    for(cl_int sectorx = minx; sectorx <= maxx; sectorx++)
    {
        for (cl_int sectory = miny; sectory <= maxy; sectory++)
        {

            MapSector* sector = &gameState->sectors[sectorx][sectory];
            CL_CHECK_NULL(sector);

            Peep* curPeep;
            OFFSET_TO_PTR(gameState->peeps, sector->lastPeepPtr, curPeep);
            

            Peep* firstPeep = curPeep;
            while (curPeep != NULL)
            {
                if (curPeep != peep) {
                    PeepToPeepInteraction(ALL_CORE_PARAMS_PASS, peep, curPeep);

                }

                
                OFFSET_TO_PTR(gameState->peeps, curPeep->prevSectorPeepPtr, curPeep);


                if (curPeep == firstPeep)
                    break;
            }

        }
    }
   

    WorldToMap(peep->physics.base.pos_Q16, &peep->posMap_Q16);

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

void ParticleUpdate(ALL_CORE_PARAMS, Particle* p)
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

    for (int z = ThisClient(ALL_CORE_PARAMS_PASS)->mapZView; z >= 1; z--)
    {       
        cl_uint* data = &gameState->map.levels[z].data[x][y];
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
    }


}









void MapBuildTileView(ALL_CORE_PARAMS, int x, int y)
{
    ge_int3 coord = (ge_int3){x,y,ThisClient(ALL_CORE_PARAMS_PASS)->mapZView };
    cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, coord);
    MapTile tile = MapDataGetTile(*data);
    MapTile tileUp;
    if (ThisClient(ALL_CORE_PARAMS_PASS)->mapZView < MAPDEPTH-1)
    {
        coord.z = ThisClient(ALL_CORE_PARAMS_PASS)->mapZView + 1;
        cl_uint* dataup = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, coord);


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
            vz++;
        }
    }

    //quick hack for the top level transitions
    int m = 1;
    if (vz == 0)
        m = 2;

    cl_uint finalAttr=0;

    finalAttr |= BITGET_MF(*data, MapTileFlags_LowCornerTPLEFT)       ;//A
    finalAttr |= BITGET_MF(*data, MapTileFlags_LowCornerTPRIGHT)  << 1;//B
    finalAttr |= BITGET_MF(*data, MapTileFlags_LowCornerBTMLEFT)  << 2;//C
    finalAttr |= BITGET_MF(*data, MapTileFlags_LowCornerBTMRIGHT) << 3;//D


    uint xlev = MapDataXLevel(data);
    finalAttr |= (2-xlev) << 4;//X


    finalAttr |= (clamp(15-vz, 0, 15) << 6);


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

            if(MapTileCoordValid(mapCoord))
            {
            
                cl_uint* dataoffup = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoord);
                MapTile tileoffup = MapDataGetTile(*dataoffup);
                
                if(tileoffup == MapTile_NONE)
                {
                    isWall = 1;
                    mapTile1VBO[y * MAPDIM + x ] = tile;


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


                    mapTile1AttrVBO[y * MAPDIM + x ] |= (clamp(4, 0, 15) << 6);//base shade
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


void PrintSelectionPeepStats(ALL_CORE_PARAMS, Peep* p)
{
///    Print_GE_INT3_Q16(p->physics.base.pos_Q16);
    Peep* peep = p;
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

void MapTileCoordClamp(ge_int3* mapCoord)
{
    (*mapCoord).x = clamp((*mapCoord).x, 0, MAPDIM - 1);
    (*mapCoord).y = clamp((*mapCoord).y, 0, MAPDIM - 1);
    (*mapCoord).z = clamp((*mapCoord).z, 0, MAPDEPTH - 1);
}


void GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS, ge_int2 world_Q16, ge_int3* mapcoord_whole, int* occluded, int zViewRestrictLevel)
{
    ge_int3 wrld_Q16;
    wrld_Q16.x = world_Q16.x;
    wrld_Q16.y = world_Q16.y;
    WorldToMap(wrld_Q16, &(*mapcoord_whole));
    (*mapcoord_whole).x = WHOLE_Q16((*mapcoord_whole).x);
    (*mapcoord_whole).y = WHOLE_Q16((*mapcoord_whole).y);


    MapTileCoordClamp(mapcoord_whole);

    for (int z = zViewRestrictLevel; z >= 0; z--)
    {
        cl_uint* data = &gameState->map.levels[z].data[(*mapcoord_whole).x][(*mapcoord_whole).y];
        MapTile tile = MapDataGetTile(*data);

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








void GUI_Text(ALL_CORE_PARAMS, const cl_char* text)
{
    

}


SyncedGui* GetGuiState(ALL_CORE_PARAMS, int clientId)
{
    return &gameState->clientStates[clientId].gui;
}
int GrabGuiId(SyncedGui* gui)
{
    int id =  gui->nextId;;
    gui->nextId++;
    return id;
}

void GUI_DrawRectangle(ALL_CORE_PARAMS, SyncedGui* gui, int x, int y, int width, int height, float3 color, float2 UVStart, float2 UVEnd)
{
    if(gui->passType == GuiStatePassType_Synced)
    {
        return;
    }


    uint idx = gui->guiRenderRectIdx;

    float2 UVSize = UVEnd - UVStart;

    //clip
    ge_int2 clipEnd;
    clipEnd.x = gui->clip.x + gui->clip.z;
    clipEnd.y = gui->clip.y + gui->clip.w;

    if(x < gui->clip.x)
    {
        width = width - (gui->clip.x - x);

        x = gui->clip.x;
    }
    
    if(y < gui->clip.y)
    {
        height = height - (gui->clip.y - y);
        y = gui->clip.y;
    }
    
    if(x + width > clipEnd.x)
    {

        width = clipEnd.x - x;
    }
    if(y + height > clipEnd.y)
    {

        height = clipEnd.y - y;
    }

    //early out if not rendered
    if(width <= 0)
        return;
    if(height <= 0)
        return;


    //map to [0,1]
    float xf = x/GUI_PXPERSCREEN_F;
    float yf = y/GUI_PXPERSCREEN_F;
    float widthf = width/GUI_PXPERSCREEN_F;
    float heightf = height/GUI_PXPERSCREEN_F;


    //map [0,1] to [-1,1]
    xf*= 2.0f;
    yf*= -2.0f;
    xf+=-1.0f;
    yf+=1.0f;

    widthf *= 2.0f;
    heightf*= -2.0f;



    const int stride = 7;

    guiVBO[idx*stride + 0] = xf+widthf;
    guiVBO[idx*stride + 1] = yf+heightf;

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] = xf;
    guiVBO[idx*stride + 1] = yf;

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    guiVBO[idx*stride + 0] = xf;
    guiVBO[idx*stride + 1] = yf+heightf;

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] = xf+widthf;
    guiVBO[idx*stride + 1] = yf+heightf;

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] = xf+widthf;
    guiVBO[idx*stride + 1] = yf;

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    guiVBO[idx*stride + 0] = xf;
    guiVBO[idx*stride + 1] = yf;

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    gui->guiRenderRectIdx = idx;
}

cl_uchar GUI_BoundsCheck(ge_int2 boundStart, ge_int2 boundEnd, ge_int2 pos)
{
    if((pos.x >= boundStart.x) && (pos.x < boundEnd.x) && (pos.y >= boundStart.y) && (pos.y < boundEnd.y) )
    {
        return 1;
    }
    return 0;
}


#define GUIID ALL_CORE_PARAMS_PASS, gui, GrabGuiId(gui)
#define GUIID_DEF ALL_CORE_PARAMS, SyncedGui* gui, int id, ge_int2 pos, ge_int2 size
#define GUI_FAKESWITCH_PARAM_INT(PARAM) GuiFakeSwitch_Param_Int(gui, PARAM)

int* GetGuiFakeInt(SyncedGui* gui)
{
    int* param = &gui->fakeInts[gui->nextFakeIntIdx];
    gui->nextFakeIntIdx++;

    if(gui->nextFakeIntIdx >= SYNCGUI_MAX_WIDGETS)
        printf("GUI_OUT_OF_WIDGETS\n");
    
    return param;
}

int* GuiFakeSwitch_Param_Int(SyncedGui* gui, int* param)
{
    if(gui->passType == GuiStatePassType_Synced)
    {
        return param;
    }
    else
    {
        return GetGuiFakeInt(gui);
    }
}



void GUI_PushOffset(SyncedGui* gui, ge_int2 offset)
{
    gui->wOSidx++;
    if(gui->wOSidx >= SYNCGUI_MAX_DEPTH)
        printf("ERROR: GUI_PushOffset Overflow (SYNCGUI_MAX_DEPTH)");

    gui->widgetOffsetStack[gui->wOSidx] = offset;
}

#define GUI_GETOFFSET() GUI_GetOffset(gui)
ge_int2 GUI_GetOffset(SyncedGui* gui)
{
    ge_int2 sum = (ge_int2){0,0};
    for(int i = 0; i <= gui->wOSidx; i++)
    {
        sum = INT2_ADD(sum, gui->widgetOffsetStack[gui->wOSidx]);
    }
    return sum;
}

void GUI_PopOffset(SyncedGui* gui)
{
    gui->wOSidx--;
    if(gui->wOSidx < -1)
        printf("ERROR: GUI PopOffset Call Missmatch.");
}

void GUI_SetClip(SyncedGui* gui, ge_int2 startPos, ge_int2 size)
{
    startPos = INT2_ADD(startPos, GUI_GETOFFSET());

    gui->clip.x = startPos.x;
    gui->clip.y = startPos.y;
    gui->clip.z = size.x;
    gui->clip.w = size.y;
}

void GUI_ReleaseClip(SyncedGui* gui)
{
    gui->clip = (ge_int4){0,0,GUI_PXPERSCREEN,GUI_PXPERSCREEN};
}






cl_uchar GUI_BUTTON(GUIID_DEF, int* down)
{
    pos = INT2_ADD(pos, GUI_GETOFFSET());


    cl_uchar ret = 0;
    *down = 0;
    if((GUI_BoundsCheck(pos, INT2_ADD(pos, size), gui->mouseLoc) && (gui->ignoreAll == 0)) || (gui->dragOff && ( gui->lastActiveWidget == id)))
    {
        gui->hoverWidget = id;

        if(BITGET(gui->mouseState, MouseButtonBits_PrimaryDown))
        {
            gui->activeWidget = id;
            *down = 1;
        }
        
        else if(BITGET(gui->mouseState, MouseButtonBits_PrimaryReleased))
        {
            ret = 1;
        }
        gui->mouseOnGUI = 1;
    }

    float3 color = (float3){0.0,0.0,1.0};

    if(gui->hoverWidget == id)
        color = (float3){1.0,1.0,1.0};

    if(gui->activeWidget == id){
        color = (float3){1.0,0.0,0.0};
    }


    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, size.y, color, (float2){0.0,0.0}, (float2){0.0,0.0} );
    
    return ret;
}

cl_uchar GUI_SLIDER_INT(GUIID_DEF, int* value, int min, int max)
{
    pos = INT2_ADD(pos, GUI_GETOFFSET());

    ge_int2 posHandle;
    ge_int2 sizeHandle;

    sizeHandle.y = size.y;
    sizeHandle.x = size.x/10;

    if(GUI_BoundsCheck(pos, INT2_ADD(pos, size), gui->mouseLoc)&&(gui->ignoreAll == 0) || (gui->dragOff && ( gui->lastActiveWidget == id+1)))
    {
        if(BITGET(gui->mouseState, MouseButtonBits_PrimaryDown) || BITGET(gui->mouseState, MouseButtonBits_PrimaryReleased) )
        {
            float perc = ((float)(gui->mouseLoc.x - pos.x))/size.x;
            int d = perc*(max-min);
            (*value) = clamp(min + d, min, max-1);
        }
        gui->mouseOnGUI = 1;
    }


    posHandle.y = pos.y;
    posHandle.x = pos.x + (size.x*((*value)-1)/(max-min));

    posHandle.x = clamp(posHandle.x, pos.x, pos.x + size.x - sizeHandle.x);


    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, size.y, (float3){0.5,0.5,0.5}, (float2){0.0,0.0}, (float2){0.0,0.0} );
    

    int down;
    GUI_BUTTON(GUIID, posHandle, sizeHandle, &down);


}




void GUI_Begin_ScrollArea(GUIID_DEF, ge_int2 scroll_offset)
{
    GUI_PushOffset(gui, scroll_offset);
}



void  GUI_End_ScrollArea(SyncedGui* gui)
{
    GUI_PopOffset(gui);
}




cl_uchar GUI_MOUSE_ON_GUI(SyncedGui* gui)
{
    return gui->mouseOnGUI;
}

//call before gui path
void GUI_RESET(ALL_CORE_PARAMS, SyncedGui* gui, ge_int2 mouseLoc, int mouseState, GuiStatePassType passType)
{
    gui->passType = passType;
    if(passType == GuiStatePassType_NoLogic)
    {
        //clear just drawn rectangles
        const int stride = 7;
        for(int idx = (gui->guiRenderRectIdx-1)*stride; idx >=0; idx--)
        {
            guiVBO[idx] = 0;
        }
    }
    else
    {

    }
    gui->guiRenderRectIdx = 0;
    gui->nextId = 0;
    gui->nextFakeIntIdx = 0;

    gui->mouseFrameDelta = INT2_SUB(mouseLoc, gui->mouseLoc);

    gui->mouseLoc = mouseLoc;
    gui->mouseState = mouseState;


    gui->lastActiveWidget = gui->activeWidget;
    gui->hoverWidget = -1;
    gui->activeWidget = -1;

    gui->draggedOff = 0;
    gui->wOSidx = -1;
   
    GUI_ReleaseClip( gui);

    if(BITGET(mouseState, MouseButtonBits_PrimaryPressed) || BITGET(mouseState, MouseButtonBits_SecondaryPressed))
    {
        gui->mouseLocBegin = mouseLoc;
        gui->mouseDragging = 1;

    }
    else if(BITGET(mouseState, MouseButtonBits_PrimaryReleased) || BITGET(mouseState, MouseButtonBits_SecondaryReleased))
    {
        gui->ignoreAll = 0;  
        gui->mouseDragging = 0;  
    }

}

//call after gui path
void GUI_RESET_POST(ALL_CORE_PARAMS, SyncedGui* gui)
{

    if(BITGET(gui->mouseState, MouseButtonBits_PrimaryPressed) || BITGET(gui->mouseState, MouseButtonBits_SecondaryPressed))
    {
        if(gui->mouseOnGUI == 0)
        {
            gui->ignoreAll = 1;
        }
        else
        {
            gui->dragOff = 1;
        }
    }
    else if(BITGET(gui->mouseState, MouseButtonBits_PrimaryReleased) || BITGET(gui->mouseState, MouseButtonBits_SecondaryReleased))
    {
        if(gui->dragOff)
        {
            gui->dragOff = 0;
            gui->draggedOff = 1;
        }
    }


    gui->mouseOnGUI = 0;
}



__kernel void game_apply_actions(ALL_CORE_PARAMS)
{


    cl_uint curPeepIdx = gameState->clientStates[gameStateActions->clientId].selectedPeepsLastIdx;
    PeepRenderSupport peepRenderSupport[MAX_PEEPS];
    while (curPeepIdx != OFFSET_NULL)
    {
        Peep* p = &gameState->peeps[curPeepIdx];
        gameState->clientStates[gameStateActions->clientId].peepRenderSupport[curPeepIdx].render_selectedByClient = 1;

        curPeepIdx = p->prevSelectionPeepPtr[gameStateActions->clientId];
    }



    //apply turns
    for (int32_t a = 0; a < gameStateActions->numActions+1; a++)
    {
        int b = a;
        cl_uchar fakePass = 0;
        if(a == gameStateActions->numActions)
        {
            b = 0;//'local' client
            fakePass = 1;
        }

        ClientAction* clientAction = &gameStateActions->clientActions[b].action;
        ActionTracking* actionTracking = &gameStateActions->clientActions[b].tracking;
        int cliId = actionTracking->clientId;
        SynchronizedClientState* client = &gameState->clientStates[cliId];
        SyncedGui* gui = &gameState->clientStates[cliId].gui;
        


        GuiStatePassType guiPass = GuiStatePassType_Synced;
        ge_int2 mouseLoc;
        int mouseState;

        if(fakePass)//redirect pointer above so they reflect the local client only.
        {
            guiPass = GuiStatePassType_NoLogic;
            mouseLoc = (ge_int2){gameStateActions->mouseLocx, gameStateActions->mouseLocy };
            mouseState = gameStateActions->mouseState;
            clientAction= NULL;
            actionTracking=NULL;
            cliId = -1;
            gui = &gameState->fakeGui;
            client = ThisClient(ALL_CORE_PARAMS_PASS);
        }
        else
        {
            guiPass = GuiStatePassType_Synced;
            mouseLoc.x = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_X];
            mouseLoc.y = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_Y];
            mouseState = clientAction->intParameters[CAC_MouseStateChange_Param_BUTTON_BITS];
        }


        GUI_RESET(ALL_CORE_PARAMS_PASS, gui, mouseLoc, mouseState, guiPass);

        int downDummy;
        if(GUI_BUTTON(GUIID, (ge_int2){0 ,0}, (ge_int2){50, 50},&downDummy) == 1)
        {
            printf("delete mode.");
            client->curTool = EditorTools_Delete;
        }
        if(GUI_BUTTON(GUIID, (ge_int2){50 ,0}, (ge_int2){50, 50},&downDummy) == 1)
        {
            printf("create mode.");
            client->curTool = EditorTools_Create;
        }




        GUI_SLIDER_INT(GUIID,  (ge_int2){0 ,GUI_PXPERSCREEN-50}, (ge_int2){400, 50}, &client->mapZView, 0, MAPDEPTH);


        GUI_Begin_ScrollArea(GUIID, (ge_int2){0,0},(ge_int2){0,0},(ge_int2){0,0});

            GUI_SetClip(gui, (ge_int2){10,220}, (ge_int2){50,50});

            GUI_BUTTON(GUIID, (ge_int2){0 ,200}, (ge_int2){50, 50},&downDummy);

        GUI_End_ScrollArea(gui);




        if(fakePass == 0)
            printf("cli: %d, mapz: %d\n", cliId, client->mapZView);

        //selection box
        
        
        GUI_RESET_POST(ALL_CORE_PARAMS_PASS,  gui);

        if(fakePass)
            continue;


        if (clientAction->actionCode == ClientActionCode_MouseStateChange)
        {
            
            printf("Processing Action From Client: %d\n", cliId);
            int buttons = clientAction->intParameters[CAC_MouseStateChange_Param_BUTTON_BITS];


            Print_GE_INT2(mouseLoc);
            
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
                        Peep* p = &gameState->peeps[pi];

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
                                    else
                                    {
                                        p->prevSelectionPeepPtr[cliId] = OFFSET_NULL;
                                        p->nextSelectionPeepPtr[cliId] = OFFSET_NULL;
                                    }
                                    client->selectedPeepsLastIdx = pi;

                                    PrintSelectionPeepStats(ALL_CORE_PARAMS_PASS, p);
                                    printf("ClientID: %d, selected unit\n", cliId);
                                }

                            }
                        }
                    }

                }

            }

            
            //command to location
            if(BITGET(buttons, MouseButtonBits_SecondaryReleased) && (GUI_MOUSE_ON_GUI(gui) == 0) && (gui->draggedOff == 0))
            {
                cl_uint curPeepIdx = client->selectedPeepsLastIdx;
                ge_int3 mapcoord;
                ge_int2 world2D;
                cl_uchar pathFindSuccess;

                offsetPtr pathOPtr;
                if (curPeepIdx != OFFSET_NULL)
                {
                    //Do an AStarSearch
                    Peep* curPeep = &gameState->peeps[curPeepIdx];
                    world2D.x = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16];
                    world2D.y = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16];
                    int occluded;

                    GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world2D, &mapcoord, &occluded, client->mapZView);
                    AStarSearchInstantiate(&gameState->mapSearchers[0]);
                    ge_int3 start = GE_INT3_WHOLE_Q16(curPeep->posMap_Q16);
                    mapcoord.z++;
                    ge_int3 end = mapcoord;
                    Print_GE_INT3(start);
                    Print_GE_INT3(end);
                    pathFindSuccess = AStarSearchRoutine(ALL_CORE_PARAMS_PASS, &gameState->mapSearchers[0], start, end, CL_INTMAX);
                    if (pathFindSuccess != 0)
                    {
                        //printf("--------------------------\n");
                        //AStarPrintSearchPathFrom(&gameState->mapSearchers[0], start);
                        //printf("--------------------------\n");
                        //AStarPrintSearchPathTo(&gameState->mapSearchers[0], end);
                        //printf("--------------------------\n");


                        pathOPtr = AStarFormPathSteps(ALL_CORE_PARAMS_PASS , &gameState->mapSearchers[0], &gameState->paths);
                        //AStarPrintPath(&gameState->paths, pathOPtr);
                        //printf("--------------------------\n");
                    }
                }

                while (curPeepIdx != OFFSET_NULL)
                {
                    Peep* curPeep = &gameState->peeps[curPeepIdx];

                    if (pathFindSuccess != 0)
                    {
                        curPeep->physics.drive.nextPathNodeOPtr = pathOPtr;

                        AStarPathNode* nxtPathNode;
                        OFFSET_TO_PTR(gameState->paths.pathNodes, curPeep->physics.drive.nextPathNodeOPtr, nxtPathNode);


                        ge_int3 worldloc;
                        MapToWorld(nxtPathNode->mapCoord_Q16, &worldloc);
                        curPeep->physics.drive.target_x_Q16 = worldloc.x;
                        curPeep->physics.drive.target_y_Q16 = worldloc.y;
                        curPeep->physics.drive.target_z_Q16 = worldloc.z;
                        curPeep->physics.drive.drivingToTarget = 1;

                        //restrict comms to new channel
                        curPeep->comms.orders_channel = RandomRange(client->selectedPeepsLastIdx, 0, 10000);
                        curPeep->comms.message_TargetReached = 0;
                        curPeep->comms.message_TargetReached_pending = 0;
                    }
                    curPeepIdx = curPeep->prevSelectionPeepPtr[cliId];
                }


            }
            //delete
            if(BITGET(buttons, MouseButtonBits_PrimaryReleased) && (GUI_MOUSE_ON_GUI(gui) == 0) && (gui->draggedOff == 0) && client->curTool == EditorTools_Delete)
            {
                printf("mouseongui: %d", GUI_MOUSE_ON_GUI(gui));

                ge_int2 world2DMouse;
                world2DMouse.x = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16];
                world2DMouse.y = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16];

                ge_int3 mapCoord;
                int occluded;
                GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world2DMouse, &mapCoord, &occluded, client->mapZView+1);
                
                printf("delete coord: ");
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

    cl_uint* tileData = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoordWhole);
    //do 3x3 kernel test


    //offsets[22] = (ge_int3){ 1, 1, 0 };
    //offsets[23] = (ge_int3){ -1, 1, 0 };
    //offsets[24] = (ge_int3){ 1, -1, 0 };
    //offsets[25] = (ge_int3){ -1, -1, 0 };

    cl_uint* data22 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, INT3_ADD(mapCoordWhole, staticData->directionalOffsets[22]));
    MapTile tile22 = MapDataGetTile(*data22);
    if (tile22 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerBTMRIGHT);
    }
    cl_uint* data24 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, INT3_ADD(mapCoordWhole, staticData->directionalOffsets[24]));
    MapTile tile24 = MapDataGetTile(*data24);
    if (tile24 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerTPRIGHT);
    }
    cl_uint* data23 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, INT3_ADD(mapCoordWhole, staticData->directionalOffsets[23]));
    MapTile tile23 = MapDataGetTile(*data23);
    if (tile23 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerBTMLEFT);
    }
    cl_uint* data25 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, INT3_ADD(mapCoordWhole, staticData->directionalOffsets[25]));
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



    cl_uint* data0 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, INT3_ADD(mapCoordWhole, staticData->directionalOffsets[0]));
    MapTile tile0 = MapDataGetTile(*data0);
    if (tile0 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerBTMRIGHT);
        BITSET(*tileData, MapTileFlags_LowCornerTPRIGHT);
    }
    cl_uint* data1 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, INT3_ADD(mapCoordWhole, staticData->directionalOffsets[1]));
    MapTile tile1 = MapDataGetTile(*data1);
    if (tile1 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerBTMLEFT);
        BITSET(*tileData, MapTileFlags_LowCornerTPLEFT);
    }
    cl_uint* data2 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, INT3_ADD(mapCoordWhole, staticData->directionalOffsets[2]));
    MapTile tile2 = MapDataGetTile(*data2);
    if (tile2 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerTPRIGHT);
        BITSET(*tileData, MapTileFlags_LowCornerTPLEFT);
    }
    cl_uint* data3 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, INT3_ADD(mapCoordWhole, staticData->directionalOffsets[3]));
    MapTile tile3 = MapDataGetTile(*data3);
    if (tile3 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerBTMRIGHT);
        BITSET(*tileData, MapTileFlags_LowCornerBTMLEFT);
    }


    
    if(MapDataLowCornerCount(*tileData) == 4)
        MapDataSetTile(tileData, MapTile_NONE);

    

}

void MapCreate(ALL_CORE_PARAMS, int x, int y)
{
    //printf("Creating Map..\n");

    int i = 0;

            cl_int perlin_z_Q16 = cl_perlin_2d_Q16(TO_Q16(x), TO_Q16(y), TO_Q16(1) >> 6, 8, 0) ;

           
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


                cl_uint* data = &gameState->map.levels[z].data[x][y];
                *data = tileType;


                BITBANK_SET_SUBNUMBER_UINT(data, MapTileFlags_RotBit1, 2, RandomRange(x*y,0,4));


                i++;
            }





}
void MapCreate2(ALL_CORE_PARAMS, int x, int y)
{
    MapCreateSlope(ALL_CORE_PARAMS_PASS, x, y);
    MapBuildTileView(ALL_CORE_PARAMS_PASS, x, y);
    MapUpdateShadow(ALL_CORE_PARAMS_PASS, x, y);

}

void StartupTests()
{
  printf("StartupTests Tests------------------------------------------------------:\n");
  if(1){
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
  if(1)
  {
    fixedPointTests();
  }

  if(1)
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


  if(1)
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



void InitRayGUI(ALL_CORE_PARAMS)
{
    SyncedGui* gui = &gameState->clientStates[gameStateActions->clientId].gui;

}





__kernel void game_init_single(ALL_CORE_PARAMS)
{
    printf("Game Initializing...\n");




    printf("Initializing StaticData Buffer..\n");
    MakeCardinalDirectionOffsets(&staticData->directionalOffsets[0]);

    StartupTests();


    InitRayGUI(ALL_CORE_PARAMS_PASS);


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
            gameState->sectors[secx][secy].lastPeepPtr = OFFSET_NULL;
            gameState->sectors[secx][secy].lock = 0;
        }
    }
    printf("Sectors Initialized.\n");





 
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
    for (int i = 0; i < ASTARPATHSTEPSSIZE; i++)
    {
        AStarInitPathNode(&gameState->paths.pathNodes[i]);
    }
    gameState->paths.nextListIdx = 0;



    printf("AStarTests:\n");
    //printf("AStarTests1:\n");
    //AStarSearchInstantiate(&gameState->mapSearchers[0]);
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
    AStarSearchInstantiate(&gameState->mapSearchers[0]);
    ge_int3 start = (ge_int3){ 0,0,MAPDEPTH - 2 };
    ge_int3 end = (ge_int3){ MAPDIM - 1,MAPDIM - 1,1 };
    AStarSearchRoutine(ALL_CORE_PARAMS_PASS, &gameState->mapSearchers[0], start, end, CL_INTMAX);

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

        WorldToMap(gameState->peeps[p].physics.base.pos_Q16, &gameState->peeps[p].posMap_Q16);
        gameState->peeps[p].lastGoodPosMap_Q16 = gameState->peeps[p].posMap_Q16;

        gameState->peeps[p].physics.shape.radius_Q16 = TO_Q16(1);
        BITCLEAR(gameState->peeps[p].stateBasic.bitflags0, PeepState_BitFlags_deathState);
        BITSET(gameState->peeps[p].stateBasic.bitflags0, PeepState_BitFlags_valid);
        BITSET(gameState->peeps[p].stateBasic.bitflags0, PeepState_BitFlags_visible);
        gameState->peeps[p].stateBasic.health = 10;
        gameState->peeps[p].stateBasic.deathState = 0;
        gameState->peeps[p].stateBasic.buriedGlitchState = 0;

        gameState->peeps[p].physics.base.v_Q16 = (ge_int3){ 0,0,0 };
        gameState->peeps[p].physics.base.vel_add_Q16 = (ge_int3){ 0,0,0 };
        gameState->peeps[p].physics.base.pos_post_Q16 = (ge_int3){ 0,0,0 };


        gameState->peeps[p].minDistPeepPtr = OFFSET_NULL;
        gameState->peeps[p].minDistPeep_Q16 = (1 << 30);
        gameState->peeps[p].mapSectorPtr = OFFSET_NULL_2D;
        gameState->peeps[p].mapSector_pendingPtr = OFFSET_NULL_2D;
        gameState->peeps[p].nextSectorPeepPtr = OFFSET_NULL;
        gameState->peeps[p].prevSectorPeepPtr = OFFSET_NULL;
        gameState->peeps[p].physics.drive.target_x_Q16 = gameState->peeps[p].physics.base.pos_Q16.x;
        gameState->peeps[p].physics.drive.target_y_Q16 = gameState->peeps[p].physics.base.pos_Q16.y;
        gameState->peeps[p].physics.drive.drivingToTarget = 0;


        gameState->peeps[p].stateBasic.faction = RandomRange(p,0,4);

                printf("d\n");

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            gameState->clientStates[i].selectedPeepsLastIdx = OFFSET_NULL;


            CL_CHECKED_ARRAY_SET(gameState->peeps[p].nextSelectionPeepPtr, MAX_CLIENTS, i, OFFSET_NULL)
                CL_CHECKED_ARRAY_SET(gameState->peeps[p].prevSelectionPeepPtr, MAX_CLIENTS, i, OFFSET_NULL)
        }

    }

    printf("Peeps Initialized.\n");

    for (cl_uint pi = 0; pi < MAX_PEEPS; pi++)
    {
        __global Peep* p = &gameState->peeps[pi];

        PeepAssignToSector_Detach(ALL_CORE_PARAMS_PASS, p);
        PeepAssignToSector_Insert(ALL_CORE_PARAMS_PASS, p);

    }

    printf("Peep Sector Assigment Finished\n");




    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        Particle* p = &gameState->particles[i];

        p->pos.x = FloatToQMP32((float)RandomRange(i, -spread, spread));
        p->pos.y = FloatToQMP32((float)RandomRange(i + 1, -spread, spread));

        p->vel.x = FloatToQMP32(((float)RandomRange(i, -1000, 1000)) * 0.001f);
        p->vel.y = FloatToQMP32(((float)RandomRange(i + 1, -1000, 1000)) * 0.001f);
    }


}
void PeepDraw(ALL_CORE_PARAMS, Peep* peep)
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

void ParticleDraw(ALL_CORE_PARAMS, Particle* particle, cl_uint ptr)
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
        Peep* p = &gameState->peeps[globalid]; 
        PeepPreUpdate2(p);
    }
}
__kernel void game_update(ALL_CORE_PARAMS)
{
    // Get the index of the current element to be processed
    int globalid = get_global_id(0);
    int localid = get_local_id(0);


    if (globalid < MAX_PEEPS) {
        Peep* p = &gameState->peeps[globalid]; 
        PeepUpdate(ALL_CORE_PARAMS_PASS, p);
        PeepDraw(ALL_CORE_PARAMS_PASS, p);
    }

    if (globalid < MAX_PARTICLES) {
        Particle* p = &gameState->particles[globalid];
        ParticleUpdate(ALL_CORE_PARAMS_PASS, p);
        ParticleDraw(ALL_CORE_PARAMS_PASS, p, globalid);
    }

    //update map view
    if (ThisClient(ALL_CORE_PARAMS_PASS)->mapZView != ThisClient(ALL_CORE_PARAMS_PASS)->mapZView_1)
    {
        cl_uint chunkSize = (MAPDIM * MAPDIM) / GAME_UPDATE_WORKITEMS;
        if (chunkSize == 0)
            chunkSize = 1;


        for (cl_ulong i = 0; i < chunkSize; i++)
        {
            cl_ulong xyIdx = i + globalid * chunkSize;
            
            if (xyIdx < (MAPDIM * MAPDIM))
            {
                MapBuildTileView(ALL_CORE_PARAMS_PASS, xyIdx % MAPDIM, xyIdx / MAPDIM);
                MapUpdateShadow(ALL_CORE_PARAMS_PASS, xyIdx % MAPDIM, xyIdx / MAPDIM);
            }
        }
    }



}



__kernel void game_post_update_single( ALL_CORE_PARAMS )
{

    ThisClient(ALL_CORE_PARAMS_PASS)->mapZView_1 = ThisClient(ALL_CORE_PARAMS_PASS)->mapZView;
    

}

__kernel void game_preupdate_1(ALL_CORE_PARAMS) {


    // Get the index of the current element to be processed
    int globalid = get_global_id(0);

    if (globalid >= WARPSIZE)
        return;
   

    cl_uint chunkSize = MAX_PEEPS / WARPSIZE;
    if (chunkSize == 0)
        chunkSize = 1;
    
    for (cl_ulong pi = 0; pi < chunkSize; pi++)
    {
        if (pi + globalid * chunkSize < MAX_PEEPS)
        {
            Peep* p;
            CL_CHECKED_ARRAY_GET_PTR(gameState->peeps, MAX_PEEPS, pi + globalid * chunkSize, p)
            CL_CHECK_NULL(p)

           


            global volatile MapSector* mapSector;
            OFFSET_TO_PTR_2D(gameState->sectors, p->mapSectorPtr, mapSector);
            CL_CHECK_NULL(mapSector)

            global volatile cl_uint* lock = (global volatile cl_uint*) & mapSector->lock;
            CL_CHECK_NULL(lock)



            cl_uint reservation;

            reservation = atomic_add(lock, 1) + 1;
            barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);

            while (*lock != reservation) {}

            PeepAssignToSector_Detach(ALL_CORE_PARAMS_PASS, p);

            atomic_dec(lock);
        }
    }


}


__kernel void game_preupdate_2(ALL_CORE_PARAMS) {


    // Get the index of the current element to be processed
    int globalid = get_global_id(0);



    if (globalid >= WARPSIZE)
        return;


    cl_uint chunkSize = MAX_PEEPS / WARPSIZE;
    if (chunkSize == 0)
        chunkSize = 1;

    for (cl_ulong pi = 0; pi < chunkSize; pi++)
    {
        if (pi + globalid * chunkSize < MAX_PEEPS)
        {

            Peep* p = &gameState->peeps[pi + globalid * chunkSize];

            global volatile MapSector* mapSector;
            OFFSET_TO_PTR_2D(gameState->sectors, p->mapSector_pendingPtr, mapSector);


            CL_CHECK_NULL(mapSector)
                if (mapSector == NULL)
                    continue;
            global volatile cl_uint* lock = (global volatile cl_uint*) & mapSector->lock;
            CL_CHECK_NULL(lock)


            int reservation = atomic_add(lock, 1) + 1;

            barrier(CLK_GLOBAL_MEM_FENCE);

            while (atomic_add(lock, 0) != reservation) {}

            PeepAssignToSector_Insert(ALL_CORE_PARAMS_PASS, p);

            atomic_dec(lock);
        }
    }
}


__kernel void size_tests(__global SIZETESTSDATA* data)
{

    data->gameStateStructureSize = sizeof(GameState);
    data->staticDataStructSize = sizeof(StaticData);

}

