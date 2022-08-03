#include "cl_type_glue.h"
#include "fixedpoint_opencl.h"

#include "peep.h"
#include "randomcl.h"
#include "perlincl.h"

#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

#define ALL_CORE_PARAMS  __global GameState* gameState,\
 __global GameStateB* gameStateB, \
__global float* peepVBOBuffer, \
__global cl_uint* mapTile1VBO, \
__global cl_uint* mapTile1AttrVBO, \
__global cl_uint* mapTile2VBO, \
__global cl_uint* mapTile2AttrVBO

#define ALL_CORE_PARAMS_PASS  gameState, \
gameStateB, \
peepVBOBuffer, \
mapTile1VBO, \
mapTile1AttrVBO, \
mapTile2VBO, \
mapTile2AttrVBO


void PeepPrint(Peep* peep)
{
    PrintQ16(peep->physics.drive.target_x_Q16);
}


void PeepToPeepInteraction(Peep* peep, Peep* otherPeep)
{
    if (peep->stateRender.deathState || otherPeep->stateRender.deathState)
        return;


    cl_int dist_Q16 = ge_length_v3_Q16(GE_INT3_ADD(peep->physics.base.pos_Q16, -otherPeep->physics.base.pos_Q16));

    if (dist_Q16 < peep->minDistPeep_Q16)
    {
        peep->minDistPeep_Q16 = dist_Q16;
        peep->minDistPeepIdx = otherPeep->Idx;
    }
    




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

    *world_Q16 = MUL_v3_Q16(map_tilecoords_Q16, b);
}


void PeepGetMapTile(ALL_CORE_PARAMS, Peep* peep, ge_int3 offset, MapTile* out_map_tile, ge_int3* out_tile_world_pos_center_Q16, ge_int3* out_map_tile_coord_whole)
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
    

    *out_map_tile = gameState->map.levels[(*out_map_tile_coord_whole).z].tiles[(*out_map_tile_coord_whole).x][(*out_map_tile_coord_whole).y];
   
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

