#include "cl_type_glue.h"
#include "fixedpoint_opencl.h"

#include "peep.h"
#include "randomcl.h"
#include "perlincl.h"

#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable



void PeepPrint(Peep* peep)
{
    PrintQ16(peep->stateSim.target_x_Q16);

}


void RobotInteraction(Peep* peep, Peep* otherPeep)
{
    if (peep->stateRender.deathState || otherPeep->stateRender.deathState)
        return;

    cl_int dist_Q16 = distance_s2_Q16(peep->stateRender.map_x_Q15_16, peep->stateRender.map_y_Q15_16,
        otherPeep->stateRender.map_x_Q15_16, otherPeep->stateRender.map_y_Q15_16);

    if (dist_Q16 < peep->minDistPeep_Q16)
    {
        peep->minDistPeep_Q16 = dist_Q16;
        peep->minDistPeep = otherPeep;
    }
    
}




void RobotUpdate(Peep* peep)
{
    cl_int deltax_Q16 = peep->stateSim.target_x_Q16 - peep->stateRender.map_x_Q15_16;
    cl_int deltay_Q16 = peep->stateSim.target_y_Q16 - peep->stateRender.map_y_Q15_16;

    cl_int len_Q16;
    normalize_s2_Q16(&deltax_Q16, &deltay_Q16, &len_Q16);
    
    
    peep->stateRender.attackState = 0;
    if (peep->stateRender.deathState == 1)
    {
        return;
    }
    if ((peep->minDistPeep_Q16 >> 16) < 5)
    {
        deltax_Q16 = peep->minDistPeep->stateRender.map_x_Q15_16 - peep->stateRender.map_x_Q15_16;
        deltay_Q16 = peep->minDistPeep->stateRender.map_y_Q15_16 - peep->stateRender.map_y_Q15_16;
        cl_int len_Q16;
        normalize_s2_Q16(&deltax_Q16, &deltay_Q16, &len_Q16);
        peep->stateSim.xv_Q15_16 = -deltax_Q16/8;
        peep->stateSim.yv_Q15_16 = -deltay_Q16/8;

        if (peep->minDistPeep->stateRender.faction != peep->stateRender.faction && (peep->minDistPeep->stateRender.deathState != 1))
        {
            peep->stateRender.attackState = 1;
            peep->stateRender.health -= RandomRange(peep->minDistPeep->stateRender.map_x_Q15_16, 0, 5);

            if (peep->stateRender.health <= 0)
            {
                peep->stateRender.attackState = 0;
                peep->stateSim.xv_Q15_16 = 0;
                peep->stateSim.yv_Q15_16 = 0;
            }
        }
    }
    else
    {
        peep->stateRender.attackState = 0;
        if ((len_Q16 >> 16) < 2)
        {
            peep->stateSim.xv_Q15_16 = 0;
            peep->stateSim.yv_Q15_16 = 0;
        }
        else
        {
            peep->stateSim.xv_Q15_16 = deltax_Q16;
            peep->stateSim.yv_Q15_16 = deltay_Q16;
        }
    }
}


void AssignPeepToSector_Detach(GameState* gameState, Peep* peep)
{
    cl_int x = ((peep->stateRender.map_x_Q15_16 >> 16) / (SECTOR_SIZE));
    cl_int y = ((peep->stateRender.map_y_Q15_16 >> 16) / (SECTOR_SIZE));

    //clamp
    if (x < -SQRT_MAXSECTORS / 2)
        x = -SQRT_MAXSECTORS / 2;
    if (y < -SQRT_MAXSECTORS / 2)
        y = -SQRT_MAXSECTORS / 2;
    if (x >= SQRT_MAXSECTORS / 2)
        x = SQRT_MAXSECTORS / 2 - 1;
    if (y >= SQRT_MAXSECTORS / 2)
        y = SQRT_MAXSECTORS / 2 - 1;

    MapSector* newSector = &gameState->sectors[x + SQRT_MAXSECTORS / 2][y + SQRT_MAXSECTORS / 2];
    CL_CHECK_NULL(newSector)
    if ((peep->mapSector != newSector))
    {
        if (peep->mapSector != NULL)
        {
            //remove peep from old sector
            if ((peep->prevSectorPeep != NULL))
            {
                peep->prevSectorPeep->nextSectorPeep = peep->nextSectorPeep;
            }
            if ((peep->nextSectorPeep != NULL))
            {
                peep->nextSectorPeep->prevSectorPeep = peep->prevSectorPeep;
            }


            if (peep->mapSector->lastPeep == peep) {
                peep->mapSector->lastPeep = peep->prevSectorPeep;
                if (peep->mapSector->lastPeep != NULL)
                    peep->mapSector->lastPeep->nextSectorPeep = NULL;
            }

            //completely detach
            peep->nextSectorPeep = NULL;
            peep->prevSectorPeep = NULL;

        }

        //assign new sector for next stage
        peep->mapSector_pending = newSector;
    }
}
void AssignPeepToSector_Insert(GameState* gameState, Peep* peep)
{
    //assign new sector
    if (peep->mapSector != peep->mapSector_pending)
    {
        peep->mapSector = peep->mapSector_pending;

        //put peep in the sector.  extend list
        if (peep->mapSector->lastPeep != NULL)
        {
            peep->mapSector->lastPeep->nextSectorPeep = peep;
            peep->prevSectorPeep = peep->mapSector->lastPeep;
        }
        peep->mapSector->lastPeep = peep;
    }
}



