#include "cl_type_glue.h"
#include "fixedpoint_opencl.h"

#include "peep.h"
#include "randomcl.h"
#include "perlincl.h"

#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

#define ALL_CORE_PARAMS __global GameState* gameState, __global GameStateB* gameStateB, __global float* peepVBOBuffer, __global cl_uint* mapTileVBO, __global cl_uint* mapTileAttrVBO
#define ALL_CORE_PARAMS_PASS gameState, gameStateB, peepVBOBuffer, mapTileVBO, mapTileAttrVBO


void PeepPrint(Peep* peep)
{
    PrintQ16(peep->physics.drive.target_x_Q16);

}


void PeepToPeepInteraction(Peep* peep, Peep* otherPeep)
{
    if (peep->stateRender.deathState || otherPeep->stateRender.deathState)
        return;

    cl_int dist_Q16 = cl_distance_s2_Q16(peep->physics.base.pos_Q16.x, peep->physics.base.pos_Q16.y,
        otherPeep->physics.base.pos_Q16.x, otherPeep->physics.base.pos_Q16.y);

    if (dist_Q16 < peep->minDistPeep_Q16)
    {
        peep->minDistPeep_Q16 = dist_Q16;
        peep->minDistPeep = otherPeep;
    }
    

    //spread messages
    if (dist_Q16 < TO_Q16(6) && (peep->comms.orders_channel == otherPeep->comms.orders_channel))
    {
        if (otherPeep->comms.message_TargetReached)
        {
            peep->comms.message_TargetReached = otherPeep->comms.message_TargetReached;
        }
    }


}

void WorldToMap(ge_int3 world_Q16, ge_int3* out_map_tilecoords_Q16)
{
    ge_int3 b = { TO_Q16(MAP_TILE_SIZE) ,TO_Q16(MAP_TILE_SIZE) ,TO_Q16(MAP_TILE_SIZE) };
    *out_map_tilecoords_Q16 = DIV_v3_Q16(world_Q16, b);
    
}

void MapToWorld(ge_int3 map_tilecoords_Q16, ge_int3* world_Q16)
{
    ge_int3 b = { TO_Q16(MAP_TILE_SIZE) ,TO_Q16(MAP_TILE_SIZE) ,TO_Q16(MAP_TILE_SIZE) };
    *world_Q16 = MUL_v3_Q16(map_tilecoords_Q16, b);
}


void PeepGetMapTile(ALL_CORE_PARAMS, Peep* peep, ge_int3 offset, MapTile* out_map_tile)
{
    int z = WHOLE_Q16(peep->mapTileLoc_Q16.z) + offset.z;
    int x = WHOLE_Q16(peep->mapTileLoc_Q16.x) + offset.x;
    int y = WHOLE_Q16(peep->mapTileLoc_Q16.y) + offset.y;

    z = clamp(z, 0, MAPDEPTH);
    x = clamp(x, 0, MAPDIM);
    y = clamp(y, 0, MAPDIM);


    *out_map_tile = gameState->map.levels[z].tiles[x][y];
}

