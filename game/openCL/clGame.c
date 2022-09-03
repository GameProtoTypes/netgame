


#include "cpugpuvectortypes.h"
#include "cl_type_glue.h"
#include "fixedpoint_opencl.h"
#include "peep.h"
#include "randomcl.h"
#include "perlincl.h"


//#define PEEP_ALL_ALWAYS_VISIBLE
#define PEEP_DISABLE_TILECORRECTIONS


#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

#define ALL_CORE_PARAMS  __global StaticData* staticData, \
__global GameState* gameState,\
__global GameStateActions* gameStateActions, \
__global float* peepVBOBuffer, \
__global float* particleVBOBuffer, \
__global cl_uchar* mapTile1VBO, \
__global cl_uint* mapTile1AttrVBO, \
__global cl_uint* mapTile1OtherAttrVBO, \
__global cl_uchar* mapTile2VBO, \
__global cl_uint* mapTile2AttrVBO, \
__global cl_uint* mapTile2OtherAttrVBO


#define ALL_CORE_PARAMS_PASS  staticData, \
gameState, \
gameStateActions, \
peepVBOBuffer, \
particleVBOBuffer, \
mapTile1VBO, \
mapTile1AttrVBO, \
mapTile1OtherAttrVBO, \
mapTile2VBO, \
mapTile2AttrVBO, \
mapTile2OtherAttrVBO



void Print_GE_INT2(ge_int2 v)
{
    printf("{%d,%d}\n", v.x, v.y);
}
void Print_GE_INT3(ge_int3 v)
{
    printf("{%d,%d,%d}\n", v.x, v.y, v.z);
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


    return (bank & mask) >> lsbBitIdx;
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


    if (GE_VECTOR3_EQUAL(enterDir, staticData->directionalOffsets[0]))
    {
        return 2 - (BITGET_MF(*data, MapTileFlags_LowCornerBTMLEFT) + BITGET_MF(*data, MapTileFlags_LowCornerTPLEFT));
    }
    else if (GE_VECTOR3_EQUAL(enterDir, staticData->directionalOffsets[1]))
    {
        return 2 - (BITGET_MF(*data, MapTileFlags_LowCornerBTMRIGHT) + BITGET_MF(*data, MapTileFlags_LowCornerTPRIGHT));
    }
    else if (GE_VECTOR3_EQUAL(enterDir, staticData->directionalOffsets[2]))
    {
        return 2 - (BITGET_MF(*data, MapTileFlags_LowCornerBTMRIGHT) + BITGET_MF(*data, MapTileFlags_LowCornerBTMLEFT));
    }
    else if (GE_VECTOR3_EQUAL(enterDir, staticData->directionalOffsets[3]))
    {
        return 2 - (BITGET_MF(*data, MapTileFlags_LowCornerTPRIGHT) + BITGET_MF(*data, MapTileFlags_LowCornerTPLEFT));
    }
    return 2;
}


cl_uchar MapHas2LowAdjacentCorners(ALL_CORE_PARAMS, ge_int3 mapCoords)
{
    cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoords);

    if (MapDataGetTile(*data) == MapTile_NONE)
        return 0;

    if (BITGET_MF(*data, MapTileFlags_LowCornerTPLEFT) + BITGET_MF(*data, MapTileFlags_LowCornerTPRIGHT) == 2)
        return 1;

    if (BITGET_MF(*data, MapTileFlags_LowCornerTPRIGHT) + BITGET_MF(*data, MapTileFlags_LowCornerBTMRIGHT) == 2)
        return 2;

    if (BITGET_MF(*data, MapTileFlags_LowCornerBTMRIGHT) + BITGET_MF(*data, MapTileFlags_LowCornerBTMLEFT) == 2)
        return 3;

    if (BITGET_MF(*data, MapTileFlags_LowCornerBTMLEFT) + BITGET_MF(*data, MapTileFlags_LowCornerTPLEFT) == 2)
        return 4;
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
cl_uchar MapTileDataCornerCount(cl_int tileData)
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

    return MapTileDataCornerCount(*data);
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
        cl_uchar ridgeType = MapHas2LowAdjacentCorners(ALL_CORE_PARAMS_PASS, mapcoord);
        if (ridgeType == 0)
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
        return 1;
    }
    else
    {
        cl_uchar ridgeType = MapRidgeType(ALL_CORE_PARAMS_PASS, mapcoord, enterDirection);
        if (ridgeType == 0)
            return 1;
    }
    return 0;


}
void AStarNodeInstantiate(AStarNode* node)
{
    node->g_Q16 = TO_Q16(0);
    node->h_Q16 = TO_Q16(0);
    node->next = NULL;  
    node->prev = NULL;
    node->tileIdx.x = -1;
    node->tileIdx.y = -1;
    node->tileIdx.z = -1;

}
void AStarInitPathNode(AStarPathNode* node)
{
    node->mapCoord_Q16 = (ge_int3){ 0,0,0 };
    node->next = NULL;
    node->prev = NULL;
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

    

    search->endNode = NULL;
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
    return MapTileCoordValid(GE_SHORT3_TO_INT3(node->tileIdx));
}
cl_uchar AStarNode2NodeTraversible(ALL_CORE_PARAMS, AStarNode* node, AStarNode* prevNode)
{  
    ge_int3 delta = GE_INT3_ADD(node->tileIdx, GE_INT3_NEG(GE_SHORT3_TO_INT3( prevNode->tileIdx ) ));
    if (MapTileCoordEnterable(ALL_CORE_PARAMS_PASS, GE_SHORT3_TO_INT3(node->tileIdx), delta) ==0)
        return 0;

    ge_int3 downCoord = GE_INT3_ADD(GE_SHORT3_TO_INT3(node->tileIdx), staticData->directionalOffsets[5]);
    MapTile tileDown = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, downCoord);
    if (tileDown != MapTile_NONE)
    {
        //it can be stood on, thats good

        return 1;


    }



    return 0;
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

void AStarOpenHeapTrickleDown(AStarSearch* search, cl_int index)
{
    cl_int largerChild;
    AStarNode* top = search->openHeap[index];
    while (index < search->openHeapSize / 2)
    {
        int leftChild = 2 * index + 1;
        int rightChild = leftChild + 1;

        if (rightChild < search->openHeapSize && AStarOpenHeapKey(search, search->openHeap[leftChild]) > AStarOpenHeapKey(search, search->openHeap[rightChild]))
            largerChild = rightChild;
        else
            largerChild = leftChild;

        if (AStarOpenHeapKey(search, top) <= AStarOpenHeapKey(search, search->openHeap[largerChild]))
            break;

        search->openHeap[index] = search->openHeap[largerChild];
        index = largerChild;

    }
    
    search->openHeap[index] = top;
}