void PeepPreUpdate(Peep* peep)
{
    peep->stateRender.map_x_Q15_16 += peep->stateSim.xv_Q15_16;
    peep->stateRender.map_y_Q15_16 += peep->stateSim.yv_Q15_16;

    peep->stateSim.netForcex_Q16 = 0;
    peep->stateSim.netForcey_Q16 = 0;

    if (peep->stateRender.health <= 0)
        peep->stateRender.deathState = 1;
}

void PeepUpdate(__global GameState* gameState, Peep* peep)
{

    peep->minDistPeep_Q16 = (1 << 30);
    peep->minDistPeep = NULL;


    //traverse sector
    int minx = peep->mapSector->xidx - 1; if (minx < 0) minx = 0;
    int miny = peep->mapSector->yidx - 1; if (miny < 0) miny = 0;

    int maxx = peep->mapSector->xidx + 1; if (maxx >= SQRT_MAXSECTORS) maxx = SQRT_MAXSECTORS-1;
    int maxy = peep->mapSector->yidx + 1; if (maxy >= SQRT_MAXSECTORS) maxy = SQRT_MAXSECTORS-1;
    
    for(cl_int sectorx = minx; sectorx <= maxx; sectorx++)
    {
        for (cl_int sectory = miny; sectory <= maxy; sectory++)
        {

            MapSector* sector = &gameState->sectors[sectorx][sectory];
            
            Peep* curPeep = sector->lastPeep;
            Peep* firstPeep = curPeep;
            while (curPeep != NULL)
            {
                if (curPeep != peep) {
                    RobotInteraction(peep, curPeep);
                }

                curPeep = curPeep->prevSectorPeep;
                if (curPeep == firstPeep)
                    break;
            }

        }
    }
    RobotUpdate(peep);
 

}
__kernel void game_apply_actions(__global GameState* gameState, __global GameStateB* gameStateB)
{

    cl_uint curPeepIdx = gameState->clientStates[gameStateB->clientId].selectedPeepsLastIdx;
    PeepRenderSupport peepRenderSupport[MAX_PEEPS];
    while (curPeepIdx != OFFSET_NULL)
    {
        Peep* p = &gameState->peeps[curPeepIdx];

        gameState->clientStates[gameStateB->clientId].peepRenderSupport[curPeepIdx].render_selectedByClient = 1;

        curPeepIdx = p->prevSelectionPeepIdx[gameStateB->clientId];
    }




    //apply turns
    for (int32_t a = 0; a < gameStateB->numActions; a++)
    {
        ClientAction* clientAction = &gameStateB->clientActions[a].action;
        ActionTracking* actionTracking = &gameStateB->clientActions[a].tracking;
        cl_uchar cliId = actionTracking->clientId;
        SynchronizedClientState* client = &gameState->clientStates[cliId];

        if (clientAction->action_DoSelect)
        {
            client->selectedPeepsLastIdx = OFFSET_NULL;
            for (cl_uint pi = 0; pi < MAX_PEEPS; pi++)
            {
                Peep* p = &gameState->peeps[pi];

                if (p->stateRender.faction == actionTracking->clientId)
                    if ((p->stateRender.map_x_Q15_16 > clientAction->params_DoSelect_StartX_Q16)
                        && (p->stateRender.map_x_Q15_16 < clientAction->params_DoSelect_EndX_Q16))
                    {

                        if ((p->stateRender.map_y_Q15_16 < clientAction->params_DoSelect_StartY_Q16)
                            && (p->stateRender.map_y_Q15_16 > clientAction->params_DoSelect_EndY_Q16))
                        {
                            PrintQ16(p->Idx);

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
                        }
                    }
            }
        }
        else if (clientAction->action_CommandToLocation)
        {
            cl_uint curPeepIdx = client->selectedPeepsLastIdx;
            while (curPeepIdx != OFFSET_NULL)
            {
                Peep* curPeep = &gameState->peeps[curPeepIdx];
                curPeep->stateSim.target_x_Q16 = clientAction->params_CommandToLocation_X_Q16;
                curPeep->stateSim.target_y_Q16 = clientAction->params_CommandToLocation_Y_Q16;

                curPeepIdx = curPeep->prevSelectionPeepIdx[cliId];
            }


        }
    }

    gameStateB->numActions = 0;
}