void PeepRadiusPhysics(ALL_CORE_PARAMS, Peep* peep)
{

    //calculate force based on penetration distance with minDistPeep.
    if (peep->minDistPeep != NULL && ((peep->minDistPeep_Q16) < peep->physics.shape.radius_Q16))
    {
        ge_int3 d_Q16 = GE_INT3_ADD(peep->minDistPeep->physics.base.pos_Q16, GE_INT3_NEG(peep->physics.base.pos_Q16));
        cl_int combined_r_Q16 = peep->physics.shape.radius_Q16 + peep->minDistPeep->physics.shape.radius_Q16;
        cl_int len_Q16;


        ge_int3 penV_Q16 = d_Q16;
        ge_normalize_v3_Q16(&penV_Q16, &len_Q16);

        cl_int penetrationDist_Q16 = (len_Q16 - combined_r_Q16);
        ge_int3 penetrationForce_Q16;
        

        penetrationForce_Q16.x = MUL_PAD_Q16(penetrationDist_Q16, penV_Q16.x) >> 4;
        penetrationForce_Q16.y = MUL_PAD_Q16(penetrationDist_Q16, penV_Q16.y) >> 4;
        penetrationForce_Q16.z = MUL_PAD_Q16(penetrationDist_Q16, penV_Q16.z) >> 4;

      
        peep->physics.base.netForce_Q16 = GE_INT3_ADD(peep->physics.base.netForce_Q16, penetrationForce_Q16);
 
        //peep->physics.base.v_Q16 = (ge_int3){0,0,0};//todo - do dot product and trim nessecary velocity only.

        

    }
    else
    {
    //    peep->physics.base.netForce_Q16 = (ge_int3){ 0,0,0 };
    }

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

    if (WHOLE_Q16(len) < 2)
    {
        if (peep->physics.drive.drivingToTarget)
        {
            peep->physics.drive.drivingToTarget = 0;
            peep->comms.message_TargetReached = 255;//send the message
        }
    }


    targetVelocity.x = d.x << 1 ;
    targetVelocity.y = d.y << 1;

    ge_int3 vel_error = { targetVelocity.x - peep->physics.base.v_Q16.x, targetVelocity.y - peep->physics.base.v_Q16.y, 0 };
    if (peep->physics.drive.drivingToTarget)
    {
        peep->physics.base.netForce_Q16.x = peep->physics.base.netForce_Q16.x + vel_error.x >> 2;
        peep->physics.base.netForce_Q16.y = peep->physics.base.netForce_Q16.y + vel_error.y >> 2;
    }


    //drag term
    peep->physics.base.netForce_Q16.x = peep->physics.base.netForce_Q16.x - peep->physics.base.v_Q16.x >> 1;
    peep->physics.base.netForce_Q16.y = peep->physics.base.netForce_Q16.y - peep->physics.base.v_Q16.y >> 1;




}

void WalkAndFight(ALL_CORE_PARAMS, Peep* peep)
{



    PeepRadiusPhysics(ALL_CORE_PARAMS_PASS, peep);
    PeepDrivePhysics(ALL_CORE_PARAMS_PASS, peep);





    //update maptile
    //MapTile curTile;
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){ 0, 0, 0 }, &curTile);

    //MapTile foottile;
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3){ 0, 0, -1 }, &foottile);

    //if (foottile == MapTile_NONE)
    //{
    //    //fall.
    //    peep->mapTileLoc_Q16.z += TO_Q16(-2) >> 4;
    //}


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
void AssignPeepToSector_Insert(ALL_CORE_PARAMS, Peep* peep)
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

    //Euler Integrate
    peep->physics.base.v_Q16.x += DIV_PAD_Q16(peep->physics.base.netForce_Q16.x , peep->physics.base.mass_Q16);
    peep->physics.base.v_Q16.y += DIV_PAD_Q16(peep->physics.base.netForce_Q16.y , peep->physics.base.mass_Q16);
    peep->physics.base.v_Q16.z += DIV_PAD_Q16(peep->physics.base.netForce_Q16.z , peep->physics.base.mass_Q16);

    peep->physics.base.pos_Q16.x += peep->physics.base.v_Q16.x;
    peep->physics.base.pos_Q16.y += peep->physics.base.v_Q16.y;
    peep->physics.base.pos_Q16.z += peep->physics.base.v_Q16.z;


    if (peep->stateRender.health <= 0)
        peep->stateRender.deathState = 1;


    //peep comms
    if (peep->comms.message_TargetReached)
    {
        peep->physics.drive.drivingToTarget = 0;
        peep->comms.message_TargetReached--;//message fade
    }
}