AStarNode* AStarOpenHeapRemove(AStarSearch* search)
{
    AStarNode* root = search->openHeap[0];
    search->openHeap[0] = search->openHeap[search->openHeapSize-1];
    search->openHeapSize--;
    AStarOpenHeapTrickleDown(search, 0);

    return root;
}
AStarNode* AStarRemoveFromOpen(AStarSearch* search)
{
    AStarNode* node = AStarOpenHeapRemove(search);
    search->openMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z] = 0;
    return node;
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

int AStarOpenHeapKey(AStarSearch* search, AStarNode* node)
{
    //f
    return node->g_Q16 + node->h_Q16;
}

void AStarOpenHeapTrickleUp(AStarSearch* search, cl_int index)
{
    cl_int prev = (index - 1) / 2;
    AStarNode* bottom = search->openHeap[index];

    while (index > 0 && AStarOpenHeapKey(search, search->openHeap[prev]) > AStarOpenHeapKey(search, bottom))
    {
        search->openHeap[index] = search->openHeap[prev];
        index = prev;
        prev = (prev - 1) / 2;
    }
    search->openHeap[index] = bottom;
}


void AStarOpenHeapInsert(AStarSearch* search, AStarNode* node)
{
    search->openHeap[search->openHeapSize] = node;
    AStarOpenHeapTrickleUp(search, search->openHeapSize);
    search->openHeapSize++;
    if (search->openHeapSize > ASTARHEAPSIZE)
        printf("ERROR: AStarHeap Size Greater than ASTARHEAPSIZE!\n");


}
void AStarAddToOpen(AStarSearch* search, AStarNode* node)
{
    AStarOpenHeapInsert(search, node);
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
        curNode = curNode->prev;
    }
}
void AStarPrintSearchPathFrom(AStarSearch* search, ge_int3 startTile)
{
    AStarNode* curNode = &search->details[startTile.x][startTile.y][startTile.z];
    while (curNode != NULL)
    {
        AStarPrintNodeStats(curNode);
        curNode = curNode->next;
    }
}

void AStarPrintPath(AStarPathNode* startNode)
{
    AStarPathNode* curNode = startNode;
    while (curNode != NULL)
    {
        AStarPrintPathNodeStats(curNode);
        curNode = curNode->next;
    }
}

cl_uchar AStarSearchRoutine(ALL_CORE_PARAMS, AStarSearch* search, ge_int3 startTile, ge_int3 destTile, int maxIterations)
{
    if (MapTileCoordValid(startTile) == 0)
    {
        return 0;
    }
    if (MapTileCoordValid(destTile) == 0)
    {
        return 0;
    }
    if (MapTileCoordStandInValid(ALL_CORE_PARAMS_PASS, startTile) == 0)
    {
        return 0;
    }
    if (MapTileCoordStandInValid(ALL_CORE_PARAMS_PASS, destTile)==0)
    {
        return 0;
    }

    printf("starting search\n");

    AStarNode* startNode = &search->details[startTile.x][startTile.y][startTile.z];
    AStarNode* targetNode = &search->details[destTile.x][destTile.y][destTile.z];
    search->startNode = startNode;

    //add start to openList
    startNode->h_Q16 = AStarNodeDistanceHuristic(search, startNode, targetNode);
    AStarAddToOpen(search, startNode);


    cl_uchar foundDest = 0;
    int iterationCount = maxIterations;
    while (search->openHeapSize > 0 && iterationCount > 0)
    {
        
        //find node in open with lowest f cost
        AStarNode* current = AStarRemoveFromOpen(search);
        
        

        if (GE_VECTOR3_EQUAL(current->tileIdx, destTile) )
        {
            printf("Goal Found\n");
            search->endNode = targetNode;
            search->endNode->next = NULL;
            startNode->prev = NULL;

            //form next links
            AStarNode* curNode = targetNode;
            while (curNode != NULL)
            {
                AStarNode* p = curNode->prev;
                if(p != NULL)
                    p->next = curNode;

                curNode = p;
            }


            return 1;//found dest
        }
        
        //AStarAddToClosed(search, current);


        //5 neighbors
        for (int i = 0; i <= 25; i++)
        { 
            ge_int3 prospectiveTileCoord;
            prospectiveTileCoord.x = current->tileIdx.x + staticData->directionalOffsets[i].x;
            prospectiveTileCoord.y = current->tileIdx.y + staticData->directionalOffsets[i].y;
            prospectiveTileCoord.z = current->tileIdx.z + staticData->directionalOffsets[i].z;
 
            if (MapTileCoordValid(prospectiveTileCoord)==0)
            {
                continue;
            }
            
            AStarNode* prospectiveNode = &search->details[prospectiveTileCoord.x][prospectiveTileCoord.y][prospectiveTileCoord.z];

            if ((AStarNode2NodeTraversible(ALL_CORE_PARAMS_PASS,  prospectiveNode, current) == 0))
            {
                continue;
            }


            int totalMoveCost = current->g_Q16 + AStarNodeDistanceHuristic(search, current, prospectiveNode);

            if ((totalMoveCost < prospectiveNode->g_Q16) || prospectiveNode->g_Q16 == 0)
            {
                prospectiveNode->g_Q16 = totalMoveCost;
                prospectiveNode->h_Q16 = AStarNodeDistanceHuristic(search, prospectiveNode, targetNode);
                
                prospectiveNode->prev = current;

               // printf("G: "); PrintQ16(prospectiveNode->g_Q16); printf("H: ");  PrintQ16(prospectiveNode->h_Q16);
                AStarAddToOpen(search, prospectiveNode);
            }
        }

        iterationCount--;
    }

    
    printf("FAIL %d\n", search->openHeapSize);
    return foundDest;
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
    if (GE_INT3_ZERO(a))
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
    int idx = list->nextListIdx;
    while (list->pathNodes[idx].next != NULL || list->pathNodes[idx].next == -1)
    {
        idx++;
        if (idx >= ASTARPATHSTEPSSIZE)
            idx = 0;
    }
    list->nextListIdx = idx+1;
    return idx;
}

//get the last path node from a node in a path
AStarPathNode* AStarPathNode_LastPathNode(AStarPathNode* pathNode)
{
    AStarPathNode* curNode = pathNode;
    while(curNode->next != NULL)
    {
        curNode = curNode->next;
    }
    return curNode;
}