void PeepMapTileCollisions(ALL_CORE_PARAMS, Peep* peep)
{

    //maptile collisions
    MapTile tiles[26];
    ge_int3 tileCenters_Q16[26];
    ge_int3 dummy;

    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 0, 0 }, & tiles[0], & tileCenters_Q16[0],&dummy);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 0, 0 }, & tiles[1], & tileCenters_Q16[1], &dummy);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, -1, 0 }, & tiles[2], & tileCenters_Q16[2], &dummy);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 1, 0 }, & tiles[3], & tileCenters_Q16[3], &dummy);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 0, 1 }, & tiles[4], & tileCenters_Q16[4], &dummy);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 0, -1 }, & tiles[5], & tileCenters_Q16[5], &dummy);

    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 0, 0 }, & tiles[6], & tileCenters_Q16[6]);

    /*PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 0, -1 }, & tiles[6], & tileCenters_Q16[6]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 1, -1 }, & tiles[7], & tileCenters_Q16[7]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 1, -1 }, & tiles[8], & tileCenters_Q16[8]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 0, -1 }, & tiles[9], & tileCenters_Q16[9]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, -1, -1 }, & tiles[10], & tileCenters_Q16[10]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, -1, -1 }, & tiles[11], & tileCenters_Q16[11]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, -1, -1 }, & tiles[12], & tileCenters_Q16[12]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 1, -1 }, & tiles[13], & tileCenters_Q16[13]);

    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 0, 1 }, & tiles[14], & tileCenters_Q16[14]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 1, 1 }, & tiles[15], & tileCenters_Q16[15]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 1, 1 }, & tiles[16], & tileCenters_Q16[16]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 0, 1 }, & tiles[17], & tileCenters_Q16[17]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, -1, 1 }, & tiles[18], & tileCenters_Q16[18]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, -1, 1 }, & tiles[19], & tileCenters_Q16[19]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, -1, 1 }, & tiles[20], & tileCenters_Q16[20]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 1, 1 }, & tiles[21], & tileCenters_Q16[21]);


    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 1, 0 }, & tiles[22], & tileCenters_Q16[23]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 1, 0 }, & tiles[23], & tileCenters_Q16[24]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, -1, 0 }, & tiles[24], & tileCenters_Q16[25]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, -1, 0 }, & tiles[25], & tileCenters_Q16[26]);*/

    

    for (int i = 0; i < 6; i++)
    {
        //circle-circle collision with incorrect corner forces.
        MapTile tile = tiles[i];
        if (tile != MapTile_NONE)
        {
            cl_int3 tileMin_Q16;
            cl_int3 tileMax_Q16;

            tileMin_Q16.x = tileCenters_Q16[i].x - (TO_Q16(MAP_TILE_SIZE) >> 1);
            tileMin_Q16.y = tileCenters_Q16[i].y - (TO_Q16(MAP_TILE_SIZE) >> 1);
            tileMin_Q16.z = tileCenters_Q16[i].z - (TO_Q16(MAP_TILE_SIZE) >> 1);

            tileMax_Q16.x = tileCenters_Q16[i].x + (TO_Q16(MAP_TILE_SIZE) >> 1);
            tileMax_Q16.y = tileCenters_Q16[i].y + (TO_Q16(MAP_TILE_SIZE) >> 1);
            tileMax_Q16.z = tileCenters_Q16[i].z + (TO_Q16(MAP_TILE_SIZE) >> 1);

            ge_int3 futurePos;
            futurePos.x = peep->physics.base.pos_Q16.x + peep->physics.base.v_Q16.x;
            futurePos.y = peep->physics.base.pos_Q16.y + peep->physics.base.v_Q16.y;
            futurePos.z = peep->physics.base.pos_Q16.z + peep->physics.base.v_Q16.z;

            ge_int3 nearestPoint;
            nearestPoint.x = clamp(futurePos.x, tileMin_Q16.x, tileMax_Q16.x);
            nearestPoint.y = clamp(futurePos.y, tileMin_Q16.y, tileMax_Q16.y);
            nearestPoint.z = clamp(futurePos.z, tileMin_Q16.z, tileMax_Q16.z);

            ge_int3 A;
            A.x = futurePos.x - nearestPoint.x;
            A.y = futurePos.y - nearestPoint.y;
            A.z = futurePos.z - nearestPoint.z;

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


                
                peep->physics.base.pos_post_Q16.z += MUL_PAD_Q16(An.z, (peep->physics.shape.radius_Q16 - mag));
                peep->physics.base.pos_post_Q16.y += MUL_PAD_Q16(An.y, (peep->physics.shape.radius_Q16 - mag));
                peep->physics.base.pos_post_Q16.x += MUL_PAD_Q16(An.x, (peep->physics.shape.radius_Q16 - mag));

                

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

void PeepRadiusPhysics(ALL_CORE_PARAMS, Peep* peep)
{

    //calculate force based on penetration distance with minDistPeepIdx.
    if (peep->minDistPeepIdx != OFFSET_NULL && ((peep->minDistPeep_Q16) < peep->physics.shape.radius_Q16 * 2))
    {
        Peep* minDistPeep;

        OFFSET_TO_PTR(gameState->peeps, peep->minDistPeepIdx, minDistPeep);
        ge_int3 d_Q16 = GE_INT3_ADD(minDistPeep->physics.base.pos_Q16, GE_INT3_NEG(peep->physics.base.pos_Q16));
        cl_int combined_r_Q16 = peep->physics.shape.radius_Q16 + minDistPeep->physics.shape.radius_Q16;
        cl_int len_Q16;


        ge_int3 penV_Q16 = d_Q16;
        ge_normalize_v3_Q16(&penV_Q16, &len_Q16);

        cl_int penetrationDist_Q16 = (len_Q16 - (combined_r_Q16));
        ge_int3 penetrationForce_Q16;


        //pos_post_Q16
        peep->physics.base.pos_post_Q16.x += MUL_PAD_Q16(penV_Q16.x, penetrationDist_Q16 >> 1);
        peep->physics.base.pos_post_Q16.y += MUL_PAD_Q16(penV_Q16.y, penetrationDist_Q16 >> 1);
        //peep->physics.base.pos_post_Q16.z += MUL_PAD_Q16(penV_Q16.z, penetrationDist_Q16 >> 1);//dont encourage peeps standing on each other


        //V' = V - penV*(V.penV)
        //DeltaV = -penV*(V.penV)

        cl_int dot;
        ge_dot_product_3D_Q16(peep->physics.base.v_Q16, penV_Q16, &dot);
        if (dot > 0) {
            peep->physics.base.vel_add_Q16.x += -MUL_PAD_Q16(penV_Q16.x, dot);
            peep->physics.base.vel_add_Q16.y += -MUL_PAD_Q16(penV_Q16.y, dot);
            peep->physics.base.vel_add_Q16.z += -MUL_PAD_Q16(penV_Q16.z, dot);
        }

        //spread messages
        if ((peep->comms.orders_channel == minDistPeep->comms.orders_channel))
        {
            if (minDistPeep->comms.message_TargetReached)
            {
                peep->comms.message_TargetReached_pending = minDistPeep->comms.message_TargetReached;
            }
        }

    }


    PeepMapTileCollisions(ALL_CORE_PARAMS_PASS, peep);

}
void PeepDrivePhysics(ALL_CORE_PARAMS, Peep* peep)
{

    ge_int3 driveForce; //force to keep peep moving at near constant velocity
    ge_int3 targetVelocity;

    ge_int3 d;
    d.x = peep->physics.drive.target_x_Q16 - peep->physics.base.pos_Q16.x;
    d.y = peep->physics.drive.target_y_Q16 - peep->physics.base.pos_Q16.y;
    int len;
    ge_normalize_v3_Q16(&d, &len);

    if (len < TO_Q16(1))
    {
        d.x = MUL_PAD_Q16(d.x, len);
        d.y = MUL_PAD_Q16(d.y, len);
        d.z = MUL_PAD_Q16(d.z, len);
    }

    if (WHOLE_Q16(len) < 2)
    {
        if (peep->physics.drive.drivingToTarget)
        {
            peep->comms.message_TargetReached_pending = 255;//send the message
            
        }
    }

    if (peep->physics.drive.drivingToTarget)
    {
        targetVelocity.x = d.x >> 2;
        targetVelocity.y = d.y >> 2;


        peep->physics.base.v_Q16.x += targetVelocity.x;
        peep->physics.base.v_Q16.y += targetVelocity.y;


        peep->physics.base.CS_angle_rad = atan2(((float)(d.x))/(1<<16), ((float)(d.y)) / (1 << 16));
    }



}

void WalkAndFight(ALL_CORE_PARAMS, Peep* peep)
{


    PeepDrivePhysics(ALL_CORE_PARAMS_PASS, peep);

    PeepRadiusPhysics(ALL_CORE_PARAMS_PASS, peep);






}


void AssignPeepToSector_Detach(ALL_CORE_PARAMS, Peep* peep)
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
void AssignPeepToSector_Insert(ALL_CORE_PARAMS, Peep* peep)
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

    if (peep->stateRender.health <= 0)
        peep->stateRender.deathState = 1;


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
    ge_int3 maptilecoords;
    maptilecoords.x = WHOLE_Q16(peep->posMap_Q16.x);
    maptilecoords.y = WHOLE_Q16(peep->posMap_Q16.y);
    maptilecoords.z = WHOLE_Q16(peep->posMap_Q16.z);
    
    //search up to z level 0
    ge_int3 offset, tilePWorldCen, tileMapCoordWhole;
    MapTile ctile;
    offset.x = 0;
    offset.y = 0;
    offset.z = 0;
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, offset, &ctile, &tilePWorldCen, &tileMapCoordWhole);

    while (ctile == MapTile_NONE && tileMapCoordWhole.z < MAPDEPTH)
    {
        tileMapCoordWhole.z++;
        ctile = gameState->map.levels[tileMapCoordWhole.z].tiles[tileMapCoordWhole.x][tileMapCoordWhole.y];
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
        //pocket case

        if (tileMapCoordWhole.z >= mapZViewLevel+2)
        {
            printf("a");
            if (maptilecoords.z <= mapZViewLevel+1)
            {
                printf("b");
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
                    PeepToPeepInteraction(peep, curPeep);
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
    if (!GE_VECTOR3_EQUAL(maptilecoords, maptilecoords_prev) || (gameStateB->mapZView_1 != gameStateB->mapZView))
    {
     
        if (PeepMapVisiblity(ALL_CORE_PARAMS_PASS, peep, gameStateB->mapZView))
        {
            
            BITSET(peep->stateRender.bitflags0, PeepState_BitFlags_visible);
        }
        else
        {
            BITCLEAR(peep->stateRender.bitflags0, PeepState_BitFlags_visible);
        }
    }






    //revert position to last good if needed
    MapTile curTile;
    ge_int3 dummy;
    ge_int3 dummy2;
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 0, 0 }, &curTile, &dummy, & dummy2);

    if (curTile != MapTile_NONE)
    {
        ge_int3 lastGoodPos;
        MapToWorld(peep->lastGoodPosMap_Q16, &lastGoodPos);

        peep->physics.base.pos_post_Q16.x += lastGoodPos.x - peep->physics.base.pos_Q16.x;
        peep->physics.base.pos_post_Q16.y += lastGoodPos.y - peep->physics.base.pos_Q16.y;
        peep->physics.base.pos_post_Q16.z += lastGoodPos.z - peep->physics.base.pos_Q16.z;

    }
    else
    {

        peep->lastGoodPosMap_Q16 = peep->posMap_Q16;
    }
    
    WalkAndFight(ALL_CORE_PARAMS_PASS, peep);

}



void UpdateMapShadow(ALL_CORE_PARAMS, int x, int y)
{
    if (x < 1 || x >= MAPDIM - 1 || y < 1 || y >= MAPDIM - 1)
    {
        mapTile2VBO[y * MAPDIM + x] = MapTile_NONE;
        return;
    }
    MapTile tile = MapTile_NONE;
    mapTile2VBO[y * MAPDIM + x] = MapTile_NONE;
    for (int z = gameStateB->mapZView; z >= 1; z--)
    {       
        MapTile center = gameState->map.levels[z].tiles[x][y];
        MapTile below = gameState->map.levels[z].tiles[x][y];


        if (center != MapTile_NONE)
            return;


        // b | c | d
        // e |cen| f
        // g | h | i
        MapTile b = gameState->map.levels[z].tiles[x-1][y-1];
        MapTile c = gameState->map.levels[z].tiles[x][y-1];
        MapTile d = gameState->map.levels[z].tiles[x+1][y-1];
        MapTile e = gameState->map.levels[z].tiles[x-1][y];
        MapTile f = gameState->map.levels[z].tiles[x + 1][y];
        MapTile g = gameState->map.levels[z].tiles[x - 1][y+1];
        MapTile h = gameState->map.levels[z].tiles[x][y + 1];
        MapTile i = gameState->map.levels[z].tiles[x+1][y + 1];


        if ((f != MapTile_NONE) && (c == MapTile_NONE) && (e == MapTile_NONE) && (h == MapTile_NONE))
            tile = MapTile_Shadow_0;

        if ((f == MapTile_NONE) && (c == MapTile_NONE) && (e != MapTile_NONE) && (h == MapTile_NONE))
            tile = MapTile_Shadow_2;

        if ((f == MapTile_NONE) && (c != MapTile_NONE) && (e == MapTile_NONE) && (h == MapTile_NONE))
            tile = MapTile_Shadow_1;

        if ((f == MapTile_NONE) && (c == MapTile_NONE) && (e == MapTile_NONE) && (h != MapTile_NONE))
            tile = MapTile_Shadow_3;

        //-------------

        if ((f != MapTile_NONE) && (c != MapTile_NONE) && (e == MapTile_NONE) && (h == MapTile_NONE))
            tile = MapTile_Shadow_5;

        if ((f == MapTile_NONE) && (c != MapTile_NONE) && (e != MapTile_NONE) && (h == MapTile_NONE))
            tile = MapTile_Shadow_6;

        if ((f == MapTile_NONE) && (c == MapTile_NONE) && (e != MapTile_NONE) && (h != MapTile_NONE))
            tile = MapTile_Shadow_7;

        if ((f != MapTile_NONE) && (c == MapTile_NONE) && (e == MapTile_NONE) && (h != MapTile_NONE))
            tile = MapTile_Shadow_4;

        //-------------------------

        if ((f != MapTile_NONE) && (c != MapTile_NONE) && (e != MapTile_NONE) && (h == MapTile_NONE))
            tile = MapTile_Shadow_14;

        if ((f != MapTile_NONE) && (c != MapTile_NONE) && (e == MapTile_NONE) && (h != MapTile_NONE))
            tile = MapTile_Shadow_15;

        if ((f != MapTile_NONE) && (c == MapTile_NONE) && (e != MapTile_NONE) && (h != MapTile_NONE))
            tile = MapTile_Shadow_12;

        if ((f == MapTile_NONE) && (c != MapTile_NONE) && (e != MapTile_NONE) && (h != MapTile_NONE))
            tile = MapTile_Shadow_10;
        //----------------------------
        if ((f != MapTile_NONE) && (c == MapTile_NONE) && (e != MapTile_NONE) && (h == MapTile_NONE))
            tile = MapTile_Shadow_16;

        if ((f == MapTile_NONE) && (c != MapTile_NONE) && (e == MapTile_NONE) && (h != MapTile_NONE))
            tile = MapTile_Shadow_8;

        //------------------------------
        if ((f != MapTile_NONE) && (c != MapTile_NONE) && (e != MapTile_NONE) && (h != MapTile_NONE))
            tile = MapTile_Shadow_11;


        mapTile2VBO[y * MAPDIM + x] = tile;
    }


}

void BuildMapTileView(ALL_CORE_PARAMS, int x, int y)
{
    MapTile tileIdx = gameState->map.levels[gameStateB->mapZView].tiles[x][y];
    MapTile tileUpIdx;
    if (gameStateB->mapZView < MAPDEPTH-1)
    {
        tileUpIdx = gameState->map.levels[gameStateB->mapZView + 1].tiles[x][y];
    }
    else
    {
        tileUpIdx = MapTile_NONE;
    }

    mapTile1AttrVBO[y * MAPDIM + x] = 0;

    if (tileIdx == MapTile_NONE)
    {
        //look down...
        MapTile tileDownIdx = tileIdx;
        int z = gameStateB->mapZView;
        int vz = 0;
        while (tileDownIdx == MapTile_NONE)
        {
            tileDownIdx = gameState->map.levels[z].tiles[x][y];
            z--;
            vz++;
        }
        tileIdx = tileDownIdx;
        mapTile1AttrVBO[ y * MAPDIM + x ] |= clamp(vz*2,0,15);
    }
            
            

    if (tileUpIdx != MapTile_NONE)
    {
        mapTile1VBO[y * MAPDIM + x ] = MapTile_NONE;//view obstructed by foottile above.
    }
    else
    {
        mapTile1VBO[y * MAPDIM + x] = tileIdx;
    }


}

void PrintSelectionPeepStats(ALL_CORE_PARAMS, Peep* p)
{
///    Print_GE_INT3_Q16(p->physics.base.pos_Q16);
    Peep* peep = p;
    MapTile tiles[22];
    ge_int3 tileCenters_Q16[22];
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 0, 0 }, & tiles[0], & tileCenters_Q16[0]); printf("{ 1, 0, 0 }: %d\n", tiles[0]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 0, 0 }, & tiles[1], & tileCenters_Q16[1]); printf("{ -1, 0, 0 }: %d\n", tiles[1]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, -1, 0 }, & tiles[2], & tileCenters_Q16[2]); printf("{ 0, -1, 0 }: %d\n", tiles[2]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 1, 0 }, & tiles[3], & tileCenters_Q16[3]); printf("{ 0, 1, 0 }: %d\n", tiles[3]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 0, 1 }, & tiles[4], & tileCenters_Q16[4]); printf("{ 0, 0, 1 }: %d\n", tiles[4]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 0, -1 }, & tiles[5], & tileCenters_Q16[5]); printf("{ 0, 0, -1 }: %d\n", tiles[5]);

    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 0, -1 }, & tiles[6], & tileCenters_Q16[6]); printf("{ 1, 0, -1 }: %d\n", tiles[6]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 1, -1 }, & tiles[7], & tileCenters_Q16[7]); printf("{ 0, 1, -1 }: %d\n", tiles[7]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 1, -1 }, & tiles[8], & tileCenters_Q16[8]); printf("{ 1, 1, -1 }: %d\n", tiles[8]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 0, -1 }, & tiles[9], & tileCenters_Q16[9]); printf("{ -1, 0, -1 }: %d\n", tiles[9]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, -1, -1 }, & tiles[10], & tileCenters_Q16[10]); printf("{ 0, -1, -1 }: %d\n", tiles[10]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, -1, -1 }, & tiles[11], & tileCenters_Q16[11]); printf("{ -1, -1, -1 }: %d\n", tiles[11]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, -1, -1 }, & tiles[12], & tileCenters_Q16[12]); printf("{ 1, -1, -1 }: %d\n", tiles[12]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 1, -1 }, & tiles[13], & tileCenters_Q16[13]); printf("{ -1, 1, -1 }: %d\n", tiles[13]);

    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 0, 1 }, & tiles[14], & tileCenters_Q16[14]); printf("{ 1, 0, 1 }: %d\n", tiles[14]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 1, 1 }, & tiles[15], & tileCenters_Q16[15]); printf("{ 0, 1, 1 }: %d\n", tiles[15]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 1, 1 }, & tiles[16], & tileCenters_Q16[16]); printf("{ 1, 1, 1 }: %d\n", tiles[16]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 0, 1 }, & tiles[17], & tileCenters_Q16[17]); printf("{ -1, 0, 1 }: %d\n", tiles[17]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, -1, 1 }, & tiles[18], & tileCenters_Q16[18]); printf("{ 0, -1, 1 }: %d\n", tiles[18]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, -1, 1 }, & tiles[19], & tileCenters_Q16[19]); printf("{ -1, -1, 1 }: %d\n", tiles[19]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, -1, 1 }, & tiles[20], & tileCenters_Q16[20]); printf("{ 1, -1, 1 }: %d\n", tiles[20]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 1, 1 }, & tiles[21], & tileCenters_Q16[21]); printf("{ -1, 1, 1 }: %d\n", tiles[21]);

}



void GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS, ge_int2 world, ge_int3* mapcoord, int* occluded)
{
    ge_int3 wrld;
    wrld.x = world.x;
    wrld.y = world.y;
    WorldToMap(wrld, &(*mapcoord));
    (*mapcoord).x = WHOLE_Q16((*mapcoord).x);
    (*mapcoord).y = WHOLE_Q16((*mapcoord).y);



    for (int z = gameStateB->mapZView+1; z >= 0; z--)
    {

        MapTile tile = gameState->map.levels[z].tiles[(*mapcoord).x][(*mapcoord).y];

        if (tile != MapTile_NONE)
        {
            (*mapcoord).z = z;
            if (z == gameStateB->mapZView)
            {
                *occluded = 1;
              //  printf("1");
            }
            else if (z == gameStateB->mapZView + 1)
            {
               // printf("4");
                *occluded = 1;
            }
            else
            {
                *occluded = 0;
                //(*mapcoord).z--;
               // printf("2");
            }
            return;
        }
    }

   // printf("3");
    *occluded = 1;
}


__kernel void game_apply_actions(ALL_CORE_PARAMS)
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
                    if ((p->physics.base.pos_Q16.x > clientAction->params_DoSelect_StartX_Q16)
                        && (p->physics.base.pos_Q16.x < clientAction->params_DoSelect_EndX_Q16))
                    {

                        if ((p->physics.base.pos_Q16.y < clientAction->params_DoSelect_StartY_Q16)
                            && (p->physics.base.pos_Q16.y > clientAction->params_DoSelect_EndY_Q16))
                        {
                            
                            if (PeepMapVisiblity(ALL_CORE_PARAMS_PASS, p, clientAction->params_DoSelect_ZMapView))
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
        else if (clientAction->action_CommandToLocation)
        {
            cl_uint curPeepIdx = client->selectedPeepsLastIdx;
            while (curPeepIdx != OFFSET_NULL)
            {
                Peep* curPeep = &gameState->peeps[curPeepIdx];
                curPeep->physics.drive.target_x_Q16 = clientAction->params_CommandToLocation_X_Q16;
                curPeep->physics.drive.target_y_Q16 = clientAction->params_CommandToLocation_Y_Q16;
                curPeep->physics.drive.drivingToTarget = 1;

                //restrict comms to new channel
                curPeep->comms.orders_channel = RandomRange(client->selectedPeepsLastIdx, 0, 10000);
                curPeep->comms.message_TargetReached = 0;
                curPeep->comms.message_TargetReached_pending = 0;

                curPeepIdx = curPeep->prevSelectionPeepIdx[cliId];
            }
        }
        else if (clientAction->action_CommandTileDelete)
        {
            printf("Got the command\n");

            ge_int2 world2DMouse;
            world2DMouse.x = clientAction->params_CommandTileDelete_X_Q16;
            world2DMouse.y = clientAction->params_CommandTileDelete_Y_Q16;

            ge_int3 mapCoord;
            int occluded;
            GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world2DMouse, &mapCoord, &occluded);

            gameState->map.levels[mapCoord.z].tiles[mapCoord.x][mapCoord.y] = MapTile_NONE;

            BuildMapTileView(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y);

            UpdateMapShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y);
            UpdateMapShadow(ALL_CORE_PARAMS_PASS, mapCoord.x+1, mapCoord.y);
            UpdateMapShadow(ALL_CORE_PARAMS_PASS, mapCoord.x-1, mapCoord.y);
            UpdateMapShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y+1);
            UpdateMapShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y-1);

        }


    }

    gameStateB->numActions = 0;
}