__kernel void game_init_single(__global GameState* gameState, 
    __global GameStateB* gameStateB,
    __global cl_uint* mapTileVBO)
{
    printf("Game Initializing...\n");


    fixedPointTests();



    gameState->numClients = 1;
    gameStateB->pauseState = 0;


    for (int secx = 0; secx < SQRT_MAXSECTORS; secx++)
    {
        for (int secy = 0; secy < SQRT_MAXSECTORS; secy++)
        {
            gameState->sectors[secx][secy].xidx = secx;
            gameState->sectors[secx][secy].yidx = secy;
            gameState->sectors[secx][secy].lastPeep = NULL;
            gameState->sectors[secx][secy].lock = 0;
        }
    }
    printf("Sectors Initialized.\n");
    for (cl_uint p = 0; p < MAX_PEEPS; p++)
    {
        gameState->peeps[p].Idx = p;
        gameState->peeps[p].stateRender.map_x_Q15_16 = RandomRange(p,-1000 << 16, 1000 << 16) ;
        gameState->peeps[p].stateRender.map_y_Q15_16 = RandomRange(p+1,-1000 << 16, 1000 << 16) ;
        gameState->peeps[p].stateRender.attackState = 0;
        gameState->peeps[p].stateRender.health = 10;
        gameState->peeps[p].stateRender.deathState = 0;

        gameState->peeps[p].stateSim.xv_Q15_16 = RandomRange(p,-4, 4) << 16;
        gameState->peeps[p].stateSim.yv_Q15_16 = RandomRange(p+1,-4, 4) << 16;
        gameState->peeps[p].stateSim.netForcex_Q16 = 0;
        gameState->peeps[p].stateSim.netForcey_Q16 = 0;
        gameState->peeps[p].minDistPeep = NULL;
        gameState->peeps[p].minDistPeep_Q16 = (1 << 30);
        gameState->peeps[p].mapSector = NULL;
        gameState->peeps[p].mapSector_pending = NULL;
        gameState->peeps[p].nextSectorPeep = NULL;
        gameState->peeps[p].prevSectorPeep = NULL;
        gameState->peeps[p].stateSim.target_x_Q16 = RandomRange(p,-1000 << 16, 1000 << 16);
        gameState->peeps[p].stateSim.target_y_Q16 = RandomRange(p+1,-1000 << 16, 1000 << 16);



        gameState->peeps[p].stateRender.faction = RandomRange(p + 3, 0, 2);
        



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

        AssignPeepToSector_Detach(gameState, p);
        AssignPeepToSector_Insert(gameState, p);

    }

    printf("Peep Sector Assigment Finished\n");






    printf("Creating Map..\n");
    
    int i = 0;
    for (int x = 0; x < SQRT_MAPTILESIZE; x++)
    {
        for (int y = 0; y < SQRT_MAPTILESIZE; y++)
        {


           // cl_int z_Q16 = noise_2d_Q16(TO_Q16(x), TO_Q16(y),0);
            cl_int z_Q16 = perlin_2d_Q16(TO_Q16(x), TO_Q16(y), TO_Q16(1)>>4, 4, 0);
            
            int tileIdx = 0;
            if (z_Q16 * 100 < TO_Q16(20))
                tileIdx = 0;
            else if (z_Q16 * 100 < TO_Q16(40))
                tileIdx = 1;
            else if (z_Q16 * 100 < TO_Q16(60))
                tileIdx = 2;
            else if (z_Q16 * 100 < TO_Q16(80))
                tileIdx = 3;
            else
                tileIdx = 4;

            gameState->map.levels[0];

            mapTileVBO[y * SQRT_MAPTILESIZE + x % SQRT_MAPTILESIZE] = tileIdx;
            i++;
        }
    }



}