AStarPathNode* AStarFormPathSteps(ALL_CORE_PARAMS, AStarSearch* search, AStarPathSteps* steps)
{
    //grab a unused Node from pathNodes, and start building the list .
    AStarNode* curNode = search->startNode;


    AStarPathNode* startNode = NULL;
    AStarPathNode* pNP = NULL;
    int i = 0;
    while (curNode != NULL)
    { 

        int index = AStarPathStepsNextFreePathNode(&gameState->paths);

        AStarPathNode* pN = &gameState->paths.pathNodes[index];

        if (i == 0)
            startNode = pN;


        ge_int3 holdTileCoord = GE_SHORT3_TO_INT3(  curNode->tileIdx  );

        //put location at center of tile
        ge_int3 tileCenter = GE_INT3_TO_Q16(holdTileCoord);
        tileCenter.x += TO_Q16(1) >> 1;
        tileCenter.y += TO_Q16(1) >> 1;
        tileCenter.z += TO_Q16(1) >> 1;


        pN->mapCoord_Q16 = tileCenter;

        if (pNP != NULL)
        {
            pNP->next = pN;
        }
        pNP = pN;

        if (curNode->next != NULL) {
            //iterate until joint in path.
            ge_int3 delta;
            AStarNode* n2 = curNode;
            do
            {

                n2 = n2->next;
                if (n2 != NULL) {
                    delta = GE_INT3_ADD(n2->tileIdx, GE_INT3_NEG(holdTileCoord));
                }
                else
                    delta = (ge_int3){ 0,0,0 };

            } while ((n2 != NULL) && (GE_INT3_WHACHAMACOLIT1_ENTRY(delta) == 1));

            if (n2 != NULL) {
                if (curNode != n2->prev)
                    curNode = n2->prev;
                else
                    curNode = n2;
            }
            else
                curNode = search->endNode;
        }
        else
            curNode = NULL;


        i++;
    }
    pNP->next = NULL;



    //form prev links
    AStarPathNode* curNode2 = startNode;
    while (curNode2 != NULL)
    {
        AStarPathNode* p = curNode2->next;
        if (p != NULL)
            p->prev = curNode2;

        curNode2 = p;
    }



    return startNode;
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
    ge_int3 W = GE_INT3_SUB(point, triangle->base.verts_Q16[0]);


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


    ge_int3 W = GE_INT3_SUB(point_Q16, P1);

    //printf("W: ");
    //Print_GE_INT3_Q16(W);


    int dot = GE_INT3_DOT_PRODUCT_Q16(W, N_n);
    //printf("dot: ");
    //PrintQ16(dot);
    ge_int3 term2 = GE_INT3_SCALAR_MUL_Q16(dot, GE_INT3_NEG(N_n));
    P_prime = GE_INT3_ADD(point_Q16, term2);


    //Triangle2DHeavy_ProjectedPoint(triangle, point_Q16, &P_prime_bary, &P_prime);

    //printf("P_prime: ");
    //Print_GE_INT3_Q16(P_prime);

    P_prime_bary = Triangle3D_ToBaryCentric(triangle, P_prime);
    //printf("P_prime_bary: ");
    //Print_GE_INT3_Q16(P_prime_bary);
    cl_uchar onSurface = BaryCentric_In_Triangle_Q16(P_prime_bary);

    if (onSurface == 1)
    {
        *dist_Q16 = ge_length_v3_Q16(GE_INT3_SUB(point_Q16, P_prime));

        return P_prime;
    }

    ge_int3 P1_P_prime = GE_INT3_SUB(P_prime, P1);
    ge_int3 P2_P_prime = GE_INT3_SUB(P_prime, P2);
    ge_int3 P3_P_prime = GE_INT3_SUB(P_prime, P3);

    ge_int3 R1 = GE_INT3_SUB(P1, P2);
    ge_int3 R2 = GE_INT3_SUB(P2, P3);
    ge_int3 R3 = GE_INT3_SUB(P3, P1);

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
    ge_int3 P_prime_C1 = GE_INT3_SUB(P_prime, D1R3);
    ge_int3 P_prime_C2 = GE_INT3_SUB(P_prime, D2R1);
    ge_int3 P_prime_C3 = GE_INT3_SUB(P_prime, D3R2);


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

    ge_int3 L1 = GE_INT3_ADD(J1, P_prime_C1);
    ge_int3 L2 = GE_INT3_ADD(J2, P_prime_C2);
    ge_int3 L3 = GE_INT3_ADD(J3, P_prime_C3);

    //get closest L to P_prime
    int L1D = ge_length_v3_Q16( GE_INT3_SUB(L1, P_prime) );
    int L2D = ge_length_v3_Q16( GE_INT3_SUB(L2, P_prime) );
    int L3D = ge_length_v3_Q16( GE_INT3_SUB(L3, P_prime) );


    if (L1D < L2D && L1D < L2D)
    {
        *dist_Q16 = ge_length_v3_Q16(GE_INT3_SUB(point_Q16, L1));

        return L1;
    }
    else if (L2D < L1D && L2D < L3D)
    {
        *dist_Q16 = ge_length_v3_Q16(GE_INT3_SUB(point_Q16, L2));

        return L2;
    }
    else
    {
        *dist_Q16 = ge_length_v3_Q16(GE_INT3_SUB(point_Q16, L3));

        return L3;
    }

}



