
#include "common.h"


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


            //determine if simple box or convex hull collision
            if (MapTileDataHasLowCorner(tileDatas[i]))
            //if(0)
            {

                ConvexHull hull;
                MapTileConvexHull_From_TileData(&hull, &tileDatas[i]);
                ge_int3 peepPosLocalToHull_Q16 = GE_INT3_SUB(futurePos, tileCenters_Q16[i]);

                peepPosLocalToHull_Q16 = GE_INT3_DIV_Q16(peepPosLocalToHull_Q16, (ge_int3) {
                    TO_Q16(MAP_TILE_SIZE), TO_Q16(MAP_TILE_SIZE)
                        , TO_Q16(MAP_TILE_SIZE)
                });

                nearestPoint = MapTileConvexHull_ClosestPoint(&hull, peepPosLocalToHull_Q16);

                nearestPoint = GE_INT3_MUL_Q16(nearestPoint, (ge_int3) {
                    TO_Q16(MAP_TILE_SIZE), TO_Q16(MAP_TILE_SIZE)
                        , TO_Q16(MAP_TILE_SIZE)
                });

                nearestPoint = GE_INT3_ADD(nearestPoint, tileCenters_Q16[i]);
            }
            else
            {
                cl_int3 tileMin_Q16;
                cl_int3 tileMax_Q16;

                tileMin_Q16.x = tileCenters_Q16[i].x - (TO_Q16(MAP_TILE_SIZE) >> 1);
                tileMin_Q16.y = tileCenters_Q16[i].y - (TO_Q16(MAP_TILE_SIZE) >> 1);


                tileMax_Q16.x = tileCenters_Q16[i].x + (TO_Q16(MAP_TILE_SIZE) >> 1);
                tileMax_Q16.y = tileCenters_Q16[i].y + (TO_Q16(MAP_TILE_SIZE) >> 1);


                tileMin_Q16.z = tileCenters_Q16[i].z - (TO_Q16(MAP_TILE_SIZE) >> 1);
                tileMax_Q16.z = tileCenters_Q16[i].z + (TO_Q16(MAP_TILE_SIZE) >> 1);

                nearestPoint.x = clamp(futurePos.x, tileMin_Q16.x, tileMax_Q16.x);
                nearestPoint.y = clamp(futurePos.y, tileMin_Q16.y, tileMax_Q16.y);
                nearestPoint.z = clamp(futurePos.z, tileMin_Q16.z, tileMax_Q16.z);

            }

            



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


void PeepDrivePhysics(ALL_CORE_PARAMS, Peep* peep)
{
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
                }
            }
        }
    }
   
    if (peep->physics.drive.drivingToTarget)
    {
        targetVelocity.x = d.x >> 2;
        targetVelocity.y = d.y >> 2;


        peep->physics.base.vel_add_Q16.x += targetVelocity.x;
        peep->physics.base.vel_add_Q16.y += targetVelocity.y;


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

    if (curTile != MapTile_NONE)
    {
        //revert to center of last good map position
        ge_int3 lastGoodPos;
        MapToWorld(GE_INT3_WHOLE_ONLY_Q16(peep->lastGoodPosMap_Q16), &lastGoodPos);
        lastGoodPos.x += MUL_PAD_Q16(TO_Q16(MAP_TILE_SIZE), TO_Q16(1) >> 1);
        lastGoodPos.y += MUL_PAD_Q16(TO_Q16(MAP_TILE_SIZE), TO_Q16(1) >> 1);
        lastGoodPos.z += MUL_PAD_Q16(TO_Q16(MAP_TILE_SIZE), TO_Q16(1) >> 1);


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
