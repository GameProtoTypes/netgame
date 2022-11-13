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

    return search->state;
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
cl_int AStarNodeDistanceHuristic_IDA(PARAM_GLOBAL_POINTER AStarSearch_IDA* search, ge_short3 locA, ge_short3 locB)
{
    return TO_Q16(abs(locA.x - locB.x) + abs(locA.y - locB.y) + abs(locA.z - locB.z));
}