#include "cl_type_glue.h"
#include "peep.h"
#include "random.h"

// sqrt_i64 computes the squrare root of a 64bit integer and returns
// a 64bit integer value. It requires that v is positive.
long sqrt_i64(long v) {
    ulong b = ((ulong)1) << 62, q = 0, r = v;
    while (b > r)
        b >>= 2;
    while (b > 0) {
        ulong t = q + b;
        q >>= 1;
        if (r >= t) {
            r -= t;
            q += b;
        }
        b >>= 2;
    }
    return q;
}


void ForceInteraction(Peep* peep, Peep* otherPeep)
{

    cl_long deltax_Q16 = otherPeep->map_x_Q15_16 - peep->map_x_Q15_16;
    cl_long deltay_Q16 = otherPeep->map_y_Q15_16 - peep->map_y_Q15_16;

    cl_long innerx_Q32 = (deltax_Q16 * deltax_Q16);
    cl_long innery_Q32 = (deltay_Q16 * deltay_Q16);

    cl_long inner_Q32 = innerx_Q32 + innery_Q32;//dist_squared.

    cl_long distOffset_Q16 = (1 << 16);

    cl_long dist_Q16 = sqrt_i64(inner_Q32) - distOffset_Q16;

    cl_long forcex_Q16 = ((deltax_Q16 << 16) / dist_Q16);
    cl_long forcey_Q16 = ((deltay_Q16 << 16) / dist_Q16);

    cl_long forceMultiplier_Q16 = 1 << 16;


    //if (otherPeep->faction == peep->faction)
    //{
    //    forceMultiplier_Q16 = -16;
    //}
    forcex_Q16 *= forceMultiplier_Q16;
    forcey_Q16 *= forceMultiplier_Q16;
    forcex_Q16 = forcex_Q16 >> 16;
    forcey_Q16 = forcey_Q16 >> 16;


    //add to net force
    peep->netForcex_Q16 += forcex_Q16 >> 4;
    peep->netForcey_Q16 += forcey_Q16 >> 4;
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
    //traverse sector


    int minx = peep->mapSector->xidx - 2; if (minx < 0) minx = 0;
    int miny = peep->mapSector->yidx - 2; if (miny < 0) miny = 0;

    int maxx = peep->mapSector->xidx + 2; if (maxx >= SQRT_MAXSECTORS) maxx = SQRT_MAXSECTORS-1;
    int maxy = peep->mapSector->yidx + 2; if (maxy >= SQRT_MAXSECTORS) maxy = SQRT_MAXSECTORS-1;
    
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
                    ForceInteraction(peep, curPeep);
                }

                curPeep = curPeep->prevSectorPeep;
                if (curPeep == firstPeep)
                    break;
            }

        }
    }

}

__kernel void game_update(__global const GameState* gameState) {
    // Get the index of the current element to be processed
    int globalid = get_global_id(0);
    int localid = get_local_id(0);

    
    PeepUpdate(gameState, &gameState->peeps[globalid]);
}


__kernel void game_preupdate_single(__global const GameState* gameState) {
    // Get the index of the current element to be processed
    for (int pi = 0; pi < MAX_PEEPS; pi++)
    {
        Peep* p = &gameState->peeps[pi];
        p->netForcex_Q16 += -p->xv_Q15_16 >> 4;//drag
        p->netForcey_Q16 += -p->yv_Q15_16 >> 4;

        p->xv_Q15_16 += p->netForcex_Q16;
        p->yv_Q15_16 += p->netForcey_Q16;

        p->map_x_Q15_16 += p->xv_Q15_16;
        p->map_y_Q15_16 += p->yv_Q15_16;


        p->netForcex_Q16 = 0;
        p->netForcey_Q16 = 0;
        AssignPeepToSector(gameState, p );
    }
}