void Triangle3DMakeHeavy(Triangle3DHeavy* triangle)
{
    triangle->u_Q16 = GE_INT3_SUB(triangle->base.verts_Q16[1], triangle->base.verts_Q16[0]);//P_2 - P_1
    triangle->v_Q16 = GE_INT3_SUB(triangle->base.verts_Q16[2], triangle->base.verts_Q16[0]);//P_3 - P_1


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

    cl_uchar lowCornerCount = MapTileDataCornerCount(*tileData);

    if (lowCornerCount > 0) 
    {
        if (BITGET_MF(*tileData, MapTileFlags_LowCornerBTMLEFT) != 0)
            E.z = (TO_Q16(-1) >> 1);

        if (BITGET_MF(*tileData, MapTileFlags_LowCornerBTMRIGHT) != 0)
            F.z = (TO_Q16(-1) >> 1);

        if (BITGET_MF(*tileData, MapTileFlags_LowCornerTPLEFT) != 0)
            H.z = (TO_Q16(-1) >> 1);

        if (BITGET_MF(*tileData, MapTileFlags_LowCornerTPRIGHT) != 0)
            G.z = (TO_Q16(-1) >> 1);


        //simple ramp cases
        if      (E.z < 0 && F.z < 0 && G.z > 0 && H.z > 0)
            X.z = 0;
        else if (F.z < 0 && G.z < 0 && H.z > 0 && E.z > 0)
            X.z = 0;
        else if (G.z < 0 && H.z < 0 && E.z > 0 && F.z > 0)
            X.z = 0;
        else if (H.z < 0 && E.z < 0 && F.z > 0 && G.z > 0)
            X.z = 0;
        else if (lowCornerCount == 3)
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
    Triangle3D_Make2Face(&hull->triangles[i++], &hull->triangles[i++], &bottomFace[0]);



    ge_int3 NegYFace[4];
    NegYFace[0] = A;
    NegYFace[1] = E;
    NegYFace[2] = F;
    NegYFace[3] = B;
    Triangle3D_Make2Face(&hull->triangles[i++], &hull->triangles[i++], &NegYFace[0]);

    ge_int3 POSYFace[4];
    POSYFace[0] = C;
    POSYFace[1] = G;
    POSYFace[2] = H;
    POSYFace[3] = D;
    Triangle3D_Make2Face(&hull->triangles[i++], &hull->triangles[i++], &POSYFace[0]);

    ge_int3 POSXFace[4];
    POSXFace[0] = B;
    POSXFace[1] = F;
    POSXFace[2] = G;
    POSXFace[3] = C;
    Triangle3D_Make2Face(&hull->triangles[i++], &hull->triangles[i++], &POSXFace[0]);

    ge_int3 NEGXFace[4];
    NEGXFace[0] = D;
    NEGXFace[1] = H;
    NEGXFace[2] = E;
    NEGXFace[3] = A;
    Triangle3D_Make2Face(&hull->triangles[i++], &hull->triangles[i++], &NEGXFace[0]);

    ge_int3 TOPFace1[4];
    TOPFace1[0] = X;
    TOPFace1[1] = G;
    TOPFace1[2] = F;
    TOPFace1[3] = E;
    Triangle3D_Make2Face(&hull->triangles[i++], &hull->triangles[i++], &TOPFace1[0]);

    ge_int3 TOPFace2[4];
    TOPFace2[0] = X;
    TOPFace2[1] = E;
    TOPFace2[2] = H;
    TOPFace2[3] = G;
    Triangle3D_Make2Face(&hull->triangles[i++], &hull->triangles[i++], &TOPFace2[0]);

}




void PeepPrint(Peep* peep)
{
    PrintQ16(peep->physics.drive.target_x_Q16);
}


void PeepPeepPhysics(ALL_CORE_PARAMS, Peep* peep, Peep* otherPeep)
{

    //calculate force based on penetration distance with otherPeep.
    ge_int3 d_Q16 = GE_INT3_ADD(otherPeep->physics.base.pos_Q16, GE_INT3_NEG(peep->physics.base.pos_Q16));
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
    if ((peep->comms.orders_channel == otherPeep->comms.orders_channel))
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


    cl_int dist_Q16 = ge_length_v3_Q16(GE_INT3_ADD(peep->physics.base.pos_Q16, -otherPeep->physics.base.pos_Q16));

    if (dist_Q16 < peep->minDistPeep_Q16)
    {
        peep->minDistPeep_Q16 = dist_Q16;
        peep->minDistPeepIdx = otherPeep->Idx;
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


//inTileCoord_Q16 is [-1,1],[-1,1]
cl_int MapTileZHeight_Q16(cl_uint* tileData, ge_int2 inTileCoord_Q16)
{

    //4 corners
    

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
            
            ge_int3 point_vert = GE_INT3_SUB(point, vert);
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

    /*
    {
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 0, 0 }, & data[6], & tileCenters_Q16[6]);

        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 0, -1 }, & data[6], & tileCenters_Q16[6]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 1, -1 }, & data[7], & tileCenters_Q16[7]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 1, -1 }, & data[8], & tileCenters_Q16[8]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 0, -1 }, & data[9], & tileCenters_Q16[9]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, -1, -1 }, & data[10], & tileCenters_Q16[10]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, -1, -1 }, & data[11], & tileCenters_Q16[11]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, -1, -1 }, & data[12], & tileCenters_Q16[12]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 1, -1 }, & data[13], & tileCenters_Q16[13]);

        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 0, 1 }, & data[14], & tileCenters_Q16[14]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 1, 1 }, & data[15], & tileCenters_Q16[15]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 1, 1 }, & data[16], & tileCenters_Q16[16]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 0, 1 }, & data[17], & tileCenters_Q16[17]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, -1, 1 }, & data[18], & tileCenters_Q16[18]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, -1, 1 }, & data[19], & tileCenters_Q16[19]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, -1, 1 }, & data[20], & tileCenters_Q16[20]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 1, 1 }, & data[21], & tileCenters_Q16[21]);


        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 1, 0 }, & data[22], & tileCenters_Q16[23]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 1, 0 }, & data[23], & tileCenters_Q16[24]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, -1, 0 }, & data[24], & tileCenters_Q16[25]);
        PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, -1, 0 }, & data[25], & tileCenters_Q16[26]);
    }
    */
    //printf("peep Pos: "); Print_GE_INT3_Q16(peep->physics.base.pos_Q16);
    ConvexHull hull;//hull for use below
    for (int i = 0; i < 6; i++)
    {
        
        MapTile tile = tiles[i];
        
        if (tile != MapTile_NONE)
        {
            ge_int3 futurePos;
            futurePos.x = peep->physics.base.pos_Q16.x + peep->physics.base.v_Q16.x;
            futurePos.y = peep->physics.base.pos_Q16.y + peep->physics.base.v_Q16.y;
            futurePos.z = peep->physics.base.pos_Q16.z + peep->physics.base.v_Q16.z;

            ge_int3 nearestPoint;
            cl_uchar insideSolidRegion;
  
            MapTileConvexHull_From_TileData(&hull, &tileDatas[i]);
            ge_int3 peepPosLocalToHull_Q16 = GE_INT3_SUB(futurePos, tileCenters_Q16[i]);

            peepPosLocalToHull_Q16 = GE_INT3_DIV_Q16(peepPosLocalToHull_Q16, (ge_int3) {
                TO_Q16(MAP_TILE_SIZE), TO_Q16(MAP_TILE_SIZE)
                    , TO_Q16(MAP_TILE_SIZE)
            });

            nearestPoint = MapTileConvexHull_ClosestPointToPoint(&hull, peepPosLocalToHull_Q16);
            insideSolidRegion = MapTileConvexHull_PointInside(&hull, peepPosLocalToHull_Q16);

            nearestPoint = GE_INT3_MUL_Q16(nearestPoint, (ge_int3) {
                TO_Q16(MAP_TILE_SIZE), TO_Q16(MAP_TILE_SIZE)
                    , TO_Q16(MAP_TILE_SIZE)
            });

            nearestPoint = GE_INT3_ADD(nearestPoint, tileCenters_Q16[i]);
        

            ge_int3 A;
            A.x = futurePos.x - nearestPoint.x;
            A.y = futurePos.y - nearestPoint.y;
            A.z = futurePos.z - nearestPoint.z;

            //make A vector always point to outside the shape
            if(insideSolidRegion==1)
               A = GE_INT3_NEG(A);


            ge_int3 An = A;
            cl_int mag;
            ge_normalize_v3_Q16(&An, &mag);


            if (mag < peep->physics.shape.radius_Q16)
            {
                cl_int dot;
                ge_dot_product_3D_Q16(peep->physics.base.v_Q16, An, &dot);
                ge_int3 B;//velocity to cancel
                B.x = MUL_PAD_Q16(An.x, dot);
                B.y = MUL_PAD_Q16(An.y, dot);
                B.z = MUL_PAD_Q16(An.z, dot);


                int pushAmt;
                if(insideSolidRegion)
                    pushAmt = (peep->physics.shape.radius_Q16 + mag);
                else
                    pushAmt = (peep->physics.shape.radius_Q16 - mag);


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

    if (WHOLE_Q16(len) < 1)//within range of current target
    {
        if (peep->physics.drive.drivingToTarget)
        {

            if (peep->physics.drive.next == NULL)
            {
                //final node reached.
                peep->comms.message_TargetReached_pending = 255;//send the message
            }
            else
            {

                peep->physics.drive.next = peep->physics.drive.next->next;
                if (peep->physics.drive.next != NULL) 
                {
                    ge_int3 nextTarget_Q16 = peep->physics.drive.next->mapCoord_Q16;
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
        targetVelocity.x = d.x >> 2;
        targetVelocity.y = d.y >> 2;
        targetVelocity.z = d.z >> 2;


        peep->physics.base.vel_add_Q16.x += targetVelocity.x;
        peep->physics.base.vel_add_Q16.y += targetVelocity.y;
        peep->physics.base.vel_add_Q16.z += targetVelocity.z;


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
    OFFSET_TO_PTR_2D(gameState->sectors, peep->mapSectorIdx, curSector);

    if ((curSector != newSector))
    {

        if (curSector != NULL)
        {

            //remove peep from old sector
            if ((peep->prevSectorPeepIdx != OFFSET_NULL))
            {
                gameState->peeps[peep->prevSectorPeepIdx].nextSectorPeepIdx = peep->nextSectorPeepIdx;
            }
            if ((peep->nextSectorPeepIdx != OFFSET_NULL))
            {
                gameState->peeps[peep->nextSectorPeepIdx].prevSectorPeepIdx = peep->prevSectorPeepIdx;
            }


            if (curSector->lastPeepIdx == peep->Idx) {

                if (peep->prevSectorPeepIdx != OFFSET_NULL)
                    curSector->lastPeepIdx = gameState->peeps[peep->prevSectorPeepIdx].Idx;
                else
                    curSector->lastPeepIdx = OFFSET_NULL;
                
                if (curSector->lastPeepIdx != OFFSET_NULL)
                    gameState->peeps[curSector->lastPeepIdx].nextSectorPeepIdx = OFFSET_NULL;
            }

            //completely detach
            peep->nextSectorPeepIdx = OFFSET_NULL;
            peep->prevSectorPeepIdx = OFFSET_NULL;

        }

        //assign new sector for next stage
        peep->mapSector_pendingIdx = newSector->idx;
    }

}
void PeepAssignToSector_Insert(ALL_CORE_PARAMS, Peep* peep)
{
    //assign new sector
    if (!CL_VECTOR2_EQUAL(peep->mapSectorIdx, peep->mapSector_pendingIdx))
    {
        peep->mapSectorIdx = peep->mapSector_pendingIdx;

        MapSector* mapSector;
        OFFSET_TO_PTR_2D(gameState->sectors, peep->mapSectorIdx, mapSector)


        //put peep in the sector.  extend list
        if (mapSector->lastPeepIdx != OFFSET_NULL)
        {
            gameState->peeps[mapSector->lastPeepIdx].nextSectorPeepIdx = peep->Idx;
            peep->prevSectorPeepIdx = gameState->peeps[mapSector->lastPeepIdx].Idx;
        }
        mapSector->lastPeepIdx = peep->Idx;
    }
}
void PeepPreUpdate1(Peep* peep)
{
    peep->physics.base.v_Q16.z += peep->physics.base.vel_add_Q16.z;
    peep->physics.base.v_Q16.y += peep->physics.base.vel_add_Q16.y;
    peep->physics.base.v_Q16.x += peep->physics.base.vel_add_Q16.x;

    peep->physics.base.vel_add_Q16.z = 0;
    peep->physics.base.vel_add_Q16.y = 0;
    peep->physics.base.vel_add_Q16.x = 0;
}

void PeepPreUpdate2(Peep* peep)
{
    
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
    peep->minDistPeepIdx = OFFSET_NULL;

    MapSector* cursector;
    OFFSET_TO_PTR_2D(gameState->sectors, peep->mapSectorIdx, cursector);

    //traverse sector
    int minx = cursector->idx.x - 1; if (minx == 0xFFFFFFFF) minx = 0;
    int miny = cursector->idx.y - 1; if (miny == 0xFFFFFFFF) miny = 0;

    int maxx = cursector->idx.x + 1; if (maxx >= SQRT_MAXSECTORS) maxx = SQRT_MAXSECTORS-1;
    int maxy = cursector->idx.y + 1; if (maxy >= SQRT_MAXSECTORS) maxy = SQRT_MAXSECTORS-1;
    
    for(cl_int sectorx = minx; sectorx <= maxx; sectorx++)
    {
        for (cl_int sectory = miny; sectory <= maxy; sectory++)
        {

            MapSector* sector = &gameState->sectors[sectorx][sectory];
            CL_CHECK_NULL(sector);

            Peep* curPeep;
            OFFSET_TO_PTR(gameState->peeps, sector->lastPeepIdx, curPeep);
            

            Peep* firstPeep = curPeep;
            while (curPeep != NULL)
            {
                if (curPeep != peep) {
                    PeepToPeepInteraction(ALL_CORE_PARAMS_PASS, peep, curPeep);

                }

                
                OFFSET_TO_PTR(gameState->peeps, curPeep->prevSectorPeepIdx, curPeep);


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
    if (!GE_VECTOR3_EQUAL(maptilecoords, maptilecoords_prev) || (gameStateActions->mapZView_1 != gameStateActions->mapZView))
    {
        if (PeepMapVisiblity(ALL_CORE_PARAMS_PASS, peep, gameStateActions->mapZView))
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

    

    if (MapTileData_TileSolid(tileData) == 1)//if curTile is solid
    {
        //revert to center of last good map position
        #ifndef PEEP_DISABLE_TILECORRECTIONS

            ge_int3 lastGoodPos;
            MapToWorld(GE_INT3_WHOLE_ONLY_Q16(peep->lastGoodPosMap_Q16), &lastGoodPos);
            lastGoodPos.x += MUL_PAD_Q16(TO_Q16(MAP_TILE_SIZE), TO_Q16(1) >> 1);
            lastGoodPos.y += MUL_PAD_Q16(TO_Q16(MAP_TILE_SIZE), TO_Q16(1) >> 1);
            lastGoodPos.z += MUL_PAD_Q16(TO_Q16(MAP_TILE_SIZE), TO_Q16(1) >> 1);


            peep->physics.base.pos_post_Q16.x += lastGoodPos.x - peep->physics.base.pos_Q16.x;
            peep->physics.base.pos_post_Q16.y += lastGoodPos.y - peep->physics.base.pos_Q16.y;
            peep->physics.base.pos_post_Q16.z += lastGoodPos.z - peep->physics.base.pos_Q16.z;


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
    if (x < 1 || x >= MAPDIM - 1 || y < 1 || y >= MAPDIM - 1)
    {
        mapTile2VBO[y * MAPDIM + x] = MapTile_NONE;
        return;
    }
    MapTile tile = MapTile_NONE;
    mapTile2VBO[y * MAPDIM + x] = MapTile_NONE;

    for (int z = gameStateActions->mapZView; z >= 1; z--)
    {       
        MapTile center = gameState->map.levels[z].data[x][y];

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
    ge_int3 coord = (ge_int3){x,y,gameStateActions->mapZView };
    cl_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, coord);
    MapTile tile = MapDataGetTile(*data);
    MapTile tileUp;
    if (gameStateActions->mapZView < MAPDEPTH-1)
    {
        coord.z = gameStateActions->mapZView + 1;
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
    finalAttr |= clamp((vz + (int)BITGET_MF(*data, MapTileFlags_LowCornerTPLEFT)*m) * 2, 0, 15);
    finalAttr |= clamp((vz + (int)BITGET_MF(*data, MapTileFlags_LowCornerTPRIGHT)*m) * 2, 0, 15) << 4;
    finalAttr |= clamp((vz + (int)BITGET_MF(*data, MapTileFlags_LowCornerBTMRIGHT)*m) * 2, 0, 15) << 8;
    finalAttr |= clamp((vz + (int)BITGET_MF(*data, MapTileFlags_LowCornerBTMLEFT)*m) * 2, 0, 15) << 12;

    mapTile1AttrVBO[ y * MAPDIM + x ] = finalAttr;
    mapTile1OtherAttrVBO[ y * MAPDIM + x ] = RandomRange(x,0,4);
    
    if (tileUp != MapTile_NONE)
    {
        mapTile1VBO[y * MAPDIM + x ] = MapTile_NONE;//view obstructed by foottile above.
    }
    else
    {
        mapTile1VBO[y * MAPDIM + x] = tile;
    }
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


void GetMapTileCoordWithViewFromWorld2D(ALL_CORE_PARAMS, ge_int2 world_Q16, ge_int3* mapcoord_whole, int* occluded, int zViewRestrictLevel)
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
        
        MapTile tile = gameState->map.levels[z].data[(*mapcoord_whole).x][(*mapcoord_whole).y];

        if (tile != MapTile_NONE)
        {
            (*mapcoord_whole).z = z;
            if (z == gameStateActions->mapZView-1)
            {
                *occluded = 1;
            }
            else if (z == gameStateActions->mapZView)
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


__kernel void game_apply_actions(ALL_CORE_PARAMS)
{
    cl_uint curPeepIdx = gameState->clientStates[gameStateActions->clientId].selectedPeepsLastIdx;
    PeepRenderSupport peepRenderSupport[MAX_PEEPS];
    while (curPeepIdx != OFFSET_NULL)
    {
        Peep* p = &gameState->peeps[curPeepIdx];

        gameState->clientStates[gameStateActions->clientId].peepRenderSupport[curPeepIdx].render_selectedByClient = 1;

        curPeepIdx = p->prevSelectionPeepIdx[gameStateActions->clientId];
    }




    //apply turns
    for (int32_t a = 0; a < gameStateActions->numActions; a++)
    {
        ClientAction* clientAction = &gameStateActions->clientActions[a].action;
        ActionTracking* actionTracking = &gameStateActions->clientActions[a].tracking;
        cl_uchar cliId = actionTracking->clientId;
        SynchronizedClientState* client = &gameState->clientStates[cliId];

        if (clientAction->actionCode == ClientActionCode_DoSelect)
        {
            client->selectedPeepsLastIdx = OFFSET_NULL;
            for (cl_uint pi = 0; pi < MAX_PEEPS; pi++)
            {
                Peep* p = &gameState->peeps[pi];

                if (p->stateBasic.faction == actionTracking->clientId)
                    if ((p->physics.base.pos_Q16.x > clientAction->intParameters[CAC_DoSelect_Param_StartX_Q16])
                        && (p->physics.base.pos_Q16.x < clientAction->intParameters[CAC_DoSelect_Param_EndX_Q16]))
                    {

                        if ((p->physics.base.pos_Q16.y < clientAction->intParameters[CAC_DoSelect_Param_StartY_Q16])
                            && (p->physics.base.pos_Q16.y > clientAction->intParameters[CAC_DoSelect_Param_EndY_Q16]))
                        {
                            
                            if (PeepMapVisiblity(ALL_CORE_PARAMS_PASS, p, clientAction->intParameters[CAC_DoSelect_Param_ZMapView]))
                            {

                                if (client->selectedPeepsLastIdx != OFFSET_NULL)
                                {
                                    gameState->peeps[client->selectedPeepsLastIdx].nextSelectionPeepIdx[cliId] = pi;
                                    p->prevSelectionPeepIdx[cliId] = client->selectedPeepsLastIdx;
                                    p->nextSelectionPeepIdx[cliId] = OFFSET_NULL;
                                }
                                else
                                {
                                    p->prevSelectionPeepIdx[cliId] = OFFSET_NULL;
                                    p->nextSelectionPeepIdx[cliId] = OFFSET_NULL;
                                }
                                client->selectedPeepsLastIdx = pi;

                                PrintSelectionPeepStats(ALL_CORE_PARAMS_PASS, p);
                            }

                        }
                    }
            }
        }
        else if (clientAction->actionCode == ClientActionCode_CommandToLocation)
        {
            
            cl_uint curPeepIdx = client->selectedPeepsLastIdx;
            ge_int3 mapcoord;
            ge_int2 world2D;
            cl_uchar pathFindSuccess;
            AStarPathNode* path;
            if (curPeepIdx != OFFSET_NULL)
            {
                //Do an AStarSearch
                Peep* curPeep = &gameState->peeps[curPeepIdx];
                world2D.x = clientAction->intParameters[CAC_CommandToLocation_Param_X_Q16];
                world2D.y = clientAction->intParameters[CAC_CommandToLocation_Param_Y_Q16];
                int occluded;

                GetMapTileCoordWithViewFromWorld2D(ALL_CORE_PARAMS_PASS, world2D, &mapcoord, &occluded, gameStateActions->mapZView);
                AStarSearchInstantiate(&gameState->mapSearchers[0]);
                ge_int3 start = GE_INT3_WHOLE_Q16(curPeep->posMap_Q16);
                mapcoord.z++;
                ge_int3 end = mapcoord;
                Print_GE_INT3(start);
                Print_GE_INT3(end);
                pathFindSuccess = AStarSearchRoutine(ALL_CORE_PARAMS_PASS, &gameState->mapSearchers[0], start, end, CL_INTMAX);
                if (pathFindSuccess != 0)
                {
                    printf("--------------------------\n");
                    AStarPrintSearchPathFrom(&gameState->mapSearchers[0], start);
                    printf("--------------------------\n");
                    AStarPrintSearchPathTo(&gameState->mapSearchers[0], end);
                    printf("--------------------------\n");


                    path = AStarFormPathSteps(ALL_CORE_PARAMS_PASS , &gameState->mapSearchers[0], &gameState->paths);
                    CL_CHECK_NULL(path);
                    AStarPrintPath(path);
                    printf("--------------------------\n");
                }
            }

            while (curPeepIdx != OFFSET_NULL)
            {
                Peep* curPeep = &gameState->peeps[curPeepIdx];

                if (pathFindSuccess != 0)
                {
                    curPeep->physics.drive.next = path;

                    ge_int3 worldloc;
                    MapToWorld(curPeep->physics.drive.next->mapCoord_Q16, &worldloc);
                    curPeep->physics.drive.target_x_Q16 = worldloc.x;
                    curPeep->physics.drive.target_y_Q16 = worldloc.y;
                    curPeep->physics.drive.target_z_Q16 = worldloc.z;
                    curPeep->physics.drive.drivingToTarget = 1;

                    //restrict comms to new channel
                    curPeep->comms.orders_channel = RandomRange(client->selectedPeepsLastIdx, 0, 10000);
                    curPeep->comms.message_TargetReached = 0;
                    curPeep->comms.message_TargetReached_pending = 0;
                }
                curPeepIdx = curPeep->prevSelectionPeepIdx[cliId];
            }
        }
        else if (clientAction->actionCode == ClientActionCode_CommandTileDelete)
        {
            printf("Got the command\n");

            ge_int2 world2DMouse;
            world2DMouse.x = clientAction->intParameters[CAC_CommandTileDelete_Param_X_Q16];
            world2DMouse.y = clientAction->intParameters[CAC_CommandTileDelete_Param_Y_Q16];

            ge_int3 mapCoord;
            int occluded;
            GetMapTileCoordWithViewFromWorld2D(ALL_CORE_PARAMS_PASS, world2DMouse, &mapCoord, &occluded, gameStateActions->mapZView+1);
            if (mapCoord.z > 0) 
            {
                gameState->map.levels[mapCoord.z].data[mapCoord.x][mapCoord.y] = MapTile_NONE;

                MapBuildTileView(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y);

                MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y);
                MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x + 1, mapCoord.y);
                MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x - 1, mapCoord.y);
                MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y + 1);
                MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y - 1);
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
    GetMapTileCoordWithViewFromWorld2D(ALL_CORE_PARAMS_PASS, world2d_Q16,
        &mapCoordWhole, &occluded, MAPDEPTH - 1);

    cl_uint* tileData = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoordWhole);
    //do 3x3 kernel test


    //offsets[22] = (ge_int3){ 1, 1, 0 };
    //offsets[23] = (ge_int3){ -1, 1, 0 };
    //offsets[24] = (ge_int3){ 1, -1, 0 };
    //offsets[25] = (ge_int3){ -1, -1, 0 };

    cl_uint* data22 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE_INT3_ADD(mapCoordWhole, staticData->directionalOffsets[22]));
    MapTile tile22 = MapDataGetTile(*data22);
    if (tile22 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerBTMRIGHT);
    }
    cl_uint* data24 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE_INT3_ADD(mapCoordWhole, staticData->directionalOffsets[24]));
    MapTile tile24 = MapDataGetTile(*data24);
    if (tile24 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerTPRIGHT);
    }
    cl_uint* data23 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE_INT3_ADD(mapCoordWhole, staticData->directionalOffsets[23]));
    MapTile tile23 = MapDataGetTile(*data23);
    if (tile23 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerBTMLEFT);
    }
    cl_uint* data25 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE_INT3_ADD(mapCoordWhole, staticData->directionalOffsets[25]));
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



    cl_uint* data0 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE_INT3_ADD(mapCoordWhole, staticData->directionalOffsets[0]));
    MapTile tile0 = MapDataGetTile(*data0);
    if (tile0 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerBTMRIGHT);
        BITSET(*tileData, MapTileFlags_LowCornerTPRIGHT);
    }
    cl_uint* data1 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE_INT3_ADD(mapCoordWhole, staticData->directionalOffsets[1]));
    MapTile tile1 = MapDataGetTile(*data1);
    if (tile1 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerBTMLEFT);
        BITSET(*tileData, MapTileFlags_LowCornerTPLEFT);
    }
    cl_uint* data2 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE_INT3_ADD(mapCoordWhole, staticData->directionalOffsets[2]));
    MapTile tile2 = MapDataGetTile(*data2);
    if (tile2 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerTPRIGHT);
        BITSET(*tileData, MapTileFlags_LowCornerTPLEFT);
    }
    cl_uint* data3 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE_INT3_ADD(mapCoordWhole, staticData->directionalOffsets[3]));
    MapTile tile3 = MapDataGetTile(*data3);
    if (tile3 == MapTile_NONE)
    {
        BITSET(*tileData, MapTileFlags_LowCornerBTMRIGHT);
        BITSET(*tileData, MapTileFlags_LowCornerBTMLEFT);
    }




    

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



                gameState->map.levels[z].data[x][y] = tileType;


                i++;
            }



            MapCreateSlope(ALL_CORE_PARAMS_PASS, x, y);
            MapBuildTileView(ALL_CORE_PARAMS_PASS,x,y);
            MapUpdateShadow(ALL_CORE_PARAMS_PASS, x, y);

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
  if(0){
  printf("Speed Tests:\n");

    int s = 0;
    for (cl_ulong i = 0; i < 0; i++)
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

__kernel void game_init_single(ALL_CORE_PARAMS)
{
    printf("Game Initializing...\n");




    printf("Initializing StaticData Buffer..\n");
    MakeCardinalDirectionOffsets(&staticData->directionalOffsets[0]);

    StartupTests();


    gameState->numClients = 1;
    gameStateActions->pauseState = 0;
    gameStateActions->mapZView = MAPDEPTH-1;
    gameStateActions->mapZView_1 = 0;

    for (int secx = 0; secx < SQRT_MAXSECTORS; secx++)
    {
        for (int secy = 0; secy < SQRT_MAXSECTORS; secy++)
        {
            gameState->sectors[secx][secy].idx.x = secx;
            gameState->sectors[secx][secy].idx.y = secy;
            gameState->sectors[secx][secy].lastPeepIdx = OFFSET_NULL;
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

    const int spread = 500;
    for (cl_uint p = 0; p < MAX_PEEPS; p++)
    {
        gameState->peeps[p].Idx = p;
        gameState->peeps[p].physics.base.pos_Q16.x = RandomRange(p, -spread << 16, spread << 16);
        gameState->peeps[p].physics.base.pos_Q16.y = RandomRange(p + 1, -spread << 16, spread << 16);


        ge_int3 mapcoord;
        ge_int2 world2D;
        world2D.x = gameState->peeps[p].physics.base.pos_Q16.x;
        world2D.y = gameState->peeps[p].physics.base.pos_Q16.y;
        int occluded;

        GetMapTileCoordWithViewFromWorld2D(ALL_CORE_PARAMS_PASS, world2D, &mapcoord, &occluded, MAPDEPTH - 1);
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

        gameState->peeps[p].physics.base.v_Q16 = (ge_int3){ 0,0,0 };
        gameState->peeps[p].physics.base.vel_add_Q16 = (ge_int3){ 0,0,0 };
        gameState->peeps[p].physics.base.pos_post_Q16 = (ge_int3){ 0,0,0 };


        gameState->peeps[p].minDistPeepIdx = OFFSET_NULL;
        gameState->peeps[p].minDistPeep_Q16 = (1 << 30);
        gameState->peeps[p].mapSectorIdx = GE_OFFSET_NULL_2D;
        gameState->peeps[p].mapSector_pendingIdx = GE_OFFSET_NULL_2D;
        gameState->peeps[p].nextSectorPeepIdx = OFFSET_NULL;
        gameState->peeps[p].prevSectorPeepIdx = OFFSET_NULL;
        gameState->peeps[p].physics.drive.target_x_Q16 = gameState->peeps[p].physics.base.pos_Q16.x;
        gameState->peeps[p].physics.drive.target_y_Q16 = gameState->peeps[p].physics.base.pos_Q16.y;
        gameState->peeps[p].physics.drive.drivingToTarget = 0;

        if (gameState->peeps[p].physics.base.pos_Q16.x > 0)
            gameState->peeps[p].stateBasic.faction = 0;
        else
            gameState->peeps[p].stateBasic.faction = 1;


        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            gameState->clientStates[i].selectedPeepsLastIdx = OFFSET_NULL;


            CL_CHECKED_ARRAY_SET(gameState->peeps[p].nextSelectionPeepIdx, MAX_CLIENTS, i, OFFSET_NULL)
                CL_CHECKED_ARRAY_SET(gameState->peeps[p].prevSelectionPeepIdx, MAX_CLIENTS, i, OFFSET_NULL)
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

    if (gameState->clientStates[gameStateActions->clientId].peepRenderSupport[peep->Idx].render_selectedByClient)
    {
        brightFactor = 1.0f;
        gameState->clientStates[gameStateActions->clientId].peepRenderSupport[peep->Idx].render_selectedByClient = 0;
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


    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 0] = drawPosX;
    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 1] = drawPosY;

    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 2] = drawColor.x * brightFactor;
    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 3] = drawColor.y * brightFactor;
    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 4] = drawColor.z * brightFactor;

    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 5] = peep->physics.base.CS_angle_rad;
}