void PeepUpdate(ALL_CORE_PARAMS, Peep* peep)
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
                    PeepToPeepInteraction(peep, curPeep);
                }

                curPeep = curPeep->prevSectorPeep;
                if (curPeep == firstPeep)
                    break;
            }

        }
    }
   
    
    WorldToMap(peep->physics.base.pos_Q16, &peep->mapTileLoc_Q16);


    WalkAndFight(ALL_CORE_PARAMS_PASS, peep);



}


void BuildMapTileView(ALL_CORE_PARAMS, int x, int y)
{
    MapTile tileIdx = gameState->map.levels[gameStateB->mapZView].tiles[x][y];
    MapTile tileUpIdx;
    if (gameStateB->mapZView < MAPDEPTH)
    {
        tileUpIdx = gameState->map.levels[gameStateB->mapZView + 1].tiles[x][y];
    }
    else
    {
        tileUpIdx = MapTile_NONE;
    }

    mapTileAttrVBO[y * MAPDIM + x % MAPDIM] = 0;

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
        mapTileAttrVBO[y * MAPDIM + x % MAPDIM] |= clamp(vz,0,15);
    }
            
            

    if (tileUpIdx != MapTile_NONE)
    {
        mapTileVBO[y * MAPDIM + x % MAPDIM] = MapTile_NONE;//view obstructed by foottile above.
    }
    else
    {
        mapTileVBO[y * MAPDIM + x % MAPDIM] = tileIdx;
    }
            
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
                curPeep->physics.drive.target_x_Q16 = clientAction->params_CommandToLocation_X_Q16;
                curPeep->physics.drive.target_y_Q16 = clientAction->params_CommandToLocation_Y_Q16;
                curPeep->physics.drive.drivingToTarget = 1;

                //restrict comms to new channel
                curPeep->comms.orders_channel = RandomRange(client->selectedPeepsLastIdx, 0, 10000);
                curPeep->comms.message_TargetReached = 0;

                curPeepIdx = curPeep->prevSelectionPeepIdx[cliId];
            }


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
            cl_int perlin_z_Q16 = cl_perlin_2d_Q16(TO_Q16(x), TO_Q16(y), TO_Q16(1) >> 6, 8, 0);

            for (int z = 0; z < MAPDEPTH; z++)
            {
                int zPerc_Q16 = DIV_PAD_Q16(TO_Q16(z), TO_Q16(MAPDEPTH));
               
                int depthPerc_Q16 = TO_Q16(2) + perlin_z_Q16 - zPerc_Q16;
                depthPerc_Q16 = DIV_PAD_Q16(depthPerc_Q16, TO_Q16(3));
                //PrintQ16(depthPerc_Q16*100);
                MapTile tileType = MapTile_NONE;
                if (depthPerc_Q16 * 100 > TO_Q16(90))
                {
                    tileType = MapTile_DiamondOre;
                }
                else if (depthPerc_Q16 * 100 > TO_Q16(80))
                {
                    tileType = MapTile_CopperOre;
                }
                else if (depthPerc_Q16 * 100 > TO_Q16(75))
                {
                    tileType = MapTile_Rock;
                }
                else if (depthPerc_Q16 * 100 > TO_Q16(70))
                {
                    tileType = MapTile_Dirt;
                }
                else if (depthPerc_Q16 * 100 > TO_Q16(65))
                {
                    tileType = MapTile_DarkGrass;
                }



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
        }
    }
}



