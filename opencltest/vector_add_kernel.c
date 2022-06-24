#include "cl_type_glue.h"
#include "fixedpoint_opencl.h"

#include "peep.h"
#include "random.h"


#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

void RobotInteraction(Peep* peep, Peep* otherPeep)
{
    cl_int dist_Q16 = length_Q16(peep->map_x_Q15_16, peep->map_y_Q15_16, otherPeep->map_x_Q15_16, otherPeep->map_y_Q15_16);

    if (dist_Q16 < peep->minDistPeep_Q16)
    {
        peep->minDistPeep_Q16 = dist_Q16;
        peep->minDistPeep = otherPeep;
    }
}



void ForceUpdate(Peep* peep)
{
    peep->netForcex_Q16 += -peep->xv_Q15_16 >> 5;//drag
    peep->netForcey_Q16 += -peep->yv_Q15_16 >> 5;
    peep->xv_Q15_16 += peep->netForcex_Q16;
    peep->yv_Q15_16 += peep->netForcey_Q16;
}
void RobotUpdate(Peep* peep)
{
    cl_int deltax_Q16 = peep->target_x_Q16 - peep->map_x_Q15_16;
    cl_int deltay_Q16 = peep->target_y_Q16 - peep->map_y_Q15_16;

    normalize_Q16(&deltax_Q16, &deltay_Q16);


    if ((peep->minDistPeep_Q16 >> 16) < 5)
    {
        deltax_Q16 = peep->minDistPeep->map_x_Q15_16 - peep->map_x_Q15_16;
        deltay_Q16 = peep->minDistPeep->map_y_Q15_16 - peep->map_y_Q15_16;
        normalize_Q16(&deltax_Q16, &deltay_Q16);
        peep->xv_Q15_16 = -deltax_Q16/8;
        peep->yv_Q15_16 = -deltay_Q16/8;
    }
    else
    {
        peep->xv_Q15_16 = deltax_Q16;
        peep->yv_Q15_16 = deltay_Q16;
    }


    


}

void PeepUpdateGravity(__global GameState* gameState, Peep* peep)
{

    for (int p = 0; p < MAX_PEEPS; p++)
    {
        Peep* otherPeep = &gameState->peeps[p];
        if (otherPeep != peep) {
            ForceInteraction(peep, otherPeep);
        }

    }
}





void AssignPeepToSector_Detach(GameState* gameState, Peep* peep)
{
    cl_int x = ((peep->map_x_Q15_16 >> 16) / (SECTOR_SIZE));
    cl_int y = ((peep->map_y_Q15_16 >> 16) / (SECTOR_SIZE));

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



__kernel void game_init_single(__global GameState* gameState) 
{
    printf("Game Initializing...\n");

    gameState->numClients = 1;





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
        gameState->peeps[p].map_x_Q15_16 = RandomRange(p,-1000, 1000) << 16;
        gameState->peeps[p].map_y_Q15_16 = RandomRange(p+1,-1000, 1000) << 16;

        gameState->peeps[p].xv_Q15_16 = RandomRange(p,-4, 4) << 16;
        gameState->peeps[p].yv_Q15_16 = RandomRange(p+1,-4, 4) << 16;
        gameState->peeps[p].netForcex_Q16 = 0;
        gameState->peeps[p].netForcey_Q16 = 0;
        gameState->peeps[p].minDistPeep = NULL;
        gameState->peeps[p].minDistPeep_Q16 = (1 << 31);
        gameState->peeps[p].mapSector = NULL;
        gameState->peeps[p].mapSector_pending = NULL;
        gameState->peeps[p].nextSectorPeep = NULL;
        gameState->peeps[p].prevSectorPeep = NULL;
        gameState->peeps[p].target_x_Q16 = RandomRange(p,-1000, 1000) << 16;
        gameState->peeps[p].target_y_Q16 = RandomRange(p+1,-1000, 1000) << 16;
        if (gameState->peeps[p].map_x_Q15_16 >> 16 < 0)
        {
            gameState->peeps[p].faction = 0;
        }
        else
        {
            gameState->peeps[p].faction = 1;
        }


        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            gameState->clientStates[i].mousescroll = 0;
            gameState->clientStates[i].selectedPeepsLastIdx = OFFSET_NULL;



            gameState->peeps[p].nextSelectionPeepIdx[i] = OFFSET_NULL;
            gameState->peeps[p].prevSelectionPeepIdx[i] = OFFSET_NULL;
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
}




__kernel void game_update(__global const GameState* gameState) {
    // Get the index of the current element to be processed
    int globalid = get_global_id(0);
    int localid = get_local_id(0);

    
    PeepUpdate(gameState, &gameState->peeps[globalid]);
}


__kernel void game_preupdate_1(__global const GameState* gameState) {
    // Get the index of the current element to be processed
    
    int globalid = get_global_id(0);
    if (globalid >= WARPSIZE)
        return;
   
    const cl_uint chunkSize = MAX_PEEPS / WARPSIZE;
    for (cl_uint pi = 0; pi < MAX_PEEPS / WARPSIZE; pi++)
    {
        Peep* p = &gameState->peeps[pi + globalid*chunkSize];
        p->map_x_Q15_16 += p->xv_Q15_16;
        p->map_y_Q15_16 += p->yv_Q15_16;

        p->netForcex_Q16 = 0;
        p->netForcey_Q16 = 0;

        global volatile MapSector* mapSector = (global volatile MapSector *)p->mapSector;
        global volatile cl_uint* lock = (global volatile cl_uint*)&mapSector->lock;
        

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
    for (cl_uint pi = 0; pi < MAX_PEEPS / WARPSIZE; pi++)
    {
        Peep* p = &gameState->peeps[pi + globalid * chunkSize];

        global volatile MapSector* mapSector = (global volatile MapSector*)p->mapSector_pending;
        if (mapSector == NULL)
            continue;
        global volatile cl_uint* lock = (global volatile cl_uint*) & mapSector->lock;


        int reservation = atomic_add(lock, 1) + 1;
        
        barrier(CLK_GLOBAL_MEM_FENCE);

        while (atomic_add(lock, 0) != reservation) {}

            AssignPeepToSector_Insert(gameState, p);
        
        atomic_dec(lock);
    }



    //some singlethreaded update stuff
    if (globalid != 0)
        return;


    for (int a = 0; a < gameState->numActions; a++) {
        ClientAction* clientAction = &gameState->clientActions[a];
        cl_uchar cliId = clientAction->clientId;
        ClientState* client = &gameState->clientStates[cliId];
       
        if (clientAction->action_DoSelect)
        {
            client->selectedPeepsLastIdx = OFFSET_NULL;
            for (cl_uint pi = 0; pi < MAX_PEEPS; pi++)
            {
                Peep* p = &gameState->peeps[pi + globalid * chunkSize];


                if ((p->map_x_Q15_16 > clientAction->params_DoSelect_StartX_Q16) 
                    && (p->map_x_Q15_16 < clientAction->params_DoSelect_EndX_Q16))
                {

                    if ((p->map_y_Q15_16 < clientAction->params_DoSelect_StartY_Q16) 
                        && (p->map_y_Q15_16 > clientAction->params_DoSelect_EndY_Q16))
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
                curPeep->target_x_Q16 = clientAction->params_CommandToLocation_X_Q16;
                curPeep->target_y_Q16 = clientAction->params_CommandToLocation_Y_Q16;

                curPeepIdx = curPeep->prevSelectionPeepIdx[cliId];
            }


        }
    }
    
    gameState->numActions = 0;




}
