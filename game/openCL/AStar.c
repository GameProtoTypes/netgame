
#include "common.h"


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