__kernel void game_init_single(ALL_CORE_PARAMS)
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


    CreateMap(ALL_CORE_PARAMS_PASS);


    for (cl_uint p = 0; p < MAX_PEEPS; p++)
    {
        gameState->peeps[p].Idx = p;
        gameState->peeps[p].physics.base.pos_Q16.x = RandomRange(p,-500 << 16, 500 << 16) ;
        gameState->peeps[p].physics.base.pos_Q16.y = RandomRange(p+1,-500 << 16, 500 << 16) ;
        gameState->peeps[p].physics.base.pos_Q16.z = 100;
        gameState->peeps[p].physics.base.mass_Q16 = TO_Q16(1);
        gameState->peeps[p].physics.shape.radius_Q16 = TO_Q16(5);
        gameState->peeps[p].stateRender.attackState = 0;
        gameState->peeps[p].stateRender.health = 10;
        gameState->peeps[p].stateRender.deathState = 0;

        gameState->peeps[p].physics.base.v_Q16 = (ge_int3){0,0,0};
        gameState->peeps[p].physics.base.netForce_Q16 = (ge_int3){ 0,0,0 };

        gameState->peeps[p].minDistPeep = NULL;
        gameState->peeps[p].minDistPeep_Q16 = (1 << 30);
        gameState->peeps[p].mapSector = NULL;
        gameState->peeps[p].mapSector_pending = NULL;
        gameState->peeps[p].nextSectorPeep = NULL;
        gameState->peeps[p].prevSectorPeep = NULL;
        gameState->peeps[p].physics.drive.target_x_Q16 = gameState->peeps[p].physics.base.pos_Q16.x;
        gameState->peeps[p].physics.drive.target_y_Q16 = gameState->peeps[p].physics.base.pos_Q16.y;
        gameState->peeps[p].physics.drive.drivingToTarget = 0;


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

        AssignPeepToSector_Detach(ALL_CORE_PARAMS_PASS, p);
        AssignPeepToSector_Insert(ALL_CORE_PARAMS_PASS, p);

    }

    printf("Peep Sector Assigment Finished\n");



}


void PeepDraw(ALL_CORE_PARAMS, Peep* peep)
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



    peepVBOBuffer[peep->Idx * PEEP_VBO_INSTANCE_SIZE/sizeof(float) + 0] = (float)((float)peep->physics.base.pos_Q16.x / (1 << 16));
    peepVBOBuffer[peep->Idx * PEEP_VBO_INSTANCE_SIZE / sizeof(float) + 1] = (float)((float)peep->physics.base.pos_Q16.y / (1 << 16));

    peepVBOBuffer[peep->Idx * PEEP_VBO_INSTANCE_SIZE / sizeof(float) + 2] = drawColor.x* brightFactor;
    peepVBOBuffer[peep->Idx * PEEP_VBO_INSTANCE_SIZE / sizeof(float) + 3] = drawColor.y * brightFactor;
    peepVBOBuffer[peep->Idx * PEEP_VBO_INSTANCE_SIZE / sizeof(float) + 4] = drawColor.z * brightFactor;
}

__kernel void game_update(ALL_CORE_PARAMS)
{
    // Get the index of the current element to be processed
    int globalid = get_global_id(0);
    int localid = get_local_id(0);

    
    Peep* p = &gameState->peeps[globalid];
    PeepUpdate(ALL_CORE_PARAMS_PASS,p);
    PeepDraw(ALL_CORE_PARAMS_PASS,p);



    //update map view
    if (globalid >= WARPSIZE)
        return;

    if (gameStateB->mapZView != gameStateB->mapZView_1)
    {
        const cl_uint chunkSize = (MAPDIM* MAPDIM) / WARPSIZE;
        for (cl_ulong i = 0; i < (MAPDIM * MAPDIM) / WARPSIZE; i++)
        {
            cl_ulong xyIdx = i + globalid * chunkSize;
            BuildMapTileView(ALL_CORE_PARAMS_PASS, xyIdx% MAPDIM, xyIdx/ MAPDIM);

        }

    }
    barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
    gameStateB->mapZView_1 = gameStateB->mapZView;
    
}


__kernel void game_preupdate_1(ALL_CORE_PARAMS) {


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

        AssignPeepToSector_Detach(ALL_CORE_PARAMS_PASS, p);

        atomic_dec(lock);
       
    }
}


__kernel void game_preupdate_2(ALL_CORE_PARAMS) {


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

            AssignPeepToSector_Insert(ALL_CORE_PARAMS_PASS, p);
        
        atomic_dec(lock);
    }






}
