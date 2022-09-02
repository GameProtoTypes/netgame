
#include "common.h"


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




cl_uchar MapTileCoordValid(ge_int3 mapcoord)
{
    if (mapcoord.x >= 0 && mapcoord.y >= 0 && mapcoord.z >= 0 && mapcoord.x < MAPDIM && mapcoord.y < MAPDIM && mapcoord.z < MAPDEPTH)
    {
        return 1;
    }
    return 0;
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



//inTileCoord_Q16 is [-1,1],[-1,1]
cl_int MapTileZHeight_Q16(cl_uint* tileData, ge_int2 inTileCoord_Q16)
{

    //4 corners
    

}


ge_int3 MapTileConvexHull_ClosestPoint(ConvexHull* hull, ge_int3 point_Q16)
{
    int smallestDist_Q16 = TO_Q16(1000);
    ge_int3 closestPoint;
    for (int i = 0; i < 14; i++)
    {
        Triangle3DHeavy* tri = &hull->triangles[i];

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




MapTileWholeToMapTileCenterQ16(int x, int y, int z)
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
void MapTileCoordClamp(ge_int3* mapCoord)
{
    (*mapCoord).x = clamp((*mapCoord).x, 0, MAPDIM - 1);
    (*mapCoord).y = clamp((*mapCoord).y, 0, MAPDIM - 1);
    (*mapCoord).z = clamp((*mapCoord).z, 0, MAPDEPTH - 1);
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