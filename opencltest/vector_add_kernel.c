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
        peep->xv_Q15_16 = -deltax_Q16;
        peep->yv_Q15_16 = -deltay_Q16;
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
__kernel void game_init_single(__global const GameState* gameState) {
    for (int pi = 0; pi < MAX_PEEPS; pi++)
    {
        __global Peep* p = &gameState->peeps[pi];

        
        AssignPeepToSector(gameState, p);
        //printf("Assigning Peep to sector %d\n", p->mapSector);
    }
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
    if (globalid >= WORKGROUPSIZE)
        return;

    local cl_uint localLocks[SQRT_MAXSECTORS][SQRT_MAXSECTORS];
    for (int x = 0; x < SQRT_MAXSECTORS; x++)
    {
        for (int y = 0; y < SQRT_MAXSECTORS; y++)
        {
            localLocks[x][y] = gameState->sectors[x][y].lock;
        }
    }





    const cl_uint chunkSize = MAX_PEEPS / WORKGROUPSIZE;
    for (cl_uint pi = 0; pi < MAX_PEEPS / WORKGROUPSIZE; pi++)
    {
        Peep* p = &gameState->peeps[pi + globalid*chunkSize];
        p->map_x_Q15_16 += p->xv_Q15_16;
        p->map_y_Q15_16 += p->yv_Q15_16;

        p->netForcex_Q16 = 0;
        p->netForcey_Q16 = 0;

        global volatile MapSector* mapSector = (global volatile MapSector *)p->mapSector;
        global volatile int* lock = (global volatile int*)&mapSector->lock;
        



        int reservation = atomic_add(lock, 1) + 1;
        barrier(CLK_GLOBAL_MEM_FENCE);

        while (atomic_add(lock, 0) != reservation) {  }
        AssignPeepToSector_Detach(gameState, p);

        atomic_dec(lock);
    }
}


__kernel void game_preupdate_2(__global const GameState* gameState) {
   
    // Get the index of the current element to be processed
    int globalid = get_global_id(0);
    if (globalid >= WORKGROUPSIZE)
        return;


    const cl_uint chunkSize = MAX_PEEPS / WORKGROUPSIZE;
    for (cl_uint pi = 0; pi < MAX_PEEPS / WORKGROUPSIZE; pi++)
    {
        Peep* p = &gameState->peeps[pi + globalid * chunkSize];

        global volatile MapSector* mapSector = (global volatile MapSector*)p->mapSector_pending;
        if (mapSector == NULL)
            continue;
        global volatile int* lock = (global volatile int*) & mapSector->lock;




        int reservation = atomic_add(lock, 1) + 1;
        barrier(CLK_GLOBAL_MEM_FENCE);

        while (atomic_add(lock, 0) != reservation) {}

            AssignPeepToSector_Insert(gameState, p);
        
        atomic_dec(lock);
    }



}