void PeepDraw(GameState* gameState, GameStateB* gameStateB, Peep* peep, __global float* peepVBOBuffer)
{
    float3 drawColor;

    float brightFactor = 0.6f;
    if (peep->stateRender.faction == 0)
    {
        drawColor.x = 0.0f;
        drawColor.y = 1.0f;
        drawColor.z = 1.0f;
    }
    else if(peep->stateRender.faction == 1)
    {
        drawColor.x = 1.0f;
        drawColor.y = 1.0f;
        drawColor.z = 0.0f;
    }
    else if (peep->stateRender.faction == 2)
    {
        drawColor.x = 0.0f;
        drawColor.y = 1.0f;
        drawColor.z = 0.0f;
    }
    else if (peep->stateRender.faction == 3)
    {
        drawColor.x = 1.0f;
        drawColor.y = 0.0f;
        drawColor.z = 1.0f;
    }

    if (gameState->clientStates[gameStateB->clientId].peepRenderSupport[peep->Idx].render_selectedByClient)
    {
        brightFactor = 1.0f;
        gameState->clientStates[gameStateB->clientId].peepRenderSupport[peep->Idx].render_selectedByClient = 0;
    }
    if (peep->stateRender.deathState == 1)
    {
        brightFactor = 0.6f;
        drawColor.x = 0.5f;
        drawColor.y = 0.5f;
        drawColor.z = 0.5f;
    }
    if (peep->stateRender.attackState == 1)
    {
        brightFactor = 1.0f;
        drawColor.x = 1.0f;
        drawColor.y = 1.0f;
        drawColor.z = 1.0f;
    }



    peepVBOBuffer[peep->Idx * PEEP_VBO_INSTANCE_SIZE/sizeof(float) + 0] = (float)((float)peep->stateRender.map_x_Q15_16 / (1 << 16));
    peepVBOBuffer[peep->Idx * PEEP_VBO_INSTANCE_SIZE / sizeof(float) + 1] = (float)((float)peep->stateRender.map_y_Q15_16 / (1 << 16));

    peepVBOBuffer[peep->Idx * PEEP_VBO_INSTANCE_SIZE / sizeof(float) + 2] = drawColor.x* brightFactor;
    peepVBOBuffer[peep->Idx * PEEP_VBO_INSTANCE_SIZE / sizeof(float) + 3] = drawColor.y * brightFactor;
    peepVBOBuffer[peep->Idx * PEEP_VBO_INSTANCE_SIZE / sizeof(float) + 4] = drawColor.z * brightFactor;
}

__kernel void game_update(__global GameState* gameState,
    __global GameStateB* gameStateB, 
    __global float* peepVBOBuffer,
    __global cl_uint* mapTileVBO
    
    )
{
    // Get the index of the current element to be processed
    int globalid = get_global_id(0);
    int localid = get_local_id(0);

    
    Peep* p = &gameState->peeps[globalid];
    PeepUpdate(gameState, p);
    PeepDraw(gameState, gameStateB, p, peepVBOBuffer);


}


__kernel void game_preupdate_1(__global const GameState* gameState) {
    // Get the index of the current element to be processed
    
    int globalid = get_global_id(0);
    if (globalid >= WARPSIZE)
        return;
   
    const cl_uint chunkSize = MAX_PEEPS / WARPSIZE;
    for (cl_ulong pi = 0; pi < MAX_PEEPS / WARPSIZE; pi++)
    {

        //Peep* p = &gameState->peeps[pi + globalid*chunkSize];
        Peep* p;
        CL_CHECKED_ARRAY_GET_PTR(gameState->peeps, MAX_PEEPS, pi + globalid * chunkSize, p)
        PeepPreUpdate(p);

        global volatile MapSector* mapSector = (global volatile MapSector *)p->mapSector;
        CL_CHECK_NULL(mapSector)
        global volatile cl_uint* lock = (global volatile cl_uint*)&mapSector->lock;
        CL_CHECK_NULL(lock)

        cl_uint reservation;

        reservation = atomic_add(lock, 1)+1;
        barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);

        while (*lock != reservation) { }

        AssignPeepToSector_Detach(gameState, p);

        atomic_dec(lock);
       
    }
}


__kernel void game_preupdate_2(__global  GameState* gameState) {
   
    // Get the index of the current element to be processed
    int globalid = get_global_id(0);
    if (globalid >= WARPSIZE)
        return;

   // printf("Preupdate2");
    const cl_uint chunkSize = MAX_PEEPS / WARPSIZE;
    for (cl_ulong pi = 0; pi < MAX_PEEPS / WARPSIZE; pi++)
    {
        Peep* p = &gameState->peeps[pi + globalid * chunkSize];

        global volatile MapSector* mapSector = (global volatile MapSector*)p->mapSector_pending;
        CL_CHECK_NULL(mapSector)
        if (mapSector == NULL)
            continue;
        global volatile cl_uint* lock = (global volatile cl_uint*) & mapSector->lock;
        CL_CHECK_NULL(lock)


        int reservation = atomic_add(lock, 1) + 1;
        
        barrier(CLK_GLOBAL_MEM_FENCE);

        while (atomic_add(lock, 0) != reservation) {}

            AssignPeepToSector_Insert(gameState, p);
        
        atomic_dec(lock);
    }








}