void ParticleDraw(ALL_CORE_PARAMS, Particle* particle, cl_uint idx)
{
    float3 drawColor;
    float drawPosX = (float)((float)particle->pos.x.number / (1 << particle->pos.x.q));
    float drawPosY = (float)((float)particle->pos.y.number / (1 << particle->pos.y.q));


    drawColor.x = 1.0f;
    drawColor.y = 1.0f;
    drawColor.z = 1.0f;
  

    float brightFactor = 1.0f;

    particleVBOBuffer[idx * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 0] = drawPosX;
    particleVBOBuffer[idx * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 1] = drawPosY;

    particleVBOBuffer[idx * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 2] = drawColor.x * brightFactor;
    particleVBOBuffer[idx * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 3] = drawColor.y * brightFactor;
    particleVBOBuffer[idx * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 4] = drawColor.z * brightFactor;

    particleVBOBuffer[idx * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 5] = 0;
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
    if (gameStateActions->mapZView != gameStateActions->mapZView_1)
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


__kernel void game_post_update_single( ALL_CORE_PARAMS)
{

    gameStateActions->mapZView_1 = gameStateActions->mapZView;


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
        if (pi + globalid * chunkSize < MAX_PEEPS) {
            Peep* p;
            
            CL_CHECKED_ARRAY_GET_PTR(gameState->peeps, MAX_PEEPS, pi + globalid * chunkSize, p)
                CL_CHECK_NULL(p)
                
                PeepPreUpdate1(p);

        }
    }

    barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
    


    for (cl_ulong pi = 0; pi < chunkSize; pi++)
    {
        if (pi + globalid * chunkSize < MAX_PEEPS)
        {
            Peep* p;
            CL_CHECKED_ARRAY_GET_PTR(gameState->peeps, MAX_PEEPS, pi + globalid * chunkSize, p)
                CL_CHECK_NULL(p)

                PeepPreUpdate2(p);



            global volatile MapSector* mapSector;
            OFFSET_TO_PTR_2D(gameState->sectors, p->mapSectorIdx, mapSector);
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
            OFFSET_TO_PTR_2D(gameState->sectors, p->mapSector_pendingIdx, mapSector);


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

}