void CreateMap(ALL_CORE_PARAMS)
{
    printf("Creating Map..\n");

    int i = 0;
    for (int x = 0; x < MAPDIM; x++)
    {
        for (int y = 0; y < MAPDIM; y++)
        {
            cl_int perlin_z_Q16 = cl_perlin_2d_Q16(TO_Q16(x), TO_Q16(y), TO_Q16(1) >> 6, 8, 0) ;

           
            for (int z = 0; z < MAPDEPTH; z++)
            {
                int zPerc_Q16 = DIV_PAD_Q16(TO_Q16(z), TO_Q16(MAPDEPTH));
                int depthFromSurface = perlin_z_Q16 - zPerc_Q16;
                MapTile tileType = MapTile_NONE;
                if (zPerc_Q16 < perlin_z_Q16)
                {
                    //if(zPerc_Q16* TO_Q16(100) > 90)
                    //    tileType = MapTile_DarkGrass;
                    //else if (zPerc_Q16 * TO_Q16(100) >50)
                    //    tileType = MapTile_Dirt;
                    //else if (zPerc_Q16 * TO_Q16(100) >= 20)

                        tileType = MapTile_Rock;

                        if (RandomRange(x * y * z, 0, 20) == 1)
                        {
                            tileType = MapTile_IronOre;
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



                gameState->map.levels[z].tiles[x][y] = tileType;


                i++;
            }
        }
    }



    for (int x = 0; x < MAPDIM; x++)
    {
        for (int y = 0; y < MAPDIM; y++)
        {
            BuildMapTileView(ALL_CORE_PARAMS_PASS,x,y);
            UpdateMapShadow(ALL_CORE_PARAMS_PASS, x, y);
        }
    }
}



__kernel void game_init_single(ALL_CORE_PARAMS)
{
    printf("Game Initializing...\n");


    printf("Speed Tests:\n");


    int s = 0;
    for (cl_ulong i = 0; i < 0; i++)
    {
        ge_int3 a = (ge_int3){ TO_Q16(i), TO_Q16(2), TO_Q16(i*2) };
        ge_int3 b = (ge_int3){ TO_Q16(i*2), TO_Q16(i), TO_Q16(i) };

        ge_int3 c = MUL_v3_Q16(a, b);
        s += c.x + c.y +c.z;
    }


    printf("End Tests: %d\n", s);

    fixedPointTests();



    gameState->numClients = 1;
    gameStateB->pauseState = 0;
    gameStateB->mapZView = MAPDEPTH-1;
    gameStateB->mapZView_1 = 0;

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


    CreateMap(ALL_CORE_PARAMS_PASS);

    const int spread = 500;
    for (cl_uint p = 0; p < MAX_PEEPS; p++)
    {
        gameState->peeps[p].Idx = p;
        gameState->peeps[p].physics.base.pos_Q16.x = RandomRange(p,-spread << 16, spread << 16) ;
        gameState->peeps[p].physics.base.pos_Q16.y = RandomRange(p+1,-spread << 16, spread << 16) ;
        gameState->peeps[p].physics.base.pos_Q16.z = TO_Q16(200);
        gameState->peeps[p].physics.shape.radius_Q16 = TO_Q16(1);
        BITCLEAR(gameState->peeps[p].stateRender.bitflags0, PeepState_BitFlags_deathState);
        BITSET(gameState->peeps[p].stateRender.bitflags0, PeepState_BitFlags_valid);
        BITSET(gameState->peeps[p].stateRender.bitflags0, PeepState_BitFlags_visible);
        gameState->peeps[p].stateRender.health = 10;


        gameState->peeps[p].physics.base.v_Q16 = (ge_int3){ 0,0,0 };



        gameState->peeps[p].minDistPeepIdx = OFFSET_NULL;
        gameState->peeps[p].minDistPeep_Q16 = (1 << 30);
        gameState->peeps[p].mapSectorIdx = GE_OFFSET_NULL_2D;
        gameState->peeps[p].mapSector_pendingIdx = GE_OFFSET_NULL_2D;
        gameState->peeps[p].nextSectorPeepIdx = OFFSET_NULL;
        gameState->peeps[p].prevSectorPeepIdx = OFFSET_NULL;
        gameState->peeps[p].physics.drive.target_x_Q16 = gameState->peeps[p].physics.base.pos_Q16.x;
        gameState->peeps[p].physics.drive.target_y_Q16 = gameState->peeps[p].physics.base.pos_Q16.y;
        gameState->peeps[p].physics.drive.drivingToTarget = 0;

       // if(gameState->peeps[p].physics.base.pos_Q16.x > 0)
            gameState->peeps[p].stateRender.faction = 0;
       // else
       //     gameState->peeps[p].stateRender.faction = 1;


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

        AssignPeepToSector_Detach(ALL_CORE_PARAMS_PASS, p);
        AssignPeepToSector_Insert(ALL_CORE_PARAMS_PASS, p);

    }

    printf("Peep Sector Assigment Finished\n");



}


void PeepDraw(ALL_CORE_PARAMS, Peep* peep)
{
    float3 drawColor;
    float drawPosX = (float)((float)peep->physics.base.pos_Q16.x / (1 << 16));
    float drawPosY = (float)((float)peep->physics.base.pos_Q16.y / (1 << 16));

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
    if ( BITGET(peep->stateRender.bitflags0, PeepState_BitFlags_deathState) )
    {
        brightFactor = 0.6f;
        drawColor.x = 0.5f;
        drawColor.y = 0.5f;
        drawColor.z = 0.5f;
    }
    if (!BITGET(peep->stateRender.bitflags0, PeepState_BitFlags_visible))
    {
        drawPosX = 99999;
        drawPosY = 99999;
    }


    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 0] = drawPosX;
    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 1] = drawPosY;

    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 2] = drawColor.x* brightFactor;
    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 3] = drawColor.y * brightFactor;
    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 4] = drawColor.z * brightFactor;

    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 5] = peep->physics.base.CS_angle_rad;
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


    //update map view
    if (gameStateB->mapZView != gameStateB->mapZView_1)
    {
        cl_uint chunkSize = (MAPDIM * MAPDIM) / GAME_UPDATE_WORKITEMS;
        if (chunkSize == 0)
            chunkSize = 1;

        
        



        for (cl_ulong i = 0; i < chunkSize; i++)
        {
            cl_ulong xyIdx = i + globalid * chunkSize;
            
            if (xyIdx < (MAPDIM * MAPDIM))
            {
                BuildMapTileView(ALL_CORE_PARAMS_PASS, xyIdx % MAPDIM, xyIdx / MAPDIM);
                UpdateMapShadow(ALL_CORE_PARAMS_PASS, xyIdx % MAPDIM, xyIdx / MAPDIM);
            }
               

        }
    }

}


__kernel void game_post_update_single( ALL_CORE_PARAMS)
{

    gameStateB->mapZView_1 = gameStateB->mapZView;



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

            AssignPeepToSector_Detach(ALL_CORE_PARAMS_PASS, p);

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

            AssignPeepToSector_Insert(ALL_CORE_PARAMS_PASS, p);

            atomic_dec(lock);
        }
    }






}




