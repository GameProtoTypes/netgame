module;

    #include <stdio.h>
    #include <iostream>

    #include "Game.CoreParam.Defines.h"
    #include "../GE.Basic.Macro.defines.h"
    #include "../GE.ImGui.defines.h"


    #include "../GameGraphics.h"

    #define MAP_TILE_UV_WIDTH (1/16.0)
    #define MAP_TILE_UV_WIDTH_FLOAT2 (ge_float2(1/16.0, 1/16.0))

    #define PEEP_ALL_ALWAYS_VISIBLE

//#define FLATMAP

#define ALL_EXPLORED

module Game;

using namespace GE;
namespace Game {


// #define LOCAL_STR_INT(STRNAME, VALUE, STRLEN) \
//  ge_int STRLEN=CL_DIGITS_FOR_VALUE(VALUE, 10); \
//  char STRNAME[STRLEN+1]; \
//  CL_ITOA(VALUE, STRNAME, STRLEN, 10);  \
//  STRNAME[STRLEN] = '\0';  





// ge_int CL_DIGITS_FOR_VALUE(ge_int value, ge_int base)
// {
//     ge_int nDigits = 0;
//     ge_int origValue = value;
//     while(value)
//     {
//         nDigits++;
//         value /= base;
//     }
//     if(origValue < 0)
//         return nDigits+1;
//     else 
//         return nDigits;
// }

char* CL_ITOA(ge_int value, char* result, ge_int resultSize, ge_int base) {

    char* ptr = result, *ptr1 = result, tmp_char;
    ge_int tmp_value;
    ge_int counter = 0;
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
        counter++;

    } while ( value && counter < resultSize );


    //return if out of bounds of result
    if(counter >= resultSize)
    {
        printf("ERROR: CL_ITOA Overflow\n");
        return result;
    }

    
    // Apply negative sign
    if (tmp_value < 0) 
        *ptr++ = '-';

    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }

    return result;
}




void FixedPointTests()
{

    ge_int s = 0;
    for (ge_int i = 0; i < 100; i++)
    {
        ge_int3 a = ge_int3(GE_TO_Q<16>(i), GE_TO_Q<16>(2), GE_TO_Q<16>(i*2));
        ge_int3 b = ge_int3(GE_TO_Q<16>(i*2), GE_TO_Q<16>(i), GE_TO_Q<16>(i));
        ge_int x =  GE_TO_Q<16>(i);
        
        ge_int3 c = GE3_MUL(a, b);
        ge_int4 d = GE3_TO_GE4_ZERO_PAD(c);
        d = GE4_LEFTSHIFT<16>(d);


        ge_int4 vector{1,2,3,4};
        vector = GE4_MUL(vector, {i+2,i+3,i+4,i+5});


        s +=  d.x + d.y + d.z + x + vector.x + vector.y + vector.z + vector.w;
    }

    const float someFloat = -0.23f;
    constexpr ge_int h = GE_TO_Q(someFloat); 
    constexpr float j = GE_Q_TO_FLOAT(h);

    ge_int2 v(GE_TO_Q(1.0f), GE_TO_Q(1.0f));
    ge_int length = GE2_LENGTH_Q(v);
    ge_int distance = GE2_DISTANCE_Q({0,0},v);
    
    ge_int2 v_n = GE2_NORMALIZED_Q(v, length);

    const ge_int constQ = 24;
    ge_int a = GE_TO_Q<constQ>(1.0f);
    ge_int b = GE_TO_Q<constQ>(2.0f);
    const ge_int someQ = constQ-5;
    ge_int c = GE_DIV_Q<someQ,constQ,constQ>(a,b);
    GE_PRINT_Q<someQ>(length);
    //GE_PRINT(GE_SIGNED_SHIFT<ge_int, -1>(a));

    const float fltToRepresent = -123.32f;
    constexpr ge_int testQ = GE_BIGGEST_Q(fltToRepresent);
    constexpr ge_int n = GE_TO_Q<testQ>(fltToRepresent);
    constexpr float nF = GE_Q_TO_FLOAT<testQ>(n);
    printf("(GE_BIGGEST_Q) %f, is %f\n", fltToRepresent, nF);


    //ge_int overflowed = GE_MUL_Q(0xFFFFFFFF, 0xFFFFFFFF);
    //ge_int badcast = GE_MUL_Q<0,0,0>(0xFFFFFFFF, 0xFFFFF);


    printf("(Q2F)  1: %f\n", GE_Q_TO_FLOAT<16>(GE_TO_Q(1)));
    printf("(Q2F) -1: %f\n", GE_Q_TO_FLOAT<16>(GE_TO_Q(-1)));
    printf("(Q2F) -10: %f\n", GE_Q_TO_FLOAT<16>(GE_TO_Q(-10)));
    printf("(D2Q) -1: %d\n", GE_TO_Q(-1));

    printf(" (-2.2) %#x\n", GE_TO_Q(-2.2f));
    printf("-1.4: %f\n", GE_Q_TO_FLOAT<16>(GE_TO_Q(-1.414185f)));
    printf("1.7: %f\n", GE_Q_TO_FLOAT<16>(GE_TO_Q(1.748077f)));


    printf("-0.5: %f\n", GE_Q_TO_FLOAT(GE_DIV_Q(GE_TO_Q(-1), GE_TO_Q(2))));
    printf("-0.823529: %f\n", GE_Q_TO_FLOAT(GE_DIV_Q(GE_TO_Q(-1.4), GE_TO_Q(1.7))));

    printf("sqrt(0) = 0: %d\n", GE_SQRT_ULONG(0));



    ge_int2 p0 = ge_int2(GE_TO_Q(0), GE_TO_Q(0));
    ge_int2 p1 = ge_int2(GE_TO_Q(1), GE_TO_Q(1));
    ge_int2 p2 = ge_int2(GE_TO_Q(2), GE_TO_Q(-2));
    ge_int2 p3 = ge_int2(GE_TO_Q(3), GE_TO_Q(4));



    ge_int3 test = (ge_int3)(42);
    printf("%d,%d,%d\n", test);

    v = ge_int2(GE_TO_Q(1), GE_TO_Q(1));
    ge_int len;

    v = GE2_NORMALIZED_Q(v, len);

    printf("x: %f, y: %f, len: %f\n", GE_Q_TO_FLOAT(v.x), GE_Q_TO_FLOAT(v.y), GE_Q_TO_FLOAT(len));


    v = ge_int2(GE_TO_Q(1), GE_TO_Q(1));
    ge_int2 proj = ge_int2(GE_TO_Q(4), GE_TO_Q(5));
    ge_int scalar;
   
    v = GE2_PROJECTED_Q(v, proj, scalar);
    printf("[1,1] projected onto [4,5]: x: %f, y: %f, scalar: %f\n", GE_Q_TO_FLOAT(v.x), GE_Q_TO_FLOAT(v.y), GE_Q_TO_FLOAT(scalar));


    printf("Sign of -10: %f\n", GE_Q_TO_FLOAT(GE_SIGN_MAG_Q(GE_TO_Q(-10))));
    printf("Sign of 10: %f\n", GE_Q_TO_FLOAT(GE_SIGN_MAG_Q(GE_TO_Q(10))));
    printf("Sign of 0: %f\n", GE_Q_TO_FLOAT(GE_SIGN_MAG_Q(GE_TO_Q(0))));


    printf("GE_INT* FUNC TESTS:\n");

    ge_int3 A;
    A.z = GE_TO_Q(10);
    ge_int3 B;
    B.x = GE_TO_Q(10);
    B.y = GE_TO_Q(1);


    GE3_PRINT_Q(A);
    GE3_PRINT_Q(B);



}







SynchronizedClientState* ThisClient(ALL_CORE_PARAMS)
{
    return &gameState->clientStates[gameStateActions->clientId];
}




inline ge_uint BITBANK_GET_SUBNUMBER_UINT(ge_uint bank, ge_int lsbBitIdx, ge_int numBits)
{
    ge_uint mask = 0;
    for (ge_int i = 0; i < numBits; i++)
    {
        mask |= (1 << (i + lsbBitIdx));
    }


    return ((bank & mask) >> lsbBitIdx);
}


inline void BITBANK_SET_SUBNUMBER_UINT(ge_uint* bank, ge_int lsbBitIdx, ge_int numBits, ge_uint number)
{
    ge_int i = 0;
    

    ge_uint mask = 0;
    for (ge_int i = 0; i < numBits; i++)
    {
        mask |= (1 << (i + lsbBitIdx));
    }

    *bank &= ~mask;//clear the region
    *bank |= ((number << lsbBitIdx) & mask);
}



///----------------------------------------------------------------------------------------------------------------
///
/// 


inline MapTile MapDataGetTile(ge_uint tileData) {
    return (MapTile)BITBANK_GET_SUBNUMBER_UINT(tileData, 0, 8);
}
inline void MapDataSetTile(ge_uint* tileData, MapTile tile) {
    ge_uint tmp = *tileData;
    BITBANK_SET_SUBNUMBER_UINT(&tmp, 0, 8, tile);
    *tileData = tmp;
}

inline ge_int MapTileGetRotation(ge_uint tileData) {
    return BITBANK_GET_SUBNUMBER_UINT(tileData, 8, 2);
}

ge_uint* MapGetDataPointerFromCoord(ALL_CORE_PARAMS, ge_int3 mapcoord)
{
    return &(gameState->map.levels[(mapcoord).z].data[(mapcoord).x][(mapcoord).y]);
}

MapTile MapGetTileFromCoord(ALL_CORE_PARAMS, ge_int3 mapcoord)
{
    return MapDataGetTile(gameState->map.levels[(mapcoord).z].data[(mapcoord).x][(mapcoord).y]);
}

ge_ubyte MapRidgeType(ALL_CORE_PARAMS, ge_int3 mapCoords, ge_int3 enterDir)
{
    ge_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoords);

    if (MapDataGetTile(*data) == MapTile_NONE)
        return 0;


    /*offsets[0] = ge_int3{ 1, 0, 0 };
    offsets[1] = ge_int3{ -1, 0, 0 };
    offsets[2] = ge_int3{ 0, -1, 0 };
    offsets[3] = ge_int3{ 0, 1, 0 };
    offsets[4] = ge_int3{ 0, 0, 1 };
    offsets[5] = ge_int3{ 0, 0, -1 };*/

    if (GE3_EQUAL(enterDir, staticData->directionalOffsets[0]))
    {
        return 2 - (GE_BITGET_MF(*data, MapTileFlags_LowCornerBTMLEFT) + GE_BITGET_MF(*data, MapTileFlags_LowCornerTPLEFT));
    }
    else if (GE3_EQUAL(enterDir, staticData->directionalOffsets[1]))
    {
        return 2 - (GE_BITGET_MF(*data, MapTileFlags_LowCornerBTMRIGHT) + GE_BITGET_MF(*data, MapTileFlags_LowCornerTPRIGHT));
    }
    else if (GE3_EQUAL(enterDir, staticData->directionalOffsets[2]))
    {
        return 2 - (GE_BITGET_MF(*data, MapTileFlags_LowCornerBTMRIGHT) + GE_BITGET_MF(*data, MapTileFlags_LowCornerBTMLEFT));
    }
    else if (GE3_EQUAL(enterDir, staticData->directionalOffsets[3]))
    {
        return 2 - (GE_BITGET_MF(*data, MapTileFlags_LowCornerTPRIGHT) + GE_BITGET_MF(*data, MapTileFlags_LowCornerTPLEFT));
    }
    return 2;
}

ge_ubyte MapDataHas2LowAdjacentCorners( ge_uint* data)
{
    if (GE_BITGET_MF(*data, MapTileFlags_LowCornerTPLEFT) + GE_BITGET_MF(*data, MapTileFlags_LowCornerTPRIGHT) == 2)
        return 1;

    if (GE_BITGET_MF(*data, MapTileFlags_LowCornerTPRIGHT) + GE_BITGET_MF(*data, MapTileFlags_LowCornerBTMRIGHT) == 2)
        return 2;

    if (GE_BITGET_MF(*data, MapTileFlags_LowCornerBTMRIGHT) + GE_BITGET_MF(*data, MapTileFlags_LowCornerBTMLEFT) == 2)
        return 3;

    if (GE_BITGET_MF(*data, MapTileFlags_LowCornerBTMLEFT) + GE_BITGET_MF(*data, MapTileFlags_LowCornerTPLEFT) == 2)
        return 4;

    return 0;
}

ge_ubyte MapHas2LowAdjacentCorners(ALL_CORE_PARAMS, ge_int3 mapCoords)
{
    ge_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoords);
    ge_uint localCopy = *data;
    return MapDataHas2LowAdjacentCorners(&localCopy);
}


ge_ubyte MapTileDataHasLowCorner(ge_int tileData)
{
    return BITBANK_GET_SUBNUMBER_UINT(tileData, MapTileFlags_LowCornerTPLEFT, 4);
}


ge_ubyte MapTileData_TileSolid(ge_int tileData)
{
    if(MapTileDataHasLowCorner(tileData) == 0 && MapDataGetTile(tileData) != MapTile_NONE)
    {
        return 1;
    }
    else 
        return 0;
}

ge_ubyte MapTileData_PeepCount(ge_uint tileData)
{
    return BITBANK_GET_SUBNUMBER_UINT(tileData, MapTileFlags_PeepCount0, 3);
}
void MapTileData_SetPeepCount(ge_uint* tileData, ge_ubyte peepCount)
{
    peepCount = GE_CLAMP((ge_int)peepCount,0, 7);
    BITBANK_SET_SUBNUMBER_UINT(tileData, MapTileFlags_PeepCount0, 3, peepCount);
}

ge_ubyte MapHasLowCorner(ALL_CORE_PARAMS, ge_int3 mapCoords)
{
    ge_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoords);

    if (MapDataGetTile(*data) == MapTile_NONE)
        return 0;

    return MapTileDataHasLowCorner(*data);
}
ge_ubyte MapDataLowCornerCount(ge_int tileData)
{
    return (GE_BITGET_MF(tileData, MapTileFlags_LowCornerTPLEFT) +
        GE_BITGET_MF(tileData, MapTileFlags_LowCornerTPRIGHT) +
        GE_BITGET_MF(tileData, MapTileFlags_LowCornerBTMRIGHT) +
        GE_BITGET_MF(tileData, MapTileFlags_LowCornerBTMLEFT));
}
ge_ubyte MapLowCornerCount(ALL_CORE_PARAMS, ge_int3 mapCoords)
{
    ge_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoords);

    if (MapDataGetTile(*data) == MapTile_NONE)
        return 0;

    return MapDataLowCornerCount(*data);
}

ge_ubyte MapDataXLevel( ge_uint* data)
{
    if(MapDataHas2LowAdjacentCorners(data) > 0 && MapDataLowCornerCount(*data) == 2)
    {
        return 1;//mid x
    }
    else if(MapDataLowCornerCount(*data) == 3)
    {
        return 0;//low x
    }
    else 
        return 2;//high x
}

ge_ubyte MapTileXLevel(ALL_CORE_PARAMS, ge_int3 mapCoords)
{
    ge_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoords);
    ge_uint localCopy = *data;
    return MapDataXLevel(&localCopy);
}

ge_ubyte MapTileCoordStandInValid(ALL_CORE_PARAMS, ge_int3 mapcoord)
{
    ge_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapcoord);
    MapTile tile = MapDataGetTile(*data);
    if (tile == MapTile_NONE)
    {
        return 1;
    }
    else
    {
        if(MapDataLowCornerCount(*data) > 0)
        {
            return 1;
            printf("hmmmm\n");
        }
    }
    return 0;
}


ge_ubyte MapTileCoordEnterable(ALL_CORE_PARAMS, ge_int3 mapcoord, ge_int3 enterDirection)
{
    ge_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapcoord);
    MapTile tile = MapDataGetTile(*data);
    if (tile == MapTile_NONE)
    {   
        return 1;
    }
    else
        return 0;

    //  if(MapDataLowCornerCount(*data) == 0)//cant enter full blocks
    // {
    //     return 0;
    // }
    // else if(enterDirection.z == 0)//entering partial block from the side
    // {
    //     ge_ubyte ridgeType = MapRidgeType(ALL_CORE_PARAMS_PASS, mapcoord, enterDirection);
    //     if (ridgeType == 0)
    //         return 1;
    // }
    // else if(enterDirection.z > 0)//entering partial block from below
    // {
    //     return 1;
    //     ge_int3 dirNoZ = enterDirection;
    //     dirNoZ.z = 0;
    //     ge_ubyte ridgeType = MapRidgeType(ALL_CORE_PARAMS_PASS, mapcoord, dirNoZ);
    //     if (ridgeType == 0)//running up continued ramp case.
    //         return 1;

    // }
    // else if(enterDirection.z < 0)//entering partial block from above
    // {
    //     return 1;
    //     if(MapDataLowCornerCount(*data) > 0)//could be a ramp 
    //         return 1;
    // }
    // return 0;
}


void Machine_InitRecipes(ALL_CORE_PARAMS)
{
MachineRecipe* recip = &gameState->machineRecipes[MachineRecipe_IRON_ORE_CRUSHING];
   recip->numInputs = 1;
   recip->numOutputs = 2;
   recip->inputTypes[0] = ItemType_IRON_ORE;
   recip->outputTypes[0] = ItemType_IRON_DUST;
   recip->outputTypes[1] = ItemType_ROCK_DUST;

   recip->inputRatio[0] = 1;
   recip->outputRatio[0] = 1;
   recip->outputRatio[1] = 5;



    recip = &gameState->machineRecipes[MachineRecipe_IRON_DUST_SMELTING];
    recip->numInputs = 1;
    recip->numOutputs = 1;
    recip->inputTypes[0] = ItemType_IRON_DUST;
    recip->outputTypes[0] = ItemType_IRON_BAR;

    recip->inputRatio[0] = 5;
    recip->outputRatio[0] = 1;


    gameState->validMachineRecipes[MachineTypes_CRUSHER][0] = MachineRecipe_IRON_ORE_CRUSHING;
    gameState->validMachineRecipes[MachineTypes_SMELTER][0] = MachineRecipe_IRON_DUST_SMELTING;



}

void Machine_InitDescriptions(ALL_CORE_PARAMS)
{
    MachineDesc* m = &gameState->machineDescriptions[MachineTypes_CRUSHER];
    m->type = MachineTypes_CRUSHER;
    m->tile = MapTile_MACHINE_CRUSHER;
    m->processingTime = 30;


    m = &gameState->machineDescriptions[MachineTypes_SMELTER];
    m->type = MachineTypes_SMELTER;
    m->tile = MapTile_MACHINE_FURNACE;
    m->processingTime = 100;


    m = &gameState->machineDescriptions[MachineTypes_COMMAND_CENTER];
    m->type = MachineTypes_COMMAND_CENTER;
    m->tile = MapTile_MACHINE_COMMAND_CENTER;
    m->processingTime = 100;

    m = &gameState->machineDescriptions[MachineTypes_MINING_SITE];
    m->type = MachineTypes_MINING_SITE;
    m->tile = MapTile_MACHINE_MINING_SITE;
    m->processingTime = 10;
}

void InitItemTypeTiles(ALL_CORE_PARAMS)
{
    gameState->ItemTypeTiles[ItemType_IRON_ORE].itemTile = ItemTile_Ore;
    gameState->ItemTypeTiles[ItemType_IRON_DUST].itemTile = ItemTile_Dust;
    gameState->ItemTypeTiles[ItemType_IRON_BAR].itemTile = ItemTile_Bar;
    gameState->ItemTypeTiles[ItemType_ROCK_DUST].itemTile = ItemTile_Dust;


    gameState->ItemColors[ItemType_IRON_ORE] =  COLOR_ORANGE;
    gameState->ItemColors[ItemType_IRON_DUST] = COLOR_ORANGE;
    gameState->ItemColors[ItemType_IRON_BAR] =  COLOR_ORANGE;
    gameState->ItemColors[ItemType_ROCK_DUST] = COLOR_WHITE;
}

ge_offsetPtr Machine_CreateMachine(ALL_CORE_PARAMS)
{
    ge_offsetPtr ptr = gameState->nextMachineIdx;
    
    bool loopSense = false;
    do
    {
        gameState->nextMachineIdx++;
        if(gameState->nextMachineIdx >= MAX_MACHINES)
        {
            gameState->nextMachineIdx = 0;
            if(loopSense == true)
            {
                printf("Machine_CreateMachine Out of Machine Space (MAX_MACHINES)!\n");
                return GE_OFFSET_NULL;
            }
            loopSense=true;
        }
    }while(gameState->machines[gameState->nextMachineIdx].valid == true);

    Machine* machine;
    GE_OFFSET_TO_PTR(gameState->machines, ptr, machine);


    machine->rootOrderPtr = GE_OFFSET_NULL;




    return ptr;
}





























inline void AStarNodeInstantiate(AStarNode* node)
{
    
    node->g_Q16 = GE_TO_Q(0);
    node->h_Q16 = GE_TO_Q(0);
    node->nextOPtr = GE_OFFSET_NULL;  
    node->prevOPtr = GE_OFFSET_NULL;
    node->tileIdx.x = -1;
    node->tileIdx.y = -1;
    node->tileIdx.z = -1;

}
void AStarInitPathNode(AStarPathNode* node)
{
    node->mapCoord_Q16 = ge_int3{ 0,0,0 };
    node->nextOPtr = GE_OFFSET_NULL;
    node->prevOPtr = GE_OFFSET_NULL;

}


void AStarSearch_BFS_Instantiate(AStarSearch_BFS* search)
{
    for (ge_int x = 0; x < mapDim; x++)
    {
        for (ge_int y = 0; y < mapDim; y++)
        {
            for (ge_int z = 0; z < mapDepth; z++)
            {
                search->closedMap[x][y][z] = 0;
                search->openMap[x][y][z] = 0;
            }
        }  
    }

    for(ge_int i = 0; i < ASTARHEAPSIZE; i++)
    {
        search->openHeap_OPtrs[i] = GE_OFFSET_NULL;
    }
    
    search->nextDetailNodePtr = 0;

    search->startNodeOPtr = GE_OFFSET_NULL;
    search->endNodeOPtr = GE_OFFSET_NULL;
    
    search->openHeapSize = 0;
    search->pathOPtr = GE_OFFSET_NULL;

}
void AStarSearch_BFS_InstantiateParrallel(AStarSearch_BFS* search, ge_ulong idx, ge_int x, ge_int y, ge_int z)
{


    search->closedMap[x][y][z] = 0;
    search->openMap[x][y][z] = 0;

    search->nextDetailNodePtr = 0;

    if(idx < ASTARHEAPSIZE)
    {
        search->openHeap_OPtrs[idx] = GE_OFFSET_NULL;
    }
    
    



}




ge_ubyte MapTileCoordValid(ge_int3 mapcoord, ge_int xybuffer)
{
    if ((mapcoord.x >= xybuffer) && (mapcoord.y >= xybuffer) && mapcoord.z >= 0 && (mapcoord.x < mapDim-xybuffer) && (mapcoord.y < mapDim-xybuffer) && mapcoord.z < mapDepth)
    {
        return 1;
    }
    return 0;
}
ge_ubyte AStarNodeValid(AStarNode* node)
{
    return MapTileCoordValid(GE3_CAST<ge_int3>(node->tileIdx),1);
}
ge_ubyte AStarNode2NodeTraversible(ALL_CORE_PARAMS, AStarNode* node, AStarNode* prevNode)
{  

    ge_uint* fromTileData = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE3_CAST<ge_int3>(prevNode->tileIdx));
    ge_uint* toTileData = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE3_CAST<ge_int3>(node->tileIdx));

    ge_int3 delta = GE3_SUB(GE3_CAST<ge_int3>(node->tileIdx), GE3_CAST<ge_int3>( prevNode->tileIdx ));
    if (MapTileCoordEnterable(ALL_CORE_PARAMS_PASS, GE3_CAST<ge_int3>(node->tileIdx), delta) == 0)
    {

        return 0;
    }





    return 1;




}

void MakeCardinalDirectionOffsets(ge_int3* offsets)
{
    offsets[0] = ge_int3{ 1, 0, 0 };
    offsets[1] = ge_int3{ -1, 0, 0 };
    offsets[2] = ge_int3{ 0, -1, 0 };
    offsets[3] = ge_int3{ 0, 1, 0 };
    offsets[4] = ge_int3{ 0, 0, 1 };
    offsets[5] = ge_int3{ 0, 0, -1 };

    offsets[6] = ge_int3{ 1, 0, -1 };
    offsets[7] = ge_int3{ 0, 1, -1 };
    offsets[8] = ge_int3{ 1, 1, -1 };
    offsets[9] = ge_int3{ -1, 0, -1 };
    offsets[10] = ge_int3{ 0, -1, -1 };
    offsets[11] = ge_int3{ -1, -1, -1 };
    offsets[12] = ge_int3{ 1, -1, -1 };
    offsets[13] = ge_int3{ -1, 1, -1 };

    offsets[14] = ge_int3{ 1, 0, 1 };
    offsets[15] = ge_int3{ 0, 1, 1 };
    offsets[16] = ge_int3{ 1, 1, 1 };
    offsets[17] = ge_int3{ -1, 0, 1 };
    offsets[18] = ge_int3{ 0, -1, 1 };
    offsets[19] = ge_int3{ -1, -1, 1 };
    offsets[20] = ge_int3{ 1, -1, 1 };
    offsets[21] = ge_int3{ -1, 1, 1 };

    offsets[22] = ge_int3{ 1, 1, 0 };
    offsets[23] = ge_int3{ -1, 1, 0 };
    offsets[24] = ge_int3{ 1, -1, 0 };
    offsets[25] = ge_int3{ -1, -1, 0 };
}
ge_int AStarOpenHeapKey(AStarSearch_BFS* search, AStarNode* node)
{
    //f
    return node->g_Q16 + node->h_Q16;
}

void AStarOpenHeapTrickleDown(AStarSearch_BFS* search, ge_int index)
{
    ge_int largerChild;
    AStarNode* top;
    ge_offsetPtr topOPtr = search->openHeap_OPtrs[index];
    GE_OFFSET_TO_PTR(search->details, topOPtr, top);


    while (index < search->openHeapSize / 2)
    {
        ge_int leftChild = 2 * index + 1;
        ge_int rightChild = leftChild + 1;

        AStarNode* leftChildNode;
        GE_OFFSET_TO_PTR(search->details, search->openHeap_OPtrs[leftChild], leftChildNode)
        AStarNode* rightChildNode;
        GE_OFFSET_TO_PTR(search->details, search->openHeap_OPtrs[rightChild], rightChildNode)

        if ((rightChild < search->openHeapSize) && (AStarOpenHeapKey(search, leftChildNode) > AStarOpenHeapKey(search, rightChildNode)))
            largerChild = rightChild;
        else
            largerChild = leftChild;

        AStarNode* largerChildNode;

        GE_OFFSET_TO_PTR(search->details, search->openHeap_OPtrs[largerChild], largerChildNode)

        if (AStarOpenHeapKey(search, top) <= AStarOpenHeapKey(search, largerChildNode))
            break;

        search->openHeap_OPtrs[index] = search->openHeap_OPtrs[largerChild];
        index = largerChild;

        
    }
    
    search->openHeap_OPtrs[index] = topOPtr;
}

ge_offsetPtr AStarOpenHeapRemove(AStarSearch_BFS* search)
{
    ge_offsetPtr rootOPtr = search->openHeap_OPtrs[0];

    search->openHeap_OPtrs[0] = search->openHeap_OPtrs[search->openHeapSize-1];
    search->openHeapSize--;

 
    AStarOpenHeapTrickleDown(search, 0);

    return rootOPtr;
}

ge_offsetPtr AStarRemoveFromOpen(AStarSearch_BFS* search)
{

    ge_offsetPtr nodeOPtr = AStarOpenHeapRemove(search);
    

    AStarNode* node;
    GE_OFFSET_TO_PTR(search->details, nodeOPtr, node);

    search->openMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z] = 0;
    return nodeOPtr;
}
void AStarAddToClosed(AStarSearch_BFS* search,AStarNode* node, ge_offsetPtr ptr)
{
    search->closedMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z] = ptr;
}
bool AStarNodeInClosed(AStarSearch_BFS* search,AStarNode* node)
{
    return (bool)search->closedMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z];
}

bool AStarNodeInOpen(AStarSearch_BFS* search, AStarNode* node)
{
    return (bool)search->openMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z];
}
bool AStarCoordInOpen(AStarSearch_BFS* search, ge_short3 tile)
{
    return (bool)search->openMap[tile.x][tile.y][tile.z];
}


void AStarOpenHeapTrickleUp(AStarSearch_BFS* search, ge_int index)
{
    ge_int prev = (index - 1) / 2;
    ge_offsetPtr bottomOPtr = search->openHeap_OPtrs[index];

    AStarNode* bottomNode;
    GE_OFFSET_TO_PTR(search->details, bottomOPtr, bottomNode);

    AStarNode* prevNode;
    GE_OFFSET_TO_PTR(search->details, search->openHeap_OPtrs[prev], prevNode);

    while (index > 0 && AStarOpenHeapKey(search, prevNode) > AStarOpenHeapKey(search, bottomNode))
    {
        search->openHeap_OPtrs[index] = search->openHeap_OPtrs[prev];
        index = prev;
        prev = (prev - 1) / 2;
        GE_OFFSET_TO_PTR(search->details, search->openHeap_OPtrs[prev], prevNode);
    }
    search->openHeap_OPtrs[index] = bottomOPtr;
}


void AStarOpenHeapInsert(AStarSearch_BFS* search, ge_offsetPtr nodeOPtr)
{
    search->openHeap_OPtrs[search->openHeapSize] = nodeOPtr;
    AStarOpenHeapTrickleUp(search, search->openHeapSize);
    search->openHeapSize++;
    if (search->openHeapSize > ASTARHEAPSIZE)
        printf("ERROR: AStarHeap Size Greater than ASTARHEAPSIZE!\n");

}
void AStarAddToOpen(AStarSearch_BFS* search, ge_offsetPtr nodeOPtr)
{
    AStarOpenHeapInsert(search, nodeOPtr);

    AStarNode* node;
    GE_OFFSET_TO_PTR(search->details, nodeOPtr, node);
    search->openMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z] = nodeOPtr;
}

ge_offsetPtr AStarPathStepsNextFreePathNode(AStarPathSteps* list)
{
    ge_offsetPtr ptr = list->nextListIdx;
    while ((list->pathNodes[ptr].nextOPtr != GE_OFFSET_NULL))
    {
        ptr++;
        if (ptr >= ASTARPATHSTEPSSIZE)
            ptr = 0;
        
        if(ptr == list->nextListIdx)//wrap around
        {
            printf("No More Path Nodes Available.\n");
            return GE_OFFSET_NULL;
        }
    }
    list->nextListIdx = ptr+1;

    return ptr;
}





void AStarRemoveFromClosed(AStarSearch_BFS* search, AStarNode* node)
{
    search->closedMap[node->tileIdx.x][node->tileIdx.y][node->tileIdx.z] = 0;
}

ge_int AStarNodeDistanceHuristic(AStarSearch_BFS* search, AStarNode* nodeA, AStarNode* nodeB)
{
    return GE_TO_Q(GE_ABS(nodeA->tileIdx.x - nodeB->tileIdx.x) + GE_ABS(nodeA->tileIdx.y - nodeB->tileIdx.y) + GE_ABS(nodeA->tileIdx.z - nodeB->tileIdx.z));
}

void AStarPrintNodeStats(AStarNode* node)
{
    printf("Node: Loc: ");
    GE3_PRINT(node->tileIdx);
    printf(" H: %f, G: %f\n", GE_Q_TO_FLOAT(node->h_Q16), GE_Q_TO_FLOAT(node->g_Q16));
}
void AStarPrintPathNodeStats(AStarPathNode* node)
{
    printf("Node: Loc: ");
    GE3_PRINT_Q(node->mapCoord_Q16);
}



void AStarPrintPath(AStarPathSteps* paths, ge_offsetPtr startNodeOPtr)
{
    AStarPathNode* curNode;
    GE_OFFSET_TO_PTR(paths->pathNodes, startNodeOPtr, curNode);
    while (curNode != NULL)
    {
        AStarPrintPathNodeStats(curNode);
        GE_OFFSET_TO_PTR(paths->pathNodes, curNode->nextOPtr, curNode);
    }
}


// Ret 1: [2,0,2],[-1,-1,0],[4,4,4],[3,0,0] etc
// Ret 0: [2,0,1],[1,-1,1],[2,3,4],[0,0,0] etc
ge_ubyte GE_INT3_WHACHAMACOLIT1_ENTRY(ge_int3 a)
{
    if (GE3_ISZERO(a))
        return 0;

    ge_int n = a.x + a.y + a.z;
    ge_int s =0;
    if (a.x != 0)
        s++;
    if (a.y != 0)
        s++;
    if (a.z != 0)
        s++;

    ge_int f = 0;
    if (a.x != 0)
        f = a.x;
    else if (a.y != 0)
        f = a.y;
    else if (a.z != 0)
        f = a.z;


    if ((n / s) == f)
    {
        return 1;
    }
    return 0;
}


ge_offsetPtr AStarFormPathSteps(ALL_CORE_PARAMS, AStarSearch_BFS* search,
AStarPathSteps* steps,
 ge_offsetPtr startingPathPtr
 )
{
    //start building the list .
    ge_offsetPtr curNodeOPtr =  search->startNodeOPtr;
    AStarNode* curNode;
    GE_OFFSET_TO_PTR(search->details, curNodeOPtr, curNode);
    CL_CHECK_NULL(curNode);




    ge_offsetPtr startNodeOPtr = GE_OFFSET_NULL;
    AStarPathNode* pNP = NULL;
    ge_int i = 0;
    while (curNode != NULL)
    { 
        
        ge_offsetPtr index;
        if(i == 0 && (startingPathPtr != GE_OFFSET_NULL))
        {
            index = startingPathPtr;
        }
        else
            index = AStarPathStepsNextFreePathNode(&gameState->paths);

        AStarPathNode* pN = &gameState->paths.pathNodes[index];
        CL_CHECK_NULL(pN);

        pN->valid = true;
        pN->processing = false;
        pN->completed = true;

        if (i == 0)
            startNodeOPtr = index;


        ge_int3 holdTileCoord = GE3_CAST<ge_int3>(  curNode->tileIdx  );

        //put location at center of tile
        ge_int3 tileCenter = GE3_TO_Q(holdTileCoord);
        tileCenter.x += GE_TO_Q(1) >> 1;
        tileCenter.y += GE_TO_Q(1) >> 1;
        tileCenter.z += GE_TO_Q(1) >> 1;


        pN->mapCoord_Q16 = tileCenter;

        if (pNP != NULL)
        {
            pNP->nextOPtr = index;
        }
        pNP = pN;

        if (curNode->nextOPtr != GE_OFFSET_NULL)
        {

            GE_OFFSET_TO_PTR(search->details, curNode->nextOPtr, curNode);

            if(0)
            {
                //iterate until joint in path.
                ge_int3 delta;
                AStarNode* n2 = curNode;
                do
                {
                    GE_OFFSET_TO_PTR(search->details, n2->nextOPtr, n2);

                    if (n2 != NULL) {
                        delta = GE3_ADD(GE3_CAST<ge_int3>(n2->tileIdx), GE3_NEG(holdTileCoord));
                    }
                    else
                        delta = ge_int3{ 0,0,0 };

                } while ((n2 != NULL) && (GE_INT3_WHACHAMACOLIT1_ENTRY(delta) == 1));

                if (n2 != NULL) 
                {
                    AStarNode* n2Prev;
                    GE_OFFSET_TO_PTR(search->details, n2->prevOPtr, n2Prev);

                    if (curNode != n2Prev)
                        curNode = n2Prev;
                    else
                        curNode = n2;
                }
                else
                {
                    GE_OFFSET_TO_PTR(search->details, search->endNodeOPtr, curNode);
                }
            }
        }
        else
            curNode = NULL;


        i++;
    }
    pNP->nextOPtr = GE_OFFSET_NULL;



    //form prev links
    AStarPathNode* curNode2;
    ge_offsetPtr curNode2OPtr = startNodeOPtr;
    GE_OFFSET_TO_PTR(steps->pathNodes, startNodeOPtr, curNode2);

    while (curNode2 != NULL)
    {

        AStarPathNode* p;
        GE_OFFSET_TO_PTR(steps->pathNodes, curNode2->nextOPtr, p);


        if (p != NULL)
            p->prevOPtr = curNode2OPtr;


        curNode2 = p;
        if(curNode2 != NULL)
            curNode2OPtr = curNode2->nextOPtr;
        
    }


    bool loopCheck = false;
    while(steps->pathStarts[steps->nextPathStartIdx] != GE_OFFSET_NULL)
    {
        steps->nextPathStartIdx++;
        if(steps->nextPathStartIdx >= ASTAR_MAX_PATHS)
        {   
            if(loopCheck == true)
            {
                printf("Max Paths Reached!\n");
                break;
            }
            loopCheck = true;
            steps->nextPathStartIdx = 0;
        }
    }

    steps->pathStarts[steps->nextPathStartIdx] = startNodeOPtr;
    steps->nextPathStartIdx++;

   

  //  GE_OFFSET_TO_PTR(steps->pathNodes, startNodeOPtr, curNode2);
   // curNode2->prevOPtr = GE_OFFSET_NULL;

    return startNodeOPtr;
}

void AStarPathSteps_DeletePath(ALL_CORE_PARAMS, ge_offsetPtr pathStartPtr)
{
    for(ge_int i = 0 ; i < ASTAR_MAX_PATHS; i++)
    {
        if(gameState->paths.pathStarts[i] == pathStartPtr)
        {
            gameState->paths.pathStarts[i] = GE_OFFSET_NULL;
        }
    }

    AStarPathNode* pathNode;
    ge_offsetPtr curPathPtr = pathStartPtr;
    do {
        GE_OFFSET_TO_PTR(gameState->paths.pathNodes, curPathPtr, pathNode);
        curPathPtr = pathNode->nextOPtr;
        pathNode->valid = false;
        pathNode->nextOPtr = GE_OFFSET_NULL;
        pathNode->prevOPtr = GE_OFFSET_NULL;
    } while(curPathPtr != GE_OFFSET_NULL);



}

ge_offsetPtr AStarGrabDetailNode(AStarSearch_BFS* search, AStarNode** node_out)
{
    ge_offsetPtr ptr = search->nextDetailNodePtr;

    search->nextDetailNodePtr++;
    if(search->nextDetailNodePtr >= ASTARDETAILSSIZE)
    {
        printf("AStarGrabDetailNode Out of Nodes. (ASTARDETAILSSIZE)\n");
    }

    AStarNode* node;
    GE_OFFSET_TO_PTR(search->details, ptr, node);

    AStarNodeInstantiate(node);


    *node_out = node;
    

    return ptr;
}


AStarPathFindingProgress AStarSearch_BFS_Continue(ALL_CORE_PARAMS,AStarSearch_BFS* search, ge_int iterations)
{
    AStarNode* startNode;
    AStarNode* targetNode;
    GE_OFFSET_TO_PTR(search->details, search->startNodeOPtr,startNode);
    GE_OFFSET_TO_PTR(search->details, search->endNodeOPtr,targetNode);


    //printf("AStarSearch_BFS_Continue..openHeapSize: %d\n", search->openHeapSize);
    while (search->openHeapSize > 0 && iterations > 0)
    {
        //printf("AStarSearch_BFS_Continue iterating..%d\n", iterations);
        //find node in open with lowest f cost
        ge_offsetPtr currentOPtr = AStarRemoveFromOpen(search);


        AStarNode* current;
        GE_OFFSET_TO_PTR(search->details, currentOPtr, current);

        //printf("G: "); GE_PRINT_Q(current->g_Q16); printf(" H: "); GE_PRINT_Q(current->h_Q16);

        AStarAddToClosed(search, current, currentOPtr);//visited
        if (GE3_EQUAL(current->tileIdx , targetNode->tileIdx) )
        {
            printf("AStarSearch_BFS_Continue AStarPathFindingProgress_Finished\n");
            printf("Details Array Size: %d\n", search->nextDetailNodePtr);
            printf("target node ptr: %d\n", search->endNodeOPtr);


            search->state = AStarPathFindingProgress_Finished;
            

            targetNode->nextOPtr = GE_OFFSET_NULL;
            startNode->prevOPtr = GE_OFFSET_NULL;

            targetNode->prevOPtr = current->prevOPtr;

            //form next links
            AStarNode* curNode = targetNode;
            ge_offsetPtr curNodeOPtr = search->endNodeOPtr;

            while (curNode != NULL)
            {
                AStarNode* p;
                GE_OFFSET_TO_PTR(search->details, curNode->prevOPtr, p);

                if(p != NULL)
                    p->nextOPtr = curNodeOPtr;

                curNodeOPtr = curNode->prevOPtr;
                curNode = p;

            }

            //form a simplified path
            AStarFormPathSteps(ALL_CORE_PARAMS_PASS , search, &gameState->paths, search->pathOPtr);


            return search->state;//found dest
        }
        
        



        //5 neighbors
        for (ge_int i = 0; i <= 5; i++)
        { 
            ge_short3 prospectiveTileCoord;
            ge_int3 dir = staticData->directionalOffsets[i];
            prospectiveTileCoord.x = current->tileIdx.x + dir.x;
            prospectiveTileCoord.y = current->tileIdx.y + dir.y;
            prospectiveTileCoord.z = current->tileIdx.z + dir.z;
 
            if (MapTileCoordValid(GE3_CAST<ge_int3>(prospectiveTileCoord), 1)==0)
            {
                continue;
            }


            AStarNode* prospectiveNode;
            ge_offsetPtr prospectiveNodeOPtr;
            if(AStarCoordInOpen(search, prospectiveTileCoord))
            {
                prospectiveNodeOPtr = search->openMap[prospectiveTileCoord.x][prospectiveTileCoord.y][prospectiveTileCoord.z];
                GE_OFFSET_TO_PTR(search->details, prospectiveNodeOPtr, prospectiveNode);
            }
            else
            {
                //printf("grabbing new prospective node\n");
                prospectiveNodeOPtr = AStarGrabDetailNode(search, &prospectiveNode);
                prospectiveNode->tileIdx = prospectiveTileCoord;
            }






            if ((AStarNode2NodeTraversible(ALL_CORE_PARAMS_PASS,  prospectiveNode, current) == 0) || (AStarNodeInClosed(search, prospectiveNode)))
            {

                if(GE3_EQUAL(prospectiveNode->tileIdx , targetNode->tileIdx))
                {
                    printf("skippping obvious bullshit. %d, %d\n", AStarNode2NodeTraversible(ALL_CORE_PARAMS_PASS,  prospectiveNode, current), AStarNodeInClosed(search, prospectiveNode));
                }
                continue;
            }




            ge_int totalMoveCost = current->g_Q16 + AStarNodeDistanceHuristic(search, current, prospectiveNode);
           // GE_PRINT_Q(totalMoveCost); GE_PRINT_Q(current->g_Q16); GE_PRINT_Q(prospectiveNode->g_Q16);
            if (((totalMoveCost < prospectiveNode->g_Q16) || !AStarNodeInOpen(search, prospectiveNode)) )
            {
                

                prospectiveNode->g_Q16 = totalMoveCost;
                prospectiveNode->h_Q16 = AStarNodeDistanceHuristic(search, prospectiveNode, targetNode);
                
                prospectiveNode->prevOPtr = currentOPtr;

                //printf("G: "); GE_PRINT_Q(prospectiveNode->g_Q16); printf("H: ");  GE_PRINT_Q(prospectiveNode->h_Q16);
                
                //printf("Adding pn %d, with prevoptr of %d\n", prospectiveNodeOPtr, prospectiveNode->prevOPtr);
                
                AStarAddToOpen(search, prospectiveNodeOPtr);
            }
        }

        iterations--;
    }

    
    if(search->openHeapSize > 0)
    {
        printf("AStarPathFindingProgress_Searching, openHeap Size: %d\n", search->openHeapSize);
        search->state = AStarPathFindingProgress_Searching;
        return search->state;

    }
    else
    {
        printf("AStarPathFindingProgress_Failed");
        search->state = AStarPathFindingProgress_Failed;
        return search->state;
    }

}

AStarPathFindingProgress AStarSearch_BFS_Routine(ALL_CORE_PARAMS, AStarSearch_BFS* search, ge_int3 startTile, ge_int3 destTile, ge_int startIterations, ge_offsetPtr futurePathPtr)
{
    if (MapTileCoordValid(startTile,1) == 0)
    {
        printf("AStarSearch_BFS_Routine: Start MapCoord Not Valid.\n");
        return AStarPathFindingProgress_Failed;
    }
    if (MapTileCoordValid(destTile,1) == 0)
    {
        printf("AStarSearch_BFS_Routine: Dest MapCoord Not Valid.\n");
        return AStarPathFindingProgress_Failed;
    }
    if (MapTileCoordStandInValid(ALL_CORE_PARAMS_PASS, startTile) == 0)
    {
        printf("AStarSearch_BFS_Routine: Start Stand In Invalid.\n");
        return AStarPathFindingProgress_Failed;
    }
    if (MapTileCoordStandInValid(ALL_CORE_PARAMS_PASS, destTile)==0)
    {
        printf("AStarSearch_BFS_Routine: Dest Stand In Invalid.\n");
        return AStarPathFindingProgress_Failed;
    }

    printf("starting search from");
    search->state = AStarPathFindingProgress_Searching;
    


    AStarNode* startNode;    
    {
    //grab a extra node so offset 0 isnt valid.
    AStarGrabDetailNode(search, &startNode);
    }
    search->startNodeOPtr = AStarGrabDetailNode(search, &startNode);
    startNode->tileIdx = GE3_CAST<ge_short3>(startTile);

    AStarNode* endNode;
    search->endNodeOPtr = AStarGrabDetailNode(search, &endNode);
    endNode->tileIdx = GE3_CAST<ge_short3>(destTile);
    
    
    
    GE3_PRINT(startNode->tileIdx); printf(" to "); GE3_PRINT(endNode->tileIdx);
    
    search->pathOPtr = futurePathPtr;
    AStarPathNode* path;
    GE_OFFSET_TO_PTR(gameState->paths.pathNodes, search->pathOPtr, path);
    path->processing = true;



    //add start to openList
    startNode->h_Q16 = AStarNodeDistanceHuristic(search, startNode, endNode);
    AStarAddToOpen(search, search->startNodeOPtr );


    return AStarSearch_BFS_Continue(ALL_CORE_PARAMS_PASS, search, startIterations);
}






ge_uint* AStarPathNode_GetMapData(ALL_CORE_PARAMS, AStarPathNode* node)
{
    ge_int3 coord;
    coord = GE3_WHOLE_Q(node->mapCoord_Q16);
    #ifdef DEBUG
    if(MapTileCoordValid(coord,1) == 0)
    {   
        CL_THROW_ASSERT();
        return NULL;
    }
    #endif

    return MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, coord);
}




//get the last path node from a node in a path
ge_offsetPtr AStarPathNode_LastPathNode(AStarPathSteps* steps, ge_offsetPtr pathNodeOPtr)
{
    ge_offsetPtr curNodeOPtr = pathNodeOPtr;
    AStarPathNode* curNode;
    GE_OFFSET_TO_PTR(steps->pathNodes, curNodeOPtr, curNode);


    while(curNode->nextOPtr != GE_OFFSET_NULL)
    {
        GE_OFFSET_TO_PTR(steps->pathNodes, curNode->nextOPtr, curNode);
        curNodeOPtr = curNode->nextOPtr;
    }
    return curNodeOPtr;
}


ge_offsetPtr AStarJob_EnqueueJob(ALL_CORE_PARAMS)
{
    ge_offsetPtr ptr = gameState->lastMapSearchJobPtr;

    if(gameState->lastMapSearchJobPtr+1 == gameState->curMapSearchJobPtr)
    {
        printf("AStarJob_EnqueueJob: out of queue space (ASTAR_MAX_JOBS)");
        return ptr;
    }


    gameState->lastMapSearchJobPtr++;
    if(gameState->lastMapSearchJobPtr >= ASTAR_MAX_JOBS)
    {
        if(gameState->curMapSearchJobPtr == 0)
        {
            printf("AStarJob_EnqueueJob: out of queue space (ASTAR_MAX_JOBS)");
            return ptr;
        }
        gameState->lastMapSearchJobPtr = 0;
    }

    AStarJob* job;
    GE_OFFSET_TO_PTR(gameState->mapSearchJobQueue, ptr, job);
    job->status = AStarJobStatus_Pending;

    printf("created new job: %d\n", ptr);

    return ptr;
}

void AStarJob_UpdateJobs(ALL_CORE_PARAMS)
{
    AStarJob* job;
    GE_OFFSET_TO_PTR(gameState->mapSearchJobQueue, gameState->curMapSearchJobPtr, job);
    CL_CHECK_NULL(job);
    if(job->status == AStarJobStatus_Pending)
    {
        if(gameState->mapSearchers[0].state == AStarPathFindingProgress_Ready)
        {
            printf("Starting Search For AStarJob %d\n", gameState->curMapSearchJobPtr);

            AStarPathFindingProgress prog = AStarSearch_BFS_Routine(ALL_CORE_PARAMS_PASS, &gameState->mapSearchers[0],
            job->startLoc, job->endLoc, 1, job->pathPtr);

            if(prog == AStarPathFindingProgress_Failed)
            {
                job->status = AStarJobStatus_Done;
                printf("job failed\n");
            }
            else if(prog == AStarPathFindingProgress_Searching)
            {
                job->status = AStarJobStatus_Working;
            }
            else if(prog == AStarPathFindingProgress_Finished)
            {
                job->status = AStarJobStatus_Done;
                printf("job finished instantly\n");
            }
        }
        else
        {
            printf("cannot service job %d because pathfinder is not ready (%d)\n", gameState->curMapSearchJobPtr, gameState->mapSearchers[0].state);
        }
    } 
    
    if (job->status == AStarJobStatus_Working) 
    {
        if(gameState->mapSearchers[0].state == AStarPathFindingProgress_Finished)
        {
            job->pathPtr = gameState->mapSearchers[0].pathOPtr;
            job->status = AStarJobStatus_Done;

            printf("Job %d Done\n", gameState->curMapSearchJobPtr);


        }
        else if(gameState->mapSearchers[0].state == AStarPathFindingProgress_Failed)
        {
            printf("Job %d Failed\n", gameState->curMapSearchJobPtr);
            job->status = AStarJobStatus_Done;
        }
    }
    
    if (job->status == AStarJobStatus_Done)
    {
        printf("Job %d Finished, advancing..\n", gameState->curMapSearchJobPtr);
        job->status = AStarJobStatus_Disabled;

        //advance
        ge_offsetPtr tmp = gameState->curMapSearchJobPtr;
        gameState->curMapSearchJobPtr++;
        if(gameState->curMapSearchJobPtr >= ASTAR_MAX_JOBS)
        {
            gameState->curMapSearchJobPtr = 0;
        }
    }



    //update AStarPath Searchers
    AStarSearch_BFS* search = &gameState->mapSearchers[0];
    if(search->state == AStarPathFindingProgress_Searching)
    {
        AStarSearch_BFS_Continue(ALL_CORE_PARAMS_PASS, search, 100);
    }
    else if(	search->state == AStarPathFindingProgress_Finished ||
	search->state == AStarPathFindingProgress_Failed)
    {
        gameState->mapSearchers[0].state = AStarPathFindingProgress_ResetReady;

        
        gameState->mapSearchers[0].startNodeOPtr = GE_OFFSET_NULL;
        gameState->mapSearchers[0].endNodeOPtr = GE_OFFSET_NULL;
        gameState->mapSearchers[0].openHeapSize = 0;
        gameState->mapSearchers[0].pathOPtr = GE_OFFSET_NULL;
    }
}












ge_ubyte BaryCentric_In_Triangle_Q16(ge_int3 baryCoords)
{
    if (baryCoords.x >= 0 && baryCoords.x <= GE_TO_Q(1))
    {
        if (baryCoords.y >= 0 && baryCoords.y <= GE_TO_Q(1))
        {
            if (baryCoords.z >= 0 && baryCoords.z <= GE_TO_Q(1))
            {
                return 1;
            }
        }
    }
    return 0;
}


ge_int SOME_INTERNAL_CORDIST(ge_int x, ge_int y)
{
    if (y <= 0.0)
    {
        return -y;
    }
    else if (x <= 0)
        return  x;
    else
        return 0;
}

ge_int3 Triangle3D_ToBaryCentric( Triangle3DHeavy* triangle, ge_int3 point)
{
    ge_int3 U = triangle->u_Q16;
    ge_int3 V = triangle->v_Q16;
    ge_int3 W = GE3_SUB(point, triangle->base.verts_Q16[0]);


    long d00 = GE3_DOT_PRODUCT_Q(U , U );
    long d01 = GE3_DOT_PRODUCT_Q(U , V);
    long d11 = GE3_DOT_PRODUCT_Q(V , V);
    long d20 = GE3_DOT_PRODUCT_Q(W , U);
    long d21 = GE3_DOT_PRODUCT_Q(W , V);
    long denom = GE_MUL_Q( d00 , d11 ) - GE_MUL_Q(d01 , d01);
    long u, v, w;
    v = GE_DIV_Q((GE_MUL_Q(d11 , d20) - GE_MUL_Q(d01 , d21)) , denom);
    w = GE_DIV_Q((GE_MUL_Q(d00 , d21) - GE_MUL_Q(d01 , d20)) , denom);
    u = (GE_TO_Q(1) - v ) - w;
    return ge_int3 {
        u, v, w
    };
}

ge_int3 Triangle3DHeavy_ClosestPoint( Triangle3DHeavy* triangle, ge_int3 point_Q16, ge_int* dist_Q16)
{   
    ge_int3 P1 = triangle->base.verts_Q16[0];
    ge_int3 P2 = triangle->base.verts_Q16[1];
    ge_int3 P3 = triangle->base.verts_Q16[2];

    
    //printf("P1: ");
    //GE3_PRINT_Q(P1);
    //printf("P2: ");
    //GE3_PRINT_Q(P2);
    //printf("P3: ");
    //GE3_PRINT_Q(P3);


    ge_int3 P_prime;
    ge_int3 P_prime_bary;

    ge_int Nmag;
    ge_int3 N_n = GE3_NORMALIZED_Q(triangle->normal_Q16, Nmag);
    //printf("N_n: ");
    //GE3_PRINT_Q(N_n);


    ge_int3 W = GE3_SUB(point_Q16, P1);

    //printf("W: ");
    //GE3_PRINT_Q(W);


    ge_int dot = GE3_DOT_PRODUCT_Q(W, N_n);
    //printf("dot: ");
    //GE_PRINT_Q(dot);
    ge_int3 term2 = GE3_MUL_Q(GE3_NEG(N_n), dot);
    P_prime = GE3_ADD(point_Q16, term2);


    //Triangle2DHeavy_ProjectedPoint(triangle, point_Q16, &P_prime_bary, &P_prime);

    //printf("P_prime: ");
    //GE3_PRINT_Q(P_prime);

    P_prime_bary = Triangle3D_ToBaryCentric(triangle, P_prime);
    //printf("P_prime_bary: ");
    //GE3_PRINT_Q(P_prime_bary);
    ge_ubyte onSurface = BaryCentric_In_Triangle_Q16(P_prime_bary);

    if (onSurface == 1)
    {
        *dist_Q16 = GE3_LENGTH_Q(GE3_SUB(point_Q16, P_prime));

        return P_prime;
    }

    ge_int3 P1_P_prime = GE3_SUB(P_prime, P1);
    ge_int3 P2_P_prime = GE3_SUB(P_prime, P2);
    ge_int3 P3_P_prime = GE3_SUB(P_prime, P3);

    ge_int3 R1 = GE3_SUB(P1, P2);
    ge_int3 R2 = GE3_SUB(P2, P3);
    ge_int3 R3 = GE3_SUB(P3, P1);

    ge_int R1_mag;
    ge_int3 R1_N = GE3_NORMALIZED_Q(R1, R1_mag);
    ge_int R2_mag;
    ge_int3 R2_N = GE3_NORMALIZED_Q(R2, R2_mag);
    ge_int R3_mag;
    ge_int3 R3_N = GE3_NORMALIZED_Q(R3, R3_mag);


    ge_int3 R1_N_PERP = GE3_ROTATE_ABOUT_AXIS_POS90_Q(R1_N, triangle->normal_Q16);
    ge_int3 R2_N_PERP = GE3_ROTATE_ABOUT_AXIS_POS90_Q(R2_N, triangle->normal_Q16);
    ge_int3 R3_N_PERP = GE3_ROTATE_ABOUT_AXIS_POS90_Q(R3_N, triangle->normal_Q16);


    ge_int DOT1 = GE3_DOT_PRODUCT_Q(P1_P_prime, R3_N_PERP);
    ge_int DOT2 = GE3_DOT_PRODUCT_Q(P2_P_prime, R1_N_PERP);
    ge_int DOT3 = GE3_DOT_PRODUCT_Q(P3_P_prime, R2_N_PERP);


    ge_int3 D1R3 = GE3_MUL_Q( R3_N_PERP, DOT1);
    ge_int3 D2R1 = GE3_MUL_Q( R1_N_PERP, DOT2);
    ge_int3 D3R2 = GE3_MUL_Q( R2_N_PERP, DOT3);


    //p_prime projected to 3 edges
    ge_int3 P_prime_C1 = GE3_SUB(P_prime, D1R3);
    ge_int3 P_prime_C2 = GE3_SUB(P_prime, D2R1);
    ge_int3 P_prime_C3 = GE3_SUB(P_prime, D3R2);


    //GE_CLAMP C points to edge limits
    ge_int Z1i = GE3_DOT_PRODUCT_Q(P1_P_prime, R3_N);
    ge_int Z2i = GE3_DOT_PRODUCT_Q(P2_P_prime, R1_N);
    ge_int Z3i = GE3_DOT_PRODUCT_Q(P3_P_prime, R2_N);

    ge_int Z1 = R3_mag - Z1i;
    ge_int Z2 = R1_mag - Z2i;
    ge_int Z3 = R2_mag - Z3i;


    ge_int CD1 = SOME_INTERNAL_CORDIST(Z1, Z1i);
    ge_int CD2 = SOME_INTERNAL_CORDIST(Z2, Z2i);
    ge_int CD3 = SOME_INTERNAL_CORDIST(Z3, Z3i);


    ge_int3 J1 = GE3_MUL_Q( R3_N, CD1 );
    ge_int3 J2 = GE3_MUL_Q( R1_N, CD2 );
    ge_int3 J3 = GE3_MUL_Q( R2_N, CD3 );

    ge_int3 L1 = GE3_ADD(J1, P_prime_C1);
    ge_int3 L2 = GE3_ADD(J2, P_prime_C2);
    ge_int3 L3 = GE3_ADD(J3, P_prime_C3);

    //get closest L to P_prime
    ge_int L1D = GE3_LENGTH_Q( GE3_SUB(L1, P_prime) );
    ge_int L2D = GE3_LENGTH_Q( GE3_SUB(L2, P_prime) );
    ge_int L3D = GE3_LENGTH_Q( GE3_SUB(L3, P_prime) );


    if (L1D < L2D && L1D < L2D)
    {
        *dist_Q16 = GE3_LENGTH_Q(GE3_SUB(point_Q16, L1));

        return L1;
    }
    else if (L2D < L1D && L2D < L3D)
    {
        *dist_Q16 = GE3_LENGTH_Q(GE3_SUB(point_Q16, L2));

        return L2;
    }
    else
    {
        *dist_Q16 = GE3_LENGTH_Q(GE3_SUB(point_Q16, L3));

        return L3;
    }

}



void Triangle3DMakeHeavy( Triangle3DHeavy* triangle)
{
    triangle->u_Q16 = GE3_SUB(triangle->base.verts_Q16[1], triangle->base.verts_Q16[0]);//P_2 - P_1
    triangle->v_Q16 = GE3_SUB(triangle->base.verts_Q16[2], triangle->base.verts_Q16[0]);//P_3 - P_1


    triangle->normal_Q16 =  GE3_CROSS_PRODUCT_Q(triangle->u_Q16, triangle->v_Q16);


    if (GE3_DOT_PRODUCT_Q(triangle->normal_Q16, triangle->normal_Q16) < (GE_TO_Q(1) >> 5)){
        //printf("Warning! Small triangle!\n");
        triangle->valid = 0;
    }
    else{
        triangle->valid = 1;
    }

}
void Triangle3D_Make2Face( Triangle3DHeavy* triangle1,  Triangle3DHeavy* triangle2, ge_int3* fourCorners)
{
    triangle1->base.verts_Q16[0] = fourCorners[0];
    triangle1->base.verts_Q16[1] = fourCorners[1];
    triangle1->base.verts_Q16[2] = fourCorners[2];

    triangle2->base.verts_Q16[0] = fourCorners[2];
    triangle2->base.verts_Q16[1] = fourCorners[3];
    triangle2->base.verts_Q16[2] = fourCorners[0];

    Triangle3DMakeHeavy(triangle1);
    Triangle3DMakeHeavy(triangle2);
}



void MapTileConvexHull_From_TileData( ConvexHull* hull,  ge_uint* tileData)
{
    ge_int3 A = ge_int3{ GE_TO_Q(-1) >> 1, GE_TO_Q(-1) >> 1, GE_TO_Q(-1) >> 1 };
    ge_int3 B = ge_int3{ GE_TO_Q(1) >> 1, GE_TO_Q(-1) >> 1, GE_TO_Q(-1) >> 1 };
    ge_int3 C = ge_int3{ GE_TO_Q(1) >> 1, GE_TO_Q(1) >> 1, GE_TO_Q(-1) >> 1 };
    ge_int3 D = ge_int3{ GE_TO_Q(-1) >> 1, GE_TO_Q(1) >> 1, GE_TO_Q(-1) >> 1 };

    
    ge_int3 E = ge_int3{ GE_TO_Q(-1) >> 1, GE_TO_Q(-1) >> 1, GE_TO_Q(1) >> 1 };
    ge_int3 F = ge_int3{ GE_TO_Q(1) >> 1, GE_TO_Q(-1) >> 1, GE_TO_Q(1) >> 1 };
    ge_int3 G = ge_int3{ GE_TO_Q(1) >> 1, GE_TO_Q(1) >> 1, GE_TO_Q(1) >> 1 };
    ge_int3 H = ge_int3{ GE_TO_Q(-1) >> 1, GE_TO_Q(1) >> 1, GE_TO_Q(1) >> 1 };
    ge_int3 X = ge_int3{ 0, 0, GE_TO_Q(1) >> 1 };

    ge_ubyte lowCornerCount = MapDataLowCornerCount(*tileData);

    if (lowCornerCount > 0) 
    {
        if (GE_BITGET_MF(*tileData, MapTileFlags_LowCornerBTMLEFT) != 0)
            H.z = (GE_TO_Q(-1) >> 1);

        if (GE_BITGET_MF(*tileData, MapTileFlags_LowCornerBTMRIGHT) != 0)
            G.z = (GE_TO_Q(-1) >> 1);

        if (GE_BITGET_MF(*tileData, MapTileFlags_LowCornerTPLEFT) != 0)
            E.z = (GE_TO_Q(-1) >> 1);

        if (GE_BITGET_MF(*tileData, MapTileFlags_LowCornerTPRIGHT) != 0)
            F.z = (GE_TO_Q(-1) >> 1);


        //simple ramp cases
        ge_uint xlevel = MapDataXLevel(tileData);
        if(xlevel == 0)
        {
            X.z = 0;
        }
        else if(xlevel == 1)
        {
            X.z = GE_TO_Q(-1) >> 1;
        }



    }


    ge_int i = 0;
    //bottom (1)
    ge_int3 bottomFace[4];
    bottomFace[0] = A;
    bottomFace[1] = B;
    bottomFace[2] = C;
    bottomFace[3] = D;
    Triangle3D_Make2Face(&hull->triangles[i], &hull->triangles[i+1], &bottomFace[0]);
    i+=2;


    ge_int3 NegYFace[4];
    NegYFace[0] = A;
    NegYFace[1] = E;
    NegYFace[2] = F;
    NegYFace[3] = B;
    Triangle3D_Make2Face(&hull->triangles[i], &hull->triangles[i+1], &NegYFace[0]);
    i+=2;

    ge_int3 POSYFace[4];
    POSYFace[0] = C;
    POSYFace[1] = G;
    POSYFace[2] = H;
    POSYFace[3] = D;
    Triangle3D_Make2Face(&hull->triangles[i], &hull->triangles[i+1], &POSYFace[0]);
    i+=2;

    ge_int3 POSXFace[4];
    POSXFace[0] = B;
    POSXFace[1] = F;
    POSXFace[2] = G;
    POSXFace[3] = C;
    Triangle3D_Make2Face(&hull->triangles[i], &hull->triangles[i+1], &POSXFace[0]);
    i+=2;

    ge_int3 NEGXFace[4];
    NEGXFace[0] = D;
    NEGXFace[1] = H;
    NEGXFace[2] = E;
    NEGXFace[3] = A;
    Triangle3D_Make2Face(&hull->triangles[i], &hull->triangles[i+1], &NEGXFace[0]);
    i+=2;

    ge_int3 TOPFace1[4];
    TOPFace1[0] = X;
    TOPFace1[1] = G;
    TOPFace1[2] = F;
    TOPFace1[3] = E;
    Triangle3D_Make2Face(&hull->triangles[i], &hull->triangles[i+1], &TOPFace1[0]);
    i+=2;

    ge_int3 TOPFace2[4];
    TOPFace2[0] = X;
    TOPFace2[1] = E;
    TOPFace2[2] = H;
    TOPFace2[3] = G;
    Triangle3D_Make2Face(&hull->triangles[i], &hull->triangles[i+1], &TOPFace2[0]);
    i+=2;

}




void PeepPrint(Peep* peep)
{
    GE_PRINT_Q(peep->physics.drive.target_x_Q16);
}


void PeepPeepPhysics(ALL_CORE_PARAMS, Peep* peep, Peep* otherPeep)
{

    //calculate force based on penetration distance with otherPeep.
    ge_int3 d_Q16 = GE3_ADD(otherPeep->physics.base.pos_Q16, GE3_NEG(peep->physics.base.pos_Q16));
    ge_int combined_r_Q16 = peep->physics.shape.radius_Q16 + otherPeep->physics.shape.radius_Q16;
    ge_int len_Q16;


    ge_int3 penV_Q16 = d_Q16;
    penV_Q16 = GE3_NORMALIZED_Q(penV_Q16, len_Q16);

    if (len_Q16 > peep->physics.shape.radius_Q16 * 2)
        return;

    ge_int penetrationDist_Q16 = (len_Q16 - (combined_r_Q16));
    ge_int3 penetrationForce_Q16;


    //pos_post_Q16
    peep->physics.base.pos_post_Q16.x += GE_MUL_Q(penV_Q16.x, penetrationDist_Q16 >> 2);
    peep->physics.base.pos_post_Q16.y += GE_MUL_Q(penV_Q16.y, penetrationDist_Q16 >> 2);

    //otherPeep->physics.base.pos_post_Q16.x -= GE_MUL_Q(penV_Q16.x, penetrationDist_Q16 >> 1);
    //otherPeep->physics.base.pos_post_Q16.y -= GE_MUL_Q(penV_Q16.y, penetrationDist_Q16 >> 1);

    //peep->physics.base.pos_post_Q16.z += GE_MUL_Q(penV_Q16.z, penetrationDist_Q16 >> 1);//dont encourage peeps standing on each other


    //V' = V - penV*(V.penV)
    //DeltaV = -penV*(V.penV)

    ge_int dot = GE3_DOT_PRODUCT_Q(peep->physics.base.v_Q16, penV_Q16);
    
    if (dot > 0) {
        peep->physics.base.vel_add_Q16.x += -GE_MUL_Q(penV_Q16.x, dot);
        peep->physics.base.vel_add_Q16.y += -GE_MUL_Q(penV_Q16.y, dot);
        //peep->physics.base.vel_add_Q16.z += -GE_MUL_Q(penV_Q16.z, dot);//dont encourage peeps standing on each other

        //otherPeep->physics.base.vel_add_Q16.x -= -GE_MUL_Q(penV_Q16.x, dot);
        //otherPeep->physics.base.vel_add_Q16.y -= -GE_MUL_Q(penV_Q16.y, dot);
    }

    //spread messages
    if (peep->comms.orders_channel == otherPeep->comms.orders_channel)
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


    ge_int dist_Q16 = GE3_LENGTH_Q(GE3_ADD(peep->physics.base.pos_Q16, GE3_NEG(otherPeep->physics.base.pos_Q16)));

    if (dist_Q16 < peep->minDistPeep_Q16)
    {
        peep->minDistPeep_Q16 = dist_Q16;
        peep->minDistPeepPtr = otherPeep->ptr;
    }
    

    PeepPeepPhysics(ALL_CORE_PARAMS_PASS, peep, otherPeep);
}

void WorldToMap(ge_int3 world_Q16, ge_int3* out_map_tilecoords_Q16)
{

    ge_int3 b = { GE_TO_Q(MAP_TILE_SIZE) ,GE_TO_Q(MAP_TILE_SIZE) ,GE_TO_Q(MAP_TILE_SIZE) };
    ge_int3 map_coords_Q16 = GE3_DIV_Q(world_Q16, b);

    map_coords_Q16.x += (GE_TO_Q(mapDim) >> 1);//mapDim*0.5f
    map_coords_Q16.y += (GE_TO_Q(mapDim) >> 1);//mapDim*0.5f.
    *out_map_tilecoords_Q16 = map_coords_Q16;
}

void MapToWorld(ge_int3 map_tilecoords_Q16, ge_int3* world_Q16)
{
    ge_int3 b = { GE_TO_Q(MAP_TILE_SIZE) ,GE_TO_Q(MAP_TILE_SIZE) ,GE_TO_Q(MAP_TILE_SIZE) };

    map_tilecoords_Q16.x -= (GE_TO_Q(mapDim) >> 1);//mapDim*0.5f
    map_tilecoords_Q16.y -= (GE_TO_Q(mapDim) >> 1);//mapDim*0.5f

    *world_Q16 = GE3_MUL_Q(map_tilecoords_Q16, b);

}


void PeepGetMapTile(ALL_CORE_PARAMS, Peep* peep, ge_int3 offset, 
 MapTile* out_map_tile, 
 ge_int3* out_tile_world_pos_center_Q16,
  ge_int3* out_map_tile_coord_whole, 
   ge_uint* out_tile_data)
{
    (*out_map_tile_coord_whole).z = GE_WHOLE_Q(peep->posMap_Q16.z) + (offset.z);
    (*out_map_tile_coord_whole).x = GE_WHOLE_Q(peep->posMap_Q16.x) + (offset.x);
    (*out_map_tile_coord_whole).y = GE_WHOLE_Q(peep->posMap_Q16.y) + (offset.y);

    ge_int3 tileCoords_Q16;
    tileCoords_Q16.x = GE_TO_Q((* out_map_tile_coord_whole).x) + (GE_TO_Q(1) >> 1);//center of tile
    tileCoords_Q16.y = GE_TO_Q((* out_map_tile_coord_whole).y) + (GE_TO_Q(1) >> 1);//center of tile
    tileCoords_Q16.z = GE_TO_Q((* out_map_tile_coord_whole).z) + (GE_TO_Q(1) >> 1);//center of tile

    MapToWorld(tileCoords_Q16, out_tile_world_pos_center_Q16);


    if ((*out_map_tile_coord_whole).z < 0 || (*out_map_tile_coord_whole).z >= (mapDepth))
    {
        *out_map_tile = MapTile_NONE;
        return;
    }
    if ((*out_map_tile_coord_whole).x < 0 || (*out_map_tile_coord_whole).x >= (mapDim))
    {
        *out_map_tile = MapTile_NONE;
        return;
    }
    if ((*out_map_tile_coord_whole).y < 0 || (*out_map_tile_coord_whole).y >= (mapDim))
    {
        *out_map_tile = MapTile_NONE;
        return;
    }
    
    *out_tile_data = gameState->map.levels[(*out_map_tile_coord_whole).z].data[(*out_map_tile_coord_whole).x][(*out_map_tile_coord_whole).y];
    *out_map_tile = MapDataGetTile(*out_tile_data);
    
}

void RegionCollision(ge_int* out_pen_Q16, ge_int radius_Q16, ge_int W, ge_int lr)
{
    if (W > 0 && lr == -1)//left outside
    {
        *out_pen_Q16 = -(radius_Q16 - W);
        *out_pen_Q16 = GE_CLAMP(*out_pen_Q16, -(radius_Q16), 0);
    }
    else if (W < 0 && lr == 1)//right outside
    {
        *out_pen_Q16 = (radius_Q16 + W);
        *out_pen_Q16 = GE_CLAMP(*out_pen_Q16, 0, radius_Q16);
    }
    else if (W > 0 && lr == 1)//right inside
    {
        *out_pen_Q16 = (W + radius_Q16);
        *out_pen_Q16 = GE_CLAMP(*out_pen_Q16, 0, W + radius_Q16);
    }
    else if (W < 0 && lr == -1)//left inside
    {
        *out_pen_Q16 = (W - radius_Q16);
        *out_pen_Q16 = GE_CLAMP(*out_pen_Q16, W - radius_Q16, 0);
    }
}




ge_int3 MapTileConvexHull_ClosestPointToPoint( ConvexHull* hull, ge_int3 point_Q16)
{
    ge_int smallestDist_Q16 = GE_TO_Q(1000);
    ge_int3 closestPoint;
    for (ge_int i = 0; i < 14; i++)
    {
        Triangle3DHeavy* tri = &hull->triangles[i];
        if(tri->valid == 0)
            continue;

        ge_int dist_Q16;
        ge_int3 closest = Triangle3DHeavy_ClosestPoint(tri, point_Q16, &dist_Q16);
        //printf("Dist(%d): ", i);
        //GE_PRINT_Q(dist_Q16);
        if (dist_Q16 < smallestDist_Q16)
        {
            smallestDist_Q16 = dist_Q16;
            closestPoint = closest;
        }
    }
    //printf("Chosen Dist: ");
    //GE_PRINT_Q(smallestDist_Q16);
    //printf("Chosen Point: ");
    //GE3_PRINT_Q(closestPoint);
    return closestPoint;
}

ge_ubyte MapTileConvexHull_PointInside( ConvexHull* hull, ge_int3 point)
{
    //check dot product of point to verts against normal of the triangle
    for (ge_int i = 0; i < 14; i++)
    {
        Triangle3DHeavy* tri = &hull->triangles[i];
        if(tri->valid == 0)
            continue;

        for(ge_int v = 0; v < 3; v++)
        {
            ge_int3 vert = hull->triangles[i].base.verts_Q16[v];
            
            ge_int3 point_vert = GE3_SUB(point, vert);
            //ge_int mag;
            //ge_int3 point_vert_normalized = GE3_NORMALIZED_Q(point_vert, &mag);

            ge_int dot = GE3_DOT_PRODUCT_Q(point_vert, hull->triangles[i].normal_Q16);
            if(dot <= 0)
                return 0;
        }
    }

    return 1;
}

void PeepMapTileCollisions(ALL_CORE_PARAMS, Peep* peep)
{

    //maptile collisions
    MapTile tiles[26];
    ge_int3 tileCenters_Q16[26];
    ge_int3 dummy;
    ge_uint tileDatas[26];


    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 1, 0, 0 }, & tiles[0], & tileCenters_Q16[0],&dummy, &tileDatas[0]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { -1, 0, 0 }, & tiles[1], & tileCenters_Q16[1], &dummy, & tileDatas[1]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 0, -1, 0 }, & tiles[2], & tileCenters_Q16[2], &dummy, & tileDatas[2]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 0, 1, 0 }, & tiles[3], & tileCenters_Q16[3], &dummy, & tileDatas[3]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 0, 0, 1 }, & tiles[4], & tileCenters_Q16[4], &dummy, & tileDatas[4]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 0, 0, -1 }, & tiles[5], & tileCenters_Q16[5], &dummy, & tileDatas[5]);
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 0, 0, 0 }, & tiles[6], & tileCenters_Q16[6],&dummy, &tileDatas[6]);
    
    // {
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{0, 0, 0}, &tiles[6], &tileCenters_Q16[6], &dummy, &tileDatas[5]);

    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{1, 0, -1}, &tiles[6], &tileCenters_Q16[6], &dummy, &tileDatas[6]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{0, 1, -1}, &tiles[7], &tileCenters_Q16[7], &dummy, &tileDatas[7]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{1, 1, -1}, &tiles[8], &tileCenters_Q16[8], &dummy, &tileDatas[8]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{-1, 0, -1}, &tiles[9], &tileCenters_Q16[9], &dummy, &tileDatas[9]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{0, -1, -1}, &tiles[10], &tileCenters_Q16[10], &dummy, &tileDatas[10]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{-1, -1, -1}, &tiles[11], &tileCenters_Q16[11], &dummy, &tileDatas[11]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{1, -1, -1}, &tiles[12], &tileCenters_Q16[12], &dummy, &tileDatas[12]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{-1, 1, -1}, &tiles[13], &tileCenters_Q16[13], &dummy, &tileDatas[13]);

    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{1, 0, 1}, &tiles[14], &tileCenters_Q16[14], &dummy, &tileDatas[14]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{0, 1, 1}, &tiles[15], &tileCenters_Q16[15], &dummy, &tileDatas[15]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{1, 1, 1}, &tiles[16], &tileCenters_Q16[16], &dummy, &tileDatas[16]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{-1, 0, 1}, &tiles[17], &tileCenters_Q16[17], &dummy, &tileDatas[17]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{0, -1, 1}, &tiles[18], &tileCenters_Q16[18], &dummy, &tileDatas[18]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{-1, -1, 1}, &tiles[19], &tileCenters_Q16[19], &dummy, &tileDatas[19]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{1, -1, 1}, &tiles[20], &tileCenters_Q16[20], &dummy, &tileDatas[20]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{-1, 1, 1}, &tiles[21], &tileCenters_Q16[21], &dummy, &tileDatas[21]);

    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{1, 1, 0}, &tiles[22], &tileCenters_Q16[23], &dummy, &tileDatas[22]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{-1, 1, 0}, &tiles[23], &tileCenters_Q16[24], &dummy, &tileDatas[23]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{1, -1, 0}, &tiles[24], &tileCenters_Q16[25], &dummy, &tileDatas[24]);
    //     PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3{-1, -1, 0}, &tiles[25], &tileCenters_Q16[26], &dummy, &tileDatas[25]);
    // }
    
    //printf("peep Pos: "); GE3_PRINT_Q(peep->physics.base.pos_Q16);
    ConvexHull hull;//hull for use below
    for (ge_int i = 0; i < 7; i++)
    {
       
        MapTile tile = tiles[i];
        
        if (tile != MapTile_NONE)
        {
            ge_int3 futurePos;
            futurePos.x = (peep->physics.base.pos_Q16.x + peep->physics.base.pos_post_Q16.z) + peep->physics.base.v_Q16.x;
            futurePos.y = (peep->physics.base.pos_Q16.y + peep->physics.base.pos_post_Q16.y) + peep->physics.base.v_Q16.y;
            futurePos.z = (peep->physics.base.pos_Q16.z + peep->physics.base.pos_post_Q16.z) + peep->physics.base.v_Q16.z;

            ge_int3 nearestPoint;
            ge_ubyte insideSolidRegion;
  
            
            ge_int3 peepPosLocalToHull_Q16 = GE3_SUB(futurePos, tileCenters_Q16[i]);

            peepPosLocalToHull_Q16 = GE3_DIV_Q(peepPosLocalToHull_Q16, ge_int3 {
                GE_TO_Q(MAP_TILE_SIZE), GE_TO_Q(MAP_TILE_SIZE)
                    , GE_TO_Q(MAP_TILE_SIZE)
            });

            if(MapTileData_TileSolid(tileDatas[i]) == 0)
            {
                MapTileConvexHull_From_TileData(&hull, &tileDatas[i]);
                nearestPoint = MapTileConvexHull_ClosestPointToPoint(&hull, peepPosLocalToHull_Q16);
                insideSolidRegion = MapTileConvexHull_PointInside(&hull, peepPosLocalToHull_Q16);
            }
            else
            {
                nearestPoint.x = GE_CLAMP(peepPosLocalToHull_Q16.x,-(GE_TO_Q(1)>>1), (GE_TO_Q(1)>>1));
                nearestPoint.y = GE_CLAMP(peepPosLocalToHull_Q16.y,-(GE_TO_Q(1)>>1), (GE_TO_Q(1)>>1));
                nearestPoint.z = GE_CLAMP(peepPosLocalToHull_Q16.z,-(GE_TO_Q(1)>>1), (GE_TO_Q(1)>>1));

                insideSolidRegion = false;
                if(nearestPoint.x > -(GE_TO_Q(1)>>1) && nearestPoint.x < (GE_TO_Q(1)>>1))
                    if(nearestPoint.y > -(GE_TO_Q(1)>>1) && nearestPoint.y < (GE_TO_Q(1)>>1))
                        if(nearestPoint.z > -(GE_TO_Q(1)>>1) && nearestPoint.z < (GE_TO_Q(1)>>1))
                            insideSolidRegion = true;
            }


            peep->stateBasic.buriedGlitchState = insideSolidRegion;

            nearestPoint = GE3_MUL_Q(nearestPoint, ge_int3 {
                GE_TO_Q(MAP_TILE_SIZE), GE_TO_Q(MAP_TILE_SIZE)
                    , GE_TO_Q(MAP_TILE_SIZE)
            });

            nearestPoint = GE3_ADD(nearestPoint, tileCenters_Q16[i]);
        

            ge_int3 A;
            A.x = futurePos.x - nearestPoint.x;
            A.y = futurePos.y - nearestPoint.y;
            A.z = futurePos.z - nearestPoint.z;

            //make A vector always point to outside the shape
            if(insideSolidRegion==1){
               A = GE3_NEG(A);
               //printf("inside region!");
            }


            ge_int3 An = A;
            ge_int mag;
            An = GE3_NORMALIZED_Q(An, mag);


            if (mag < peep->physics.shape.radius_Q16)
            {
                ge_int dot;
                ge_int3 V = GE3_ADD(peep->physics.base.v_Q16 , peep->physics.base.vel_add_Q16);
                dot = GE3_DOT_PRODUCT_Q( V, An);
                ge_int3 B;//velocity to cancel
                B.x = GE_MUL_Q(An.x, dot);
                B.y = GE_MUL_Q(An.y, dot);
                B.z = GE_MUL_Q(An.z, dot);


                ge_int pushAmt;
                if(insideSolidRegion)
                    pushAmt = (peep->physics.shape.radius_Q16 + mag);
                else
                    pushAmt = (peep->physics.shape.radius_Q16 - mag);

                //corrections
                peep->physics.base.pos_post_Q16.z += GE_MUL_Q(An.z, pushAmt);
                peep->physics.base.pos_post_Q16.y += GE_MUL_Q(An.y, pushAmt);
                peep->physics.base.pos_post_Q16.x += GE_MUL_Q(An.x, pushAmt);

                
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
    //peep->physics.base.v_Q16.z += (GE_TO_Q(-1) >> 3);
}
ge_offsetPtr PeepCurrentOrder(Peep* peep)
{
    if(peep->stateBasic.orderStackIdx >= 0)
        return peep->stateBasic.orderPtrStack[peep->stateBasic.orderStackIdx];
    else
        return GE_OFFSET_NULL;

}
void PeepStartOrder(ALL_CORE_PARAMS, Peep* peep)
{
    
    Order* order;
    GE_OFFSET_TO_PTR(gameState->orders, PeepCurrentOrder(peep), order);
    if(order != NULL)
    {
        peep->physics.drive.targetPathNodeOPtr = order->pathToDestPtr;
    }
    else
    {
        peep->physics.drive.targetPathNodeOPtr = GE_OFFSET_NULL;
    }
}


void Peep_DetachFromAllOrders(ALL_CORE_PARAMS, Peep* peep)
{
    while(PeepCurrentOrder(peep) != GE_OFFSET_NULL)
    {
        Order* order;
        GE_OFFSET_TO_PTR(gameState->orders, PeepCurrentOrder(peep) , order);
        order->refCount--;
        peep->stateBasic.orderStackIdx--;
    }
    PeepStartOrder(ALL_CORE_PARAMS_PASS, peep);
}
void Peep_DetachFromOrder(ALL_CORE_PARAMS, Peep* peep)
{
    if(PeepCurrentOrder(peep) != GE_OFFSET_NULL)
    {
        Order* order;
        GE_OFFSET_TO_PTR(gameState->orders, PeepCurrentOrder(peep) , order);
        order->refCount--;
        peep->stateBasic.orderStackIdx--;
    }
    PeepStartOrder(ALL_CORE_PARAMS_PASS, peep);

}

void Peep_PushAssignOrder(ALL_CORE_PARAMS, Peep* peep, ge_offsetPtr orderPtr)
{
    peep->stateBasic.orderStackIdx++;
    peep->stateBasic.orderPtrStack[peep->stateBasic.orderStackIdx] = orderPtr;
    
    Order* order;
    GE_OFFSET_TO_PTR(gameState->orders, PeepCurrentOrder(peep) , order);
    order->refCount++;
    
    PeepStartOrder(ALL_CORE_PARAMS_PASS, peep);
}



void Order_DeleteOrder(ALL_CORE_PARAMS, ge_offsetPtr orderPtr)
{

    printf("deleting order.\n");
    Order* order;
    GE_OFFSET_TO_PTR(gameState->orders, orderPtr, order);

    order->valid = false;
    order->action = OrderAction_NONE;
    order->pendingDelete = false;
    if(order->pathToDestPtr != GE_OFFSET_NULL)
    {
        AStarPathSteps_DeletePath(ALL_CORE_PARAMS_PASS, order->pathToDestPtr);
        order->pathToDestPtr = GE_OFFSET_NULL;
    }


    
    //patch surrounding execution orders
    if(order->nextExecutionOrder != GE_OFFSET_NULL)
    {
        Order* nextExecOrder;
        GE_OFFSET_TO_PTR(gameState->orders, order->nextExecutionOrder, nextExecOrder);

        nextExecOrder->prevExecutionOrder = order->prevExecutionOrder;
        nextExecOrder->dirtyPathing = true;
    }

    if(order->prevExecutionOrder != GE_OFFSET_NULL)
    {
        Order* prevExecOrder;
        GE_OFFSET_TO_PTR(gameState->orders, order->prevExecutionOrder, prevExecOrder);

        prevExecOrder->nextExecutionOrder = order->nextExecutionOrder;
        prevExecOrder->dirtyPathing = true;
    }

    //next new pointer can be this one
    gameState->nextOrderIdx = orderPtr;
}

Machine* MachineGetFromMapCoord(ALL_CORE_PARAMS, ge_int3 mapCoord)
{
    ge_offsetPtr ptr = gameState->map.levels[mapCoord.z].machinePtr[mapCoord.x][mapCoord.y];
    Machine* machine;
    GE_OFFSET_TO_PTR(gameState->machines, ptr, machine);

    return machine;
}

void MapBuildTileView(ALL_CORE_PARAMS, ge_int x, ge_int y)
{
    ge_int3 coord = ge_int3{x,y,ThisClient(ALL_CORE_PARAMS_PASS)->mapZView };
    ge_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, coord);
    MapTile tile = MapDataGetTile(*data);
    MapTile tileUp;
    if (ThisClient(ALL_CORE_PARAMS_PASS)->mapZView < mapDepth-1)
    {
        coord.z = ThisClient(ALL_CORE_PARAMS_PASS)->mapZView + 1;
        ge_uint* dataup = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, coord);

        tileUp = MapDataGetTile(*dataup);
    }
    else
    {
        tileUp = MapTile_NONE;
    }

    mapTile1AttrVBO[y * mapDim + x] = 0;


    //look down...

    ge_int vz = 0;
    while (tile == MapTile_NONE)
    {
        data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, coord);
        tile = MapDataGetTile(*data);
        if (tile == MapTile_NONE) {
            coord.z--;
            #ifndef NO_ZSHADING
                vz++;
            #endif
        }
    }



    if(GE_BITGET(*data, MapTileFlags_Explored) == 0)
    {
        mapTile1VBO[y * mapDim + x ] = MapTile_NONE;//or "Haze"
        return;
    }


    ge_uint finalAttr=0;

    finalAttr |= GE_BITGET_MF(*data, MapTileFlags_LowCornerTPLEFT)       ;//A
    finalAttr |= GE_BITGET_MF(*data, MapTileFlags_LowCornerTPRIGHT)  << 1;//B
    finalAttr |= GE_BITGET_MF(*data, MapTileFlags_LowCornerBTMLEFT)  << 2;//C
    finalAttr |= GE_BITGET_MF(*data, MapTileFlags_LowCornerBTMRIGHT) << 3;//D

    ge_uint dataCpy = *data;
    ge_uint xlev = MapDataXLevel(&dataCpy);
    finalAttr |= (2-xlev) << 4;//X


    finalAttr |= (GE_CLAMP(15-vz-1, 0, 15) << 6);


    mapTile1AttrVBO[ y * mapDim + x ] = finalAttr;
    mapTile1OtherAttrVBO[ y * mapDim + x ] |= BITBANK_GET_SUBNUMBER_UINT(*data, MapTileFlags_RotBit1, 2);
    
    if (tileUp != MapTile_NONE)//view obstructed by foottile above.
    {

        //if next to visible show it as "wall view"
        mapTile1AttrVBO[y * mapDim + x ] = 0;
        ge_ubyte isWall = 0;
        ge_int dirOffsets[8] = {0,1,2,3,22,23,24,25}; 
        ge_int orthflags[4] = {0,0,0,0};
        for(ge_int i = 0; i < 4; i++)
        {
            ge_int3 offset = staticData->directionalOffsets[dirOffsets[i]];
            ge_int3 mapCoord = ge_int3{x +offset.x, y + offset.y, ThisClient(ALL_CORE_PARAMS_PASS)->mapZView + 1 };

            if(MapTileCoordValid(mapCoord,0))
            {
            
                ge_uint* dataoffup = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoord);
                MapTile tileoffup = MapDataGetTile(*dataoffup);
                
                if(tileoffup == MapTile_NONE)
                {
                    isWall = 1;

                    //test against mouse world coord
                    // ge_int3 mouseMapCoord;
                    // ge_int occluded;
                    // ge_int3 mouseWorld_Q16 = ge_int3{gameStateActions->mouseLocWorldx_Q16, gameStateActions->mouseLocWorldy_Q16, mapCoord.z};
                   
                    // WorldToMap(mouseWorld_Q16, &mouseMapCoord);
                    // mouseMapCoord = GE_INT3_WHOLE_ONLY_Q16(mouseMapCoord);
                    
                    // if(GE3_EQUAL(mouseMapCoord , mapCoord))
                    // {
                    //     mapTile1VBO[y * mapDim + x ] = tile;
                        
                    // }
                    // else
                    // {
                        mapTile1VBO[y * mapDim + x ] = tileUp;
                        
                    //}




                    if( i <=3 )
                        orthflags[i] = 1;

                    //fade out effect
                    


                    if((orthflags[0] + orthflags[1] + orthflags[2] + orthflags[3]) == 0)
                    {

                        //TODO better corner effect
                        // mapTile1AttrVBO[ y * mapDim + x ] |= 1<<4;

                        // if(dirOffsets[i] == 22) mapTile1AttrVBO[ y * mapDim + x ] |= 1 << 0;
                        // if(dirOffsets[i] == 23) mapTile1AttrVBO[ y * mapDim + x ] |= 1 << 1;
                        // if(dirOffsets[i] == 24) mapTile1AttrVBO[ y * mapDim + x ] |= 1 << 2;
                        // if(dirOffsets[i] == 25) mapTile1AttrVBO[ y * mapDim + x ] |= 1 << 3;
                    }


                    mapTile1AttrVBO[y * mapDim + x ] |= (GE_CLAMP(15-vz, 0, 15) << 6);//base shade
                    mapTile1AttrVBO[y * mapDim + x ] |= (1 << 10);//corners fade to nothing


                    

                }
            }
        }




        if(isWall == 0)
            mapTile1VBO[y * mapDim + x ] = MapTile_NONE;
    }
    else
    {
        mapTile1VBO[y * mapDim + x] = tile;
    }
}

void MapBuildTileView3Area(ALL_CORE_PARAMS, ge_int x, ge_int y)
{
    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x,  y);
    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x+1,  y);
    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x,  y+1);
    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x+1,  y+1);

    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x-1,  y);
    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x,  y-1);
    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x-1,  y-1);

    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x-1,  y+1);
    MapBuildTileView(ALL_CORE_PARAMS_PASS,  x+1,  y-1);
}

void MapUpdateShadow(ALL_CORE_PARAMS, ge_int x, ge_int y)
{
    if ((x < 1) || (x >= mapDim - 1) || (y < 1) || (y >= mapDim - 1))
    {
        return;
    }


    MapTile tile = MapTile_NONE;
    mapTile2VBO[y * mapDim + x] = MapTile_NONE;


    ge_int shadowIntensity;
    for (ge_int z = ThisClient(ALL_CORE_PARAMS_PASS)->mapZView+1; z >= 0; z--)
    {       
        if(z >= mapDepth) continue;

        ge_uint* data = &gameState->map.levels[z].data[x][y];
        MapTile center =MapDataGetTile(*data);

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


        ge_ubyte f = MapRidgeType(ALL_CORE_PARAMS_PASS, ge_int3 { x + 1, y, z }, ge_int3 { 1, 0, 0 });
        ge_ubyte h = MapRidgeType(ALL_CORE_PARAMS_PASS, ge_int3 { x , y+1, z }, ge_int3 { 0, 1, 0 });
        ge_ubyte e = MapRidgeType(ALL_CORE_PARAMS_PASS, ge_int3 { x - 1, y, z }, ge_int3 { -1, 0, 0 });
        ge_ubyte c = MapRidgeType(ALL_CORE_PARAMS_PASS, ge_int3 { x, y-1, z }, ge_int3 { 0, -1, 0 });


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


        mapTile2VBO[y * mapDim + x] = tile;
        
        ge_uint finalAttr = (GE_CLAMP(15-0, 0, 15) << 6);
        mapTile2AttrVBO[ y * mapDim + x ] = finalAttr;
    }


}

void MapDeleteTile(ALL_CORE_PARAMS, ge_int3 mapCoord)
{
    if (mapCoord.z > 0) 
    {
        gameState->map.levels[mapCoord.z].data[mapCoord.x][mapCoord.y] = MapTile_NONE;
        

        ge_offsetPtr machinePtr = gameState->map.levels[mapCoord.z].machinePtr[mapCoord.x][mapCoord.y];
        if(machinePtr != GE_OFFSET_NULL)
        {
            Machine* mach;
            GE_OFFSET_TO_PTR(gameState->machines, machinePtr, mach);
            mach->valid = false;
        }
        gameState->map.levels[mapCoord.z].machinePtr[mapCoord.x][mapCoord.y] = GE_OFFSET_NULL;


        MapBuildTileView3Area(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y);
        MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y);
        MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x + 1, mapCoord.y);
        MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x - 1, mapCoord.y);
        MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y + 1);
        MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y - 1);
    }
}

void PeepDrill(ALL_CORE_PARAMS, Peep* peep)
{
    ge_int3 mapCoord = GE3_CAST<ge_int3>(peep->mapCoord);
    mapCoord.z--;

    MapTile downTile = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, mapCoord);
    Machine* machine = MachineGetFromMapCoord(ALL_CORE_PARAMS_PASS, mapCoord);

    printf("digging...\n");

    if(downTile == MapTile_IronOre)
    {
        peep->inventory.counts[ItemType_IRON_ORE]+=10;
    }
    else if(downTile == MapTile_Rock)
    {
        peep->inventory.counts[ItemType_ROCK_DUST]+=100;
    }


    MapDeleteTile(ALL_CORE_PARAMS_PASS,  mapCoord);
}
void TransferInventory(Inventory* invFrom, Inventory* invTo)
{
    for(ge_int i = 0; i < ItemTypes_NUMITEMS; i++)
    {
        invTo->counts[i] += invFrom->counts[i];
        invFrom->counts[i] = 0;
    }
}

void PeepPull(ALL_CORE_PARAMS, Peep* peep)
{
    ge_int3 mapCoord = GE3_CAST<ge_int3>(peep->mapCoord);
    mapCoord.z--;

    MapTile downTile = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, mapCoord);
    Machine* machine = MachineGetFromMapCoord(ALL_CORE_PARAMS_PASS, mapCoord);

    TransferInventory(&machine->inventoryOut, &peep->inventory);
}
void PeepPush(ALL_CORE_PARAMS, Peep* peep)
{
    ge_int3 mapCoord = GE3_CAST<ge_int3>(peep->mapCoord);
    mapCoord.z--;

    MapTile downTile = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, mapCoord);
    Machine* machine = MachineGetFromMapCoord(ALL_CORE_PARAMS_PASS, mapCoord);

    TransferInventory(&peep->inventory, &machine->inventoryIn);
}


ge_offsetPtr Order_GetNewOrder(ALL_CORE_PARAMS)
{
    ge_offsetPtr ptr = gameState->nextOrderIdx;
    gameState->nextOrderIdx++;
    bool loopCheck = false;
    while( gameState->orders[gameState->nextOrderIdx].valid)
    {
        gameState->nextOrderIdx++;
        if(gameState->nextOrderIdx >= MAX_ORDERS)
        {
            gameState->nextOrderIdx = 0;
            if(loopCheck)
            {
                printf("Out of Order Space! (MAX_ORDERS)\n");
            }
            loopCheck = true;
        }
    }

    Order* order;
    GE_OFFSET_TO_PTR(gameState->orders, ptr, order);

    order->valid = true;
    order->ptr = ptr;
    order->pendingDelete = false;
    order->refCount = 0;
    order->prevExecutionOrder = GE_OFFSET_NULL;
    order->nextExecutionOrder = GE_OFFSET_NULL;
    order->pathToDestPtr = GE_OFFSET_NULL;
    order->selectingOrderLocation = false;
    order->destinationSet = false;
    order->aStarJobPtr = GE_OFFSET_NULL;

    return ptr;
}


ge_offsetPtr PeepOperate(ALL_CORE_PARAMS, Peep* peep, ge_offsetPtr nextOrderPtr)
{
    ge_int3 mapCoord = GE3_CAST<ge_int3>(peep->mapCoord);
    mapCoord.z--;

    MapTile downTile = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, mapCoord);
    Machine* machine = MachineGetFromMapCoord(ALL_CORE_PARAMS_PASS, mapCoord);

    MachineDesc* desc;
    GE_OFFSET_TO_PTR(gameState->machineDescriptions, machine->MachineDescPtr, desc);
    if(desc->type == MachineTypes_MINING_SITE )
    {
            printf("peep operating mining site\n");
            ge_int3 coord = GE3_CAST<ge_int3>(machine->mapTilePtr);
            coord.x += GE_UNIFORM_RANDOM_RANGE(gameStateActions->tickIdx, -2, 3);
                        coord.y += GE_UNIFORM_RANDOM_RANGE(gameStateActions->tickIdx+1, -2,3);
            if(coord.x == machine->mapTilePtr.x && coord.y == machine->mapTilePtr.y)
            {
             coord.x++;
             coord.y++;
            }


            MapTile tile = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, coord);
            while(tile == MapTile_NONE)
            {
                coord.z--;
                tile = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, coord);
            }
            coord.z++;

            {
                Order* nextOrder;
                GE_OFFSET_TO_PTR(gameState->orders, nextOrderPtr, nextOrder);

                //make order to return to next order destination after finish
                Order* returnOrder;
                ge_offsetPtr returnOrderPtr = Order_GetNewOrder(ALL_CORE_PARAMS_PASS);
                GE_OFFSET_TO_PTR(gameState->orders, returnOrderPtr, returnOrder);
                returnOrder->mapDest_Coord = nextOrder->mapDest_Coord;
                returnOrder->pathToDestPtr = AStarPathStepsNextFreePathNode(&gameState->paths);
                returnOrder->aStarJobPtr = AStarJob_EnqueueJob(ALL_CORE_PARAMS_PASS);
                returnOrder->nextExecutionOrder = nextOrder->nextExecutionOrder;
                returnOrder->action = nextOrder->action;

                AStarJob* returnJob;
                GE_OFFSET_TO_PTR(gameState->mapSearchJobQueue, returnOrder->aStarJobPtr, returnJob);

                returnJob->startLoc = GE3_CAST<ge_int3>( coord);
                returnJob->endLoc = nextOrder->mapDest_Coord;
                returnJob->pathPtr = returnOrder->pathToDestPtr;





                //make order to navigate to tile.
                Order* order;
                ge_offsetPtr orderPtr = Order_GetNewOrder(ALL_CORE_PARAMS_PASS);
                GE_OFFSET_TO_PTR(gameState->orders, orderPtr, order);
                order->mapDest_Coord = coord;
                order->pathToDestPtr = AStarPathStepsNextFreePathNode(&gameState->paths);
                order->aStarJobPtr = AStarJob_EnqueueJob(ALL_CORE_PARAMS_PASS);
                order->nextExecutionOrder = returnOrderPtr;
                order->action = OrderAction_MINE;

                AStarJob* job;
                GE_OFFSET_TO_PTR(gameState->mapSearchJobQueue, order->aStarJobPtr, job);

                job->startLoc = GE3_CAST<ge_int3>( machine->mapTilePtr);
                job->startLoc.z++;
                job->endLoc = coord;
                job->pathPtr = order->pathToDestPtr;






                return orderPtr;
            }
    }

    return GE_OFFSET_NULL;
}
void PeepDrivePhysics(ALL_CORE_PARAMS, Peep* peep)
{

    ge_int3 targetVelocity;

    ge_int3 d;
    d.x = peep->physics.drive.target_x_Q16 - peep->physics.base.pos_Q16.x;
    d.y = peep->physics.drive.target_y_Q16 - peep->physics.base.pos_Q16.y;
    d.z = peep->physics.drive.target_z_Q16 - peep->physics.base.pos_Q16.z;
    ge_int len;
    d = GE3_NORMALIZED_Q(d, len);

    if (len < GE_TO_Q(1))
    {
        d.x = GE_MUL_Q(d.x, len);
        d.y = GE_MUL_Q(d.y, len);
        d.z = GE_MUL_Q(d.z, len);
    }

    if(PeepCurrentOrder(peep) != GE_OFFSET_NULL)
    {
        Order* order;
        GE_OFFSET_TO_PTR(gameState->orders, PeepCurrentOrder(peep), order);

        if(!order->valid)
            Peep_DetachFromOrder(ALL_CORE_PARAMS_PASS, peep);
    

        if (GE_WHOLE_Q(len) < 2)//within range of current target
        {
           
            if(peep->physics.drive.targetPathNodeOPtr != GE_OFFSET_NULL)
            {

                //advance if theres room
                AStarPathNode* targetPathNode;
                GE_OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.targetPathNodeOPtr,targetPathNode);
                CL_CHECK_NULL(targetPathNode);

                AStarPathNode* prevpathNode;
                GE_OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.prevPathNodeOPtr,prevpathNode);

                
                //advance
                if(targetPathNode->completed)
                {

                    if(targetPathNode->nextOPtr != GE_OFFSET_NULL)
                    {

                        peep->physics.drive.prevPathNodeOPtr = peep->physics.drive.targetPathNodeOPtr;        
                        peep->physics.drive.targetPathNodeOPtr = targetPathNode->nextOPtr;

                        if (peep->physics.drive.targetPathNodeOPtr != GE_OFFSET_NULL ) 
                        {

                            AStarPathNode* targetPathNode;
                            GE_OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.targetPathNodeOPtr,targetPathNode);
                            CL_CHECK_NULL(targetPathNode);

                            ge_int3 nextTarget_Q16 = targetPathNode->mapCoord_Q16;
                            MapToWorld(nextTarget_Q16, &nextTarget_Q16);

                            peep->physics.drive.target_x_Q16 = nextTarget_Q16.x;
                            peep->physics.drive.target_y_Q16 = nextTarget_Q16.y;
                            peep->physics.drive.target_z_Q16 = nextTarget_Q16.z;
                        }
                    }
                    else
                    {
                        //final dest reached.
                        peep->comms.message_TargetReached_pending = 255;//send the message

                        if( order != NULL)
                        {
                            ge_offsetPtr nextOrder = order->nextExecutionOrder;

                            Peep_DetachFromOrder(ALL_CORE_PARAMS_PASS, peep);


                            //do the order action
                            if(order->action == OrderAction_MINE)
                            {
                                PeepDrill(ALL_CORE_PARAMS_PASS, peep);
                            }
                            else if(order->action == OrderAction_DROPOFF_MACHINE)
                            {
                                PeepPush(ALL_CORE_PARAMS_PASS, peep);
                            }
                            else if(order->action == OrderAction_PICKUP_MACHINE)
                            {
                                PeepPull(ALL_CORE_PARAMS_PASS, peep);
                            }
                            else if(order->action == OrderAction_OPERATE_MACHINE)
                            {
                                nextOrder = PeepOperate(ALL_CORE_PARAMS_PASS, peep, nextOrder);
                            }



                            
                            if((order->nextExecutionOrder == GE_OFFSET_NULL && order->prevExecutionOrder == GE_OFFSET_NULL))
                            {
                               // printf("peep detach from order\n");
                               
                            }
                            else//advance order
                            {

                                //printf("peep advancing to order %d\n", order->nextExecutionOrder);
                                Peep_PushAssignOrder(ALL_CORE_PARAMS_PASS, peep, nextOrder);
                            }
                            
                        }
                    }
                }
            }
            
        }
    }
   

    //drive
    if(peep->physics.drive.targetPathNodeOPtr != GE_OFFSET_NULL)
    {
        
        AStarPathNode* targetPathNode;
        GE_OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.targetPathNodeOPtr,targetPathNode);

        if(targetPathNode->completed)
        {
            //ge_uint* mapData = AStarPathNode_GetMapData(ALL_CORE_PARAMS_PASS, targetPathNode);

            //CL_CHECK_NULL(mapData);
            ge_int3 coord = GE3_WHOLE_Q(targetPathNode->mapCoord_Q16);
            ge_int peepCnt = gameState->map.levels[coord.z].peepCounts[coord.x][coord.y];

            //if(peepCnt < PEEP_PATH_CROWD /*|| GE3_WHOLE_Q(len) > 10 */)
            {
                targetVelocity.x = d.x >> 2;
                targetVelocity.y = d.y >> 2;
                targetVelocity.z = d.z >> 2;

                ge_int3 error = GE3_SUB( targetVelocity, peep->physics.base.v_Q16 );

                peep->physics.base.vel_add_Q16.x += error.x;
                peep->physics.base.vel_add_Q16.y += error.y;
                peep->physics.base.vel_add_Q16.z += error.z;
            }
        }
    }


    peep->physics.base.CS_angle_rad = GE_ATAN(((float)(d.x))/(1<<16), ((float)(d.y)) / (1 << 16));




}

void WalkAndFight(ALL_CORE_PARAMS, Peep* peep)
{

    //if search is done - start the peep on path
    if( PeepCurrentOrder(peep) != GE_OFFSET_NULL)
    {
        Order* order;
        GE_OFFSET_TO_PTR(gameState->orders, PeepCurrentOrder(peep) , order);
        CL_CHECK_NULL(order);

        //keep the order alive
        order->pendingDelete = false;

        if(order->pathToDestPtr != GE_OFFSET_NULL)
        {
            AStarPathNode* path;
            GE_OFFSET_TO_PTR(gameState->paths.pathNodes, order->pathToDestPtr, path);
            CL_CHECK_NULL(path);
                
            if(path->processing == false && peep->physics.drive.targetPathNodeOPtr == GE_OFFSET_NULL)
            {
                peep->physics.drive.targetPathNodeOPtr = order->pathToDestPtr;
                peep->physics.drive.prevPathNodeOPtr = GE_OFFSET_NULL;
            }
        }
    }


    if(peep->physics.drive.targetPathNodeOPtr != GE_OFFSET_NULL)
    {
        //drive to the next path node
        AStarPathNode* nxtPathNode;
        GE_OFFSET_TO_PTR(gameState->paths.pathNodes, peep->physics.drive.targetPathNodeOPtr, nxtPathNode);
        CL_CHECK_NULL(nxtPathNode);

            ge_int3 worldloc;
            MapToWorld(nxtPathNode->mapCoord_Q16, &worldloc);
            peep->physics.drive.target_x_Q16 = worldloc.x;
            peep->physics.drive.target_y_Q16 = worldloc.y;
            peep->physics.drive.target_z_Q16 = worldloc.z;


            //restrict comms to new channel
            peep->comms.orders_channel = GE_UNIFORM_RANDOM_RANGE(worldloc.x, 0, 10000);//broke
            peep->comms.message_TargetReached = 0;
            peep->comms.message_TargetReached_pending = 0;
    }



    PeepDrivePhysics(ALL_CORE_PARAMS_PASS, peep);
    PeepMapTileCollisions(ALL_CORE_PARAMS_PASS, peep);

}


void PeepPreUpdate1(ALL_CORE_PARAMS, Peep* peep)
{

}

void PeepPreUpdate2(ALL_CORE_PARAMS, Peep* peep)
{
    peep->physics.base.v_Q16.z += peep->physics.base.vel_add_Q16.z;
    peep->physics.base.v_Q16.y += peep->physics.base.vel_add_Q16.y;
    peep->physics.base.v_Q16.x += peep->physics.base.vel_add_Q16.x;

    peep->physics.base.vel_add_Q16.z = 0;
    peep->physics.base.vel_add_Q16.y = 0;
    peep->physics.base.vel_add_Q16.x = 0;




    peep->physics.base.pos_Q16.x += peep->physics.base.v_Q16.x;
    peep->physics.base.pos_Q16.y += peep->physics.base.v_Q16.y;
    peep->physics.base.pos_Q16.z += peep->physics.base.v_Q16.z;

    peep->physics.base.pos_Q16.z += peep->physics.base.pos_post_Q16.z;
    peep->physics.base.pos_Q16.y += peep->physics.base.pos_post_Q16.y;
    peep->physics.base.pos_Q16.x += peep->physics.base.pos_post_Q16.x;


    //hard map limits
    const ge_int lb = 1;
    peep->physics.base.pos_Q16.x = GE_CLAMP(peep->physics.base.pos_Q16.x, (-(MAP_TILE_SIZE*(((mapDim)/2)-lb)))<<16, (MAP_TILE_SIZE*(((mapDim)/2)-lb))<<16);
    peep->physics.base.pos_Q16.y = GE_CLAMP(peep->physics.base.pos_Q16.y, (-(MAP_TILE_SIZE*(((mapDim)/2)-lb)))<<16, (MAP_TILE_SIZE*(((mapDim)/2)-lb))<<16);
    peep->physics.base.pos_Q16.z = GE_CLAMP(peep->physics.base.pos_Q16.z, -(MAP_TILE_SIZE)<<16, (MAP_TILE_SIZE*(mapDepth))<<16);


    peep->physics.base.pos_post_Q16.z = 0;
    peep->physics.base.pos_post_Q16.y = 0;
    peep->physics.base.pos_post_Q16.x = 0;

    peep->physics.base.v_Q16.x = 0;
    peep->physics.base.v_Q16.y = 0;
    peep->physics.base.v_Q16.z = 0;

    if (peep->stateBasic.health <= 0)
        peep->stateBasic.deathState = 1;


    //peep comms
    /*
    if (peep->comms.message_TargetReached_pending)
    {
        peep->comms.message_TargetReached = peep->comms.message_TargetReached_pending;
        peep->comms.message_TargetReached--;//message fade
    }
    */



}

ge_int PeepMapVisiblity(ALL_CORE_PARAMS, Peep* peep, ge_int mapZViewLevel)
{
    #ifdef PEEP_ALL_ALWAYS_VISIBLE
        return 1;
    #endif


    ge_int3 maptilecoords;
    maptilecoords.x = GE_WHOLE_Q(peep->posMap_Q16.x);
    maptilecoords.y = GE_WHOLE_Q(peep->posMap_Q16.y);
    maptilecoords.z = GE_WHOLE_Q(peep->posMap_Q16.z);
    
    //search up to z level 0
    ge_int3 offset, tilePWorldCen, tileMapCoordWhole;
    MapTile tile;
    ge_uint tileData;
    offset.x = 0;
    offset.y = 0;
    offset.z = 0;
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, offset, &tile, &tilePWorldCen, &tileMapCoordWhole, &tileData);
    tile = MapDataGetTile(tileData);
    ge_ubyte firstTileOK = 0;
    if((MapDataLowCornerCount(tileData) > 0)  || tile == MapTile_NONE)
        firstTileOK = 1;

    tileMapCoordWhole.z++;
    tileData = gameState->map.levels[tileMapCoordWhole.z].data[tileMapCoordWhole.x][tileMapCoordWhole.y];
    tile = MapDataGetTile(tileData);

    while (firstTileOK && (tile == MapTile_NONE) && tileMapCoordWhole.z < mapDepth)
    {
        tileMapCoordWhole.z++;

        tileData = gameState->map.levels[tileMapCoordWhole.z].data[tileMapCoordWhole.x][tileMapCoordWhole.y];
        tile = MapDataGetTile(tileData);
    }
    //printf("%d\n", tileMapCoordWhole.z);

    if (tileMapCoordWhole.z == mapDepth)
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

void MachineUpdate(ALL_CORE_PARAMS,Machine* machine)
{
    MachineDesc* desc;
    GE_OFFSET_TO_PTR(gameState->machineDescriptions, machine->MachineDescPtr, desc);

    MachineRecipe* recip;
    GE_OFFSET_TO_PTR(gameState->machineRecipes, machine->recipePtr, recip);

    bool readyToProcess = true;
    for(ge_int i = 0; i < 8; i++)
    {
        ge_int ratio = recip->inputRatio[i];
        ItemTypes type = recip->inputTypes[i];
        if(ratio >= 1)
        {
            if(machine->inventoryIn.counts[type] >= ratio)
            {
                
            }
            else
            {
                readyToProcess = false;
            }
        }
    }

    if(readyToProcess && machine->state == MachineState_Running)
    {
        machine->tickProgess++;

        if(machine->tickProgess >= desc->processingTime)
        {
            machine->tickProgess = 0;
            MachineTypes type = desc->type;
            
          

            for(ge_int i = 0; i < 8; i++)
            {
                ge_int ratio = recip->inputRatio[i];
                ge_int outRatio = recip->outputRatio[i];
                ItemTypes type = recip->inputTypes[i];
                ItemTypes outType = recip->outputTypes[i];

                machine->inventoryIn.counts[type]-=ratio;
                machine->inventoryOut.counts[outType]+=outRatio;
            }
            
        }
    }



    //update command center order sequences
   if(desc->type == MachineTypes_COMMAND_CENTER)
   {
        if(machine->orderLen > 1)
        {

            Order* order;
            GE_OFFSET_TO_PTR(gameState->orders, machine->rootOrderPtr, order);

            ge_offsetPtr curOrderPtr = machine->rootOrderPtr;


            while(curOrderPtr != GE_OFFSET_NULL)
            {
                Order* curOrder;
                GE_OFFSET_TO_PTR(gameState->orders, curOrderPtr, curOrder);

                if(curOrder->destinationSet)
                {
                    Order* nextOrder;
                    GE_OFFSET_TO_PTR(gameState->orders, curOrder->nextExecutionOrder, nextOrder);

                    if(nextOrder != NULL)
                    {
                        if(nextOrder->destinationSet)
                        {
                            if(nextOrder->dirtyPathing)
                            {
                                if(nextOrder->pathToDestPtr != GE_OFFSET_NULL)
                                {
                                    AStarPathSteps_DeletePath(ALL_CORE_PARAMS_PASS, nextOrder->pathToDestPtr);
                                    nextOrder->pathToDestPtr = GE_OFFSET_NULL;
                                }
                                nextOrder->dirtyPathing = false;
                            }
                            if(nextOrder->pathToDestPtr == GE_OFFSET_NULL)
                            {
                                nextOrder->pathToDestPtr = AStarPathStepsNextFreePathNode(&gameState->paths);
                            }

                            AStarPathNode* pathNode;
                            GE_OFFSET_TO_PTR(gameState->paths.pathNodes, nextOrder->pathToDestPtr, pathNode);
                            
                            if((!pathNode->completed && !pathNode->processing))
                            {

                                printf("path: %d\n", nextOrder->pathToDestPtr);

                                pathNode->processing = true;

                                //enqueue job for path from previous order to this order
                                nextOrder->aStarJobPtr = AStarJob_EnqueueJob(ALL_CORE_PARAMS_PASS);

                                AStarJob* job;
                                GE_OFFSET_TO_PTR(gameState->mapSearchJobQueue, nextOrder->aStarJobPtr, job);


                                job->startLoc = curOrder->mapDest_Coord;
                                job->endLoc = nextOrder->mapDest_Coord;
                                job->pathPtr = nextOrder->pathToDestPtr;

                                nextOrder->pathToDestPtr = job->pathPtr;
                            }

                        }        
                    }
                }


                curOrderPtr = curOrder->nextExecutionOrder;


                if(curOrderPtr == machine->rootOrderPtr)
                   break;
            }


        }
   }
   else if(desc->type == MachineTypes_MINING_SITE)
   {
       





   }

    


}


void PeepUpdate(ALL_CORE_PARAMS, Peep* peep)
{

    peep->minDistPeep_Q16 = (1 << 30);
    peep->minDistPeepPtr = GE_OFFSET_NULL;


    ge_int x = ((peep->physics.base.pos_Q16.x >> 16) / (SECTOR_SIZE));
    ge_int y = ((peep->physics.base.pos_Q16.y >> 16) / (SECTOR_SIZE));


    MapSector* cursector = &(gameState->sectors[x + SQRT_MAXSECTORS / 2][y + SQRT_MAXSECTORS / 2]);
    CL_CHECK_NULL(cursector)


    //traverse sector
    ge_int minx = cursector->ptr.x - 1; if (minx == 0xFFFFFFFF) minx = 0;
    ge_int miny = cursector->ptr.y - 1; if (miny == 0xFFFFFFFF) miny = 0;

    ge_int maxx = cursector->ptr.x + 1; if (maxx >= SQRT_MAXSECTORS) maxx = SQRT_MAXSECTORS-1;
    ge_int maxy = cursector->ptr.y + 1; if (maxy >= SQRT_MAXSECTORS) maxy = SQRT_MAXSECTORS-1;
    
    for(ge_int sectorx = minx; sectorx <= maxx; sectorx++)
    {
        for (ge_int sectory = miny; sectory <= maxy; sectory++)
        {

            MapSector* sector = &gameState->sectors[sectorx][sectory];
            CL_CHECK_NULL(sector);


            for(ge_int i = 0; i < MAX_PEEPS_PER_SECTOR; i++)
            {
                if(sector->peepPtrs[i] == GE_OFFSET_NULL)
                    continue;

                Peep* otherPeep;
                GE_OFFSET_TO_PTR(gameState->peeps, sector->peepPtrs[i], otherPeep);
                
                if (otherPeep != peep) {
                    
                    PeepToPeepInteraction(ALL_CORE_PARAMS_PASS, peep, otherPeep);
                }
            }
        }
    }
   
    ge_int3 posMap_Q16;
    WorldToMap( peep->physics.base.pos_Q16, &posMap_Q16);
    peep->posMap_Q16 = posMap_Q16;

    ge_int3 maptilecoords;
    maptilecoords.x = GE_WHOLE_Q(peep->posMap_Q16.x);
    maptilecoords.y = GE_WHOLE_Q(peep->posMap_Q16.y);
    maptilecoords.z = GE_WHOLE_Q(peep->posMap_Q16.z);

    ge_int3 maptilecoords_prev;
    maptilecoords_prev.x = GE_WHOLE_Q(peep->lastGoodPosMap_Q16.x);
    maptilecoords_prev.y = GE_WHOLE_Q(peep->lastGoodPosMap_Q16.y);
    maptilecoords_prev.z = GE_WHOLE_Q(peep->lastGoodPosMap_Q16.z);


    //update visibility
    if (!GE3_EQUAL(maptilecoords, maptilecoords_prev) || ThisClient(ALL_CORE_PARAMS_PASS)->updateMap)
    {
        if (PeepMapVisiblity(ALL_CORE_PARAMS_PASS, peep, ThisClient(ALL_CORE_PARAMS_PASS)->mapZView))
        {     
            GE_BITSET(peep->stateBasic.bitflags0, PeepState_BitFlags_visible);
        }
        else
        {
            GE_BITCLEAR(peep->stateBasic.bitflags0, PeepState_BitFlags_visible);
        }
    }



    //update map coord tracking
    if(GE3_EQUAL(peep->mapCoord, maptilecoords))
    {



        peep->mapCoord_1 = peep->mapCoord;
        peep->mapCoord = GE3_CAST<ge_offsetPtrShort3>(maptilecoords);

        //printf("a");
            // ge_uint* mapData = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE3_CAST(peep->mapCoord, ge_int3));
            // peepCnt = MapTileData_PeepCount(*mapData);
            // MapTileData_SetPeepCount(mapData, peepCnt+1);
        
        //ge_int b = atomic_inc(&gameState->map.levels[peep->mapCoord.z].peepCounts[peep->mapCoord.x][peep->mapCoord.y]);
        //ge_int a = atomic_dec(&gameState->map.levels[peep->mapCoord_1.z].peepCounts[peep->mapCoord_1.x][peep->mapCoord_1.y]);

        //printf("%d,%d\n", a,b);
    }
        

    
    

    


    //revert position to last good if needed
    MapTile curTile;
    ge_uint tileData;
    ge_int3 dummy;
    ge_int3 dummy2;
    PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 0, 0, 0 }, &curTile, &dummy, & dummy2, & tileData);

    

    if (peep->stateBasic.buriedGlitchState != 0)
    {
        //revert to center of last good map position
        #ifndef PEEP_DISABLE_TILECORRECTIONS

            ge_int3 lastGoodPos;
            MapToWorld(peep->lastGoodPosMap_Q16, &lastGoodPos);

            peep->physics.base.pos_post_Q16.x += lastGoodPos.x - peep->physics.base.pos_Q16.x;
            peep->physics.base.pos_post_Q16.y += lastGoodPos.y - peep->physics.base.pos_Q16.y;
            peep->physics.base.pos_post_Q16.z += lastGoodPos.z - peep->physics.base.pos_Q16.z;

            peep->physics.base.vel_add_Q16.x = -peep->physics.base.v_Q16.x;
            peep->physics.base.vel_add_Q16.y = -peep->physics.base.v_Q16.y;
            peep->physics.base.vel_add_Q16.z = -peep->physics.base.v_Q16.z;

        #endif
    }
    else
    {

        peep->lastGoodPosMap_Q16 = peep->posMap_Q16;
    }
    
    WalkAndFight(ALL_CORE_PARAMS_PASS, peep);

}












void PrintSelectionPeepStats(ALL_CORE_PARAMS, Peep* p)
{
///    GE3_PRINT_Q(p->physics.base.pos_Q16);
    Peep* peep = p;
    MapTile data[22];
    ge_int3 tileCenters_Q16[22];
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 1, 0, 0 }, & data[0], & tileCenters_Q16[0]); printf("{ 1, 0, 0 }: %d\n", data[0]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { -1, 0, 0 }, & data[1], & tileCenters_Q16[1]); printf("{ -1, 0, 0 }: %d\n", data[1]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 0, -1, 0 }, & data[2], & tileCenters_Q16[2]); printf("{ 0, -1, 0 }: %d\n", data[2]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 0, 1, 0 }, & data[3], & tileCenters_Q16[3]); printf("{ 0, 1, 0 }: %d\n", data[3]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 0, 0, 1 }, & data[4], & tileCenters_Q16[4]); printf("{ 0, 0, 1 }: %d\n", data[4]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 0, 0, -1 }, & data[5], & tileCenters_Q16[5]); printf("{ 0, 0, -1 }: %d\n", data[5]);

    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 1, 0, -1 }, & data[6], & tileCenters_Q16[6]); printf("{ 1, 0, -1 }: %d\n", data[6]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 0, 1, -1 }, & data[7], & tileCenters_Q16[7]); printf("{ 0, 1, -1 }: %d\n", data[7]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 1, 1, -1 }, & data[8], & tileCenters_Q16[8]); printf("{ 1, 1, -1 }: %d\n", data[8]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { -1, 0, -1 }, & data[9], & tileCenters_Q16[9]); printf("{ -1, 0, -1 }: %d\n", data[9]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 0, -1, -1 }, & data[10], & tileCenters_Q16[10]); printf("{ 0, -1, -1 }: %d\n", data[10]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { -1, -1, -1 }, & data[11], & tileCenters_Q16[11]); printf("{ -1, -1, -1 }: %d\n", data[11]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 1, -1, -1 }, & data[12], & tileCenters_Q16[12]); printf("{ 1, -1, -1 }: %d\n", data[12]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { -1, 1, -1 }, & data[13], & tileCenters_Q16[13]); printf("{ -1, 1, -1 }: %d\n", data[13]);

    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 1, 0, 1 }, & data[14], & tileCenters_Q16[14]); printf("{ 1, 0, 1 }: %d\n", data[14]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 0, 1, 1 }, & data[15], & tileCenters_Q16[15]); printf("{ 0, 1, 1 }: %d\n", data[15]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 1, 1, 1 }, & data[16], & tileCenters_Q16[16]); printf("{ 1, 1, 1 }: %d\n", data[16]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { -1, 0, 1 }, & data[17], & tileCenters_Q16[17]); printf("{ -1, 0, 1 }: %d\n", data[17]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 0, -1, 1 }, & data[18], & tileCenters_Q16[18]); printf("{ 0, -1, 1 }: %d\n", data[18]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { -1, -1, 1 }, & data[19], & tileCenters_Q16[19]); printf("{ -1, -1, 1 }: %d\n", data[19]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { 1, -1, 1 }, & data[20], & tileCenters_Q16[20]); printf("{ 1, -1, 1 }: %d\n", data[20]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, ge_int3 { -1, 1, 1 }, & data[21], & tileCenters_Q16[21]); printf("{ -1, 1, 1 }: %d\n", data[21]);
}

void MapTileCoordClamp( ge_int3* mapCoord, ge_int xybuffer)
{
    (*mapCoord).x = GE_CLAMP((*mapCoord).x, xybuffer, mapDim - 1 - xybuffer);
    (*mapCoord).y = GE_CLAMP((*mapCoord).y, xybuffer, mapDim - 1 - xybuffer);
    (*mapCoord).z = GE_CLAMP((*mapCoord).z, 0, mapDepth - 1);
}


void GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS, ge_int2 world_Q16, 
 ge_int3* mapcoord_whole, 
 ge_int* occluded, ge_int zViewRestrictLevel)
{
    ge_int3 wrld_Q16;
    wrld_Q16.x = world_Q16.x;
    wrld_Q16.y = world_Q16.y;
    WorldToMap(wrld_Q16, &(*mapcoord_whole));
    (*mapcoord_whole).x = GE_WHOLE_Q((*mapcoord_whole).x);
    (*mapcoord_whole).y = GE_WHOLE_Q((*mapcoord_whole).y);


    MapTileCoordClamp(mapcoord_whole,1);

    for (ge_int z = zViewRestrictLevel; z >= 0; z--)
    {
        ge_uint* data = &gameState->map.levels[z].data[(*mapcoord_whole).x][(*mapcoord_whole).y];
        ge_uint dataCopy = *data;
        MapTile tile = MapDataGetTile(dataCopy);

        if (tile != MapTile_NONE)
        {
            (*mapcoord_whole).z = z;
            if (z == ThisClient(ALL_CORE_PARAMS_PASS)->mapZView-1)
            {
                *occluded = 1;
            }
            else if (z == ThisClient(ALL_CORE_PARAMS_PASS)->mapZView)
            {
                *occluded = 1;
            }
            else
            {
                *occluded = 0;
            }
            return;
        }
    }

   // printf("3");
    *occluded = 1;
}







void LINES_DrawLine(ALL_CORE_PARAMS, ge_float2 screenPosStart, ge_float2 screenPosEnd, ge_float3 color)
{
    if(gameState->debugLinesIdx+10 >= maxLines)
    {
        printf("out of debug line space!\n");
        return;
    }

    linesVBO[gameState->debugLinesIdx*5 + 0] = screenPosStart.x;
    linesVBO[gameState->debugLinesIdx*5 + 1] = screenPosStart.y;

    linesVBO[gameState->debugLinesIdx*5 + 2] = color.x;
    linesVBO[gameState->debugLinesIdx*5 + 3] = color.y;
    linesVBO[gameState->debugLinesIdx*5 + 4] = color.z;

    
    linesVBO[gameState->debugLinesIdx*5 + 5] = screenPosEnd.x;
    linesVBO[gameState->debugLinesIdx*5 + 6] = screenPosEnd.y;

    linesVBO[gameState->debugLinesIdx*5 + 7] = color.x;
    linesVBO[gameState->debugLinesIdx*5 + 8] = color.y;
    linesVBO[gameState->debugLinesIdx*5 + 9] = color.z;

    gameState->debugLinesIdx+=10;

}
void LINES_ClearAll(ALL_CORE_PARAMS)
{
    for(ge_int i = gameState->debugLinesIdx*10; i >=0; i--)
        linesVBO[i] = 0.0f;

    gameState->debugLinesIdx=0;;
}

ge_float4 Matrix_Float4_Times_Vec4(float mat[][4], ge_float4 vec)
{
    ge_float4 res;
    res.x = mat[0][0] * vec.x + mat[0][1] * vec.y + mat[0][2] * vec.z + mat[0][3] * vec.w;
    res.y = mat[1][0] * vec.x + mat[1][1] * vec.y + mat[1][2] * vec.z + mat[1][3] * vec.w;
    res.z = mat[2][0] * vec.x + mat[2][1] * vec.y + mat[2][2] * vec.z + mat[2][3] * vec.w;
    res.w = mat[3][0] * vec.x + mat[3][1] * vec.y + mat[3][2] * vec.z + mat[3][3] * vec.w;
    return res;
}

void LINES_DrawLineWorld(ALL_CORE_PARAMS, ge_float2 worldPosStart, ge_float2 worldPosEnd, ge_float3 color)
{
    ge_float4 worldPosStart4 = (ge_float4)(worldPosStart.x, worldPosStart.y, 0.0f, 1.0f);
    ge_float4 screenPosStart4 = Matrix_Float4_Times_Vec4(&gameStateActions->viewMatrix[0], worldPosStart4);
    ge_float2 screenPosStart2 = (ge_float2)(screenPosStart4.x, screenPosStart4.y);

    ge_float4 worldPosEnd4 = (ge_float4)(worldPosEnd.x, worldPosEnd.y, 0.0f, 1.0f);
    ge_float4 screenPosEnd4 = Matrix_Float4_Times_Vec4(&gameStateActions->viewMatrix[0], worldPosEnd4);
    ge_float2 screenPosEnd2 = (ge_float2)(screenPosEnd4.x, screenPosEnd4.y);

    // printf("[%f,%f,%f,%f]\n", gameStateActions->viewMatrix[0][0], gameStateActions->viewMatrix[0][1], gameStateActions->viewMatrix[0][2], gameStateActions->viewMatrix[0][3]);
    // printf("[%f,%f,%f,%f]\n", gameStateActions->viewMatrix[1][0], gameStateActions->viewMatrix[1][1], gameStateActions->viewMatrix[1][2], gameStateActions->viewMatrix[1][3]);
    // printf("[%f,%f,%f,%f]\n", gameStateActions->viewMatrix[2][0], gameStateActions->viewMatrix[2][1], gameStateActions->viewMatrix[2][2], gameStateActions->viewMatrix[2][3]);
    // printf("[%f,%f,%f,%f]\n", gameStateActions->viewMatrix[3][0], gameStateActions->viewMatrix[3][1], gameStateActions->viewMatrix[3][2], gameStateActions->viewMatrix[3][3]);
    // printf("---------------------\n");
   // gameStateActions->
    LINES_DrawLine(ALL_CORE_PARAMS_PASS,  screenPosStart2, screenPosEnd2,  color);
}

ge_int2 GUI_TO_WORLD_Q16(ALL_CORE_PARAMS, ge_int2 guiCoord)
{
    ge_float4 v;
    v.x = ge_float(guiCoord.x);
    v.y = ge_float(guiCoord.y);
    v.z = 0.0;
    v.w = 1.0f;
    ge_float4 worldPos = Matrix_Float4_Times_Vec4(&gameStateActions->viewMatrix_Inv[0], v);

    printf("%f, %f, %f, %f\n", worldPos.x, worldPos.y, worldPos.z, worldPos.w);

    return ge_int2{0,0};
}

ge_float2 TileToUV(TileUnion tile)
{

    //duplicate of geomMapTile.glsl code.
    ge_float2 uv;
    uv.x = ((ge_uint)tile.mapTile & 15u) / 16.0;
    uv.y = (((ge_uint)tile.mapTile >> 4u) & 15u) / 16.0;    
    return uv;
}


void PrintMouseState(ge_int mouseState)
{
    if(GE_BITGET(mouseState, MouseButtonBits_PrimaryDown))
        printf("MouseButtonBits_PrimaryDown |");
    if(GE_BITGET(mouseState, MouseButtonBits_PrimaryPressed))
        printf("MouseButtonBits_PrimaryPressed |");
    if(GE_BITGET(mouseState, MouseButtonBits_PrimaryReleased))
        printf("MouseButtonBits_PrimaryReleased |");
    if(GE_BITGET(mouseState, MouseButtonBits_SecondaryDown))
        printf("MouseButtonBits_SecondaryDown |");
    if(GE_BITGET(mouseState, MouseButtonBits_SecondaryPressed))
        printf("MouseButtonBits_SecondaryPressed |");
    if(GE_BITGET(mouseState, MouseButtonBits_SecondaryReleased))
        printf("MouseButtonBits_SecondaryReleased |");

    printf("\n");
}


void InventoryGui(ALL_CORE_PARAMS, GAMEGUI_PARAMS, Inventory* inventory)
{
    GUI_COMMON_WIDGET_START()

    if(!goodStart)
    {
        return;
    }

    ge_int j = 0;
    for(ge_int i = 0; i < ItemTypes_NUMITEMS; i++)
    {
        ge_int count = inventory->counts[i];
        if(count > 0)
        {
            const ge_int height = 40;
            ge_float2 uv = TileToUV(gameState->ItemTypeTiles[i]);
            GUI_IMAGE(GUIID_PASS, GE2_ADD(origPos , ge_int2(0,(j+1)*height + 100)), ge_int2(height,height), GuiFlags(0),  uv, GE2_ADD(uv , MAP_TILE_UV_WIDTH_FLOAT2), gameState->ItemColors[i]);


            LOCAL_STR(cntstr, "-----------");
            CL_ITOA(count, cntstr, cntstr_len, 10 );                 
            GUI_TEXT(GUIID_PASS, GE2_ADD(origPos , ge_int2(height,(j+1)*height + 100)), ge_int2(50,height), GuiFlags(0), cntstr);

            GUI_TEXT(GUIID_PASS, GE2_ADD(origPos , ge_int2(height+50,(j+1)*height + 100)), ge_int2(50,height), GuiFlags(0), (char*)&ItemTypeStrings[i][0]);
            j++;

        }
    }
}




void PeepCommandGui(ALL_CORE_PARAMS, SyncedGui* gui, SynchronizedClientState* client)
{
    if(client->selectedPeepPrimary != GE_OFFSET_NULL)
    {
        Peep* peep;
        GE_OFFSET_TO_PTR(gameState->peeps, client->selectedPeepPrimary, peep);

        LOCAL_STRL(mw, "Miner 123456", mwlen); 
        CL_ITOA(peep->ptr, (mw)+6,mwlen-6, 10);
        if(GUI_BEGIN_WINDOW(GUIID_PASS, &gui->guiState.windowPositions[2],
            &gui->guiState.windowSizes[2] , GuiFlags(0),  mw ))
        {

            if(peep->physics.drive.targetPathNodeOPtr != GE_OFFSET_NULL)
            {
                LOCAL_STR(thinkingtxt, "Traveling..."); 
                GUI_LABEL(GUIID_PASS, ge_int2(0,0), ge_int2(gui->guiState.windowSizes[2].x,20),  GuiFlags(0), thinkingtxt, ge_float3(0.0,0,0) );
            }

            if(PeepCurrentOrder(peep) != GE_OFFSET_NULL)
            {
                LOCAL_STR(descstr, "ORDER ------")
                CL_ITOA(PeepCurrentOrder(peep), descstr+6, 6, 10 );
                GUI_LABEL(GUIID_PASS, ge_int2(0,20), ge_int2(100,50),  GuiFlags(0), descstr, ge_float3(0,0,0));
            }

            

            
            ge_int3 mapCoord = GE3_CAST<ge_int3>(peep->mapCoord);
            mapCoord.z--;

            MapTile downTile = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, mapCoord);
            Machine* machine = MachineGetFromMapCoord(ALL_CORE_PARAMS_PASS, mapCoord);


            if(downTile != MapTile_NONE)
            {
                LOCAL_STR(dig, "Drill Down"); 
                if(GUI_BUTTON(GUIID_PASS, ge_int2(0,50), ge_int2(100,50), GuiFlags_Beveled, ge_float3(0.4,0.4,0.4), dig, NULL, NULL ))
                {
                    if(gui->passType == GuiStatePassType_Synced)
                    {
                        PeepDrill(ALL_CORE_PARAMS_PASS, peep);
                    }
                }

                if(machine != NULL)
                {
                    
                    MachineDesc* machDesc;
                    GE_OFFSET_TO_PTR(gameState->machineDescriptions, machine->MachineDescPtr, machDesc);

                    if(machDesc->type == MachineTypes_COMMAND_CENTER)
                    {
                        LOCAL_STR(str, "Link"); 
                        if(GUI_BUTTON(GUIID_PASS, ge_int2(0,50+50), ge_int2(gui->guiState.windowSizes[2].x,50),GuiFlags_Beveled,
                        ge_float3(0.4,0.4,0.4), str, NULL, NULL ))
                        {    
                            if(gui->passType == GuiStatePassType_Synced)
                            {

                                Order* machineRootOrder;
                                GE_OFFSET_TO_PTR(gameState->orders,machine->rootOrderPtr,  machineRootOrder);

                                //make path to the first machine order

                                Order* newOrder;
                                ge_offsetPtr newOrderPtr = Order_GetNewOrder(ALL_CORE_PARAMS_PASS);
                                GE_OFFSET_TO_PTR(gameState->orders, newOrderPtr, newOrder);
                                newOrder->valid = true;
                                newOrder->ptr = newOrderPtr;
                                newOrder->pendingDelete = false;
                                newOrder->mapDest_Coord = machineRootOrder->mapDest_Coord;
                                newOrder->prevExecutionOrder = GE_OFFSET_NULL;
                                newOrder->nextExecutionOrder = machine->rootOrderPtr;
                                newOrder->pathToDestPtr = AStarPathStepsNextFreePathNode(&gameState->paths);
                                newOrder->aStarJobPtr = AStarJob_EnqueueJob(ALL_CORE_PARAMS_PASS);



                                AStarJob* job;
                                GE_OFFSET_TO_PTR(gameState->mapSearchJobQueue, newOrder->aStarJobPtr, job);
                                job->startLoc = GE3_WHOLE_Q(peep->posMap_Q16);
                                job->endLoc = machineRootOrder->mapDest_Coord;
                                job->pathPtr = newOrder->pathToDestPtr;

                                Peep_DetachFromAllOrders(ALL_CORE_PARAMS_PASS, peep);
                                Peep_PushAssignOrder(ALL_CORE_PARAMS_PASS, peep, newOrderPtr);
                            }
                        }
                    }
                    else if(machDesc->type == MachineTypes_MINING_SITE)
                    {
                        LOCAL_STR(str, "Mine"); 
                        if(GUI_BUTTON(GUIID_PASS, ge_int2(0,50+50), ge_int2(gui->guiState.windowSizes[2].x,50),GuiFlags_Beveled,
                        ge_float3(0.4,0.4,0.4), str, NULL, NULL ))
                        {
                            PeepOperate(ALL_CORE_PARAMS_PASS, peep, GE_OFFSET_NULL);
                        }
                    }
                    else
                    {

                
                        LOCAL_STR(transferStr, "Push Down"); 
                        if(GUI_BUTTON(GUIID_PASS, ge_int2(0,50+50), ge_int2(gui->guiState.windowSizes[2].x,50),GuiFlags_Beveled,
                        ge_float3(0.4,0.4,0.4), transferStr, NULL, NULL ))
                        {    
                            if(gui->passType == GuiStatePassType_Synced)
                            {
                                PeepPush(ALL_CORE_PARAMS_PASS, peep);
                            }
                        }
                        LOCAL_STR(transferStr2, "Pull Up"); 
                        if(GUI_BUTTON(GUIID_PASS, ge_int2(0,50+50+50), ge_int2(gui->guiState.windowSizes[2].x,50),
                        GuiFlags_Beveled, ge_float3(0.4,0.4,0.4), transferStr2, NULL, NULL ))
                        {
                            if(gui->passType == GuiStatePassType_Synced)
                            {
                                PeepPull(ALL_CORE_PARAMS_PASS, peep);
                            }
                        }

                    }

                }

            }
            InventoryGui(ALL_CORE_PARAMS_PASS, GAMEGUI_PARAMS_PASS, ge_int2(0,50),gui->guiState.windowSizes[2], GuiFlags(0), &peep->inventory);

            GUI_END_WINDOW(GUIID_PASS);
        }
    }
}

float ANIMATION_BLINK(ALL_CORE_PARAMS)
{
    return (GE_SIN(gameStateActions->tickIdx*0.2f)+1)*0.5;
}


void CLIENT_PushTool( SynchronizedClientState* client, EditorTools tool){
    client->curTool_1 = client->curTool;
    client->curTool = tool;
}

void CLIENT_PopTool( SynchronizedClientState* client)
{
    client->curTool = client->curTool_1;
}

bool OrderEntryGui(ALL_CORE_PARAMS, GAMEGUI_PARAMS, Order* order,
SynchronizedClientState* client)
{
    GUI_COMMON_WIDGET_START();
    if(!goodStart)
        return false;

    if(order->valid)
    {
        GUI_DrawRectangle(GUI_CORE_PARAMS_PASS,
         gui, pos.x, pos.y, origSize.x-50, origSize.y, COLOR_DARKGRAY,
          gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE, false);

        LOCAL_STR(descstr, "ORDER -----------")
        CL_ITOA(order->ptr, descstr+6, 6, 10 );
        GUI_LABEL(GUIID_PASS, origPos,ge_int2(origSize.x-50, 20), GuiFlags(0), descstr, COLOR_GRAY);
        
        // LOCAL_STR(nxtstr, "NEXT  -----------")
        // CL_ITOA(order->nextExecutionOrder, nxtstr+6, 6, 10 );
        // GUI_LABEL(GUIID_PASS, origPos + ge_int2(0,20), origSize - ge_int2(50,1), 0, nxtstr, ge_float3(0,0,0));
        
        // LOCAL_STR(prevstr, "PREV  -----------")
        // CL_ITOA(order->prevExecutionOrder, prevstr+6, 6, 10 );
        // GUI_LABEL(GUIID_PASS, origPos + ge_int2(0,40), origSize - ge_int2(50,1), 0, prevstr, ge_float3(0,0,0));
        

        LOCAL_STR(locStr, "SET TARGET")
        LOCAL_STR(locStr2, "TARGETING..")
        LOCAL_STR(locStr3, "ASSIGNED")





        char* str;
        ge_float3 color = COLOR_ORANGE;
        if(client->curEditingOrderPtr == order->ptr && client->curEditingOrder_targeting)
        {
            str = locStr2;  
            color = COLOR_RED * ANIMATION_BLINK(ALL_CORE_PARAMS_PASS);

            if(client->tileTargetFound )
            {
                order->mapDest_Coord = client->tileTargetMapCoord;
                order->destinationSet = true;

                order->dirtyPathing = true;

                if(order->nextExecutionOrder != GE_OFFSET_NULL)
                {
                    Order* nextOrder;
                    GE_OFFSET_TO_PTR(gameState->orders, order->nextExecutionOrder, nextOrder);
                    nextOrder->dirtyPathing = true;
                }
                



                client->curEditingOrderPtr = GE_OFFSET_NULL;
                client->curEditingOrder_targeting = false;
                client->tileTargetFound = false;
            }
        }
        else if(order->destinationSet)
        {
            str = locStr3;  
            color = COLOR_GREEN;
        }
        else
        {
            str = locStr;
        }
        

        bool* toggle = &gui->fakeDummyBool;
        gui->fakeDummyBool = order->selectingOrderLocation;
        if(gui->passType == GuiStatePassType_Synced)
            toggle = &order->selectingOrderLocation;

        if(GUI_BUTTON(GUIID_PASS, origPos + ge_int2(0,20), ge_int2(100,50), GuiFlags_Beveled, color, str, NULL, NULL))
        {
            if(gui->passType == GuiStatePassType_Synced && gui->hoverWidget == gui->nextId-2)
            {
                client->curEditingOrderPtr = order->ptr;
                
                if(!client->curEditingOrder_targeting)
                {

                    printf("Entering Tile Select Mode");

                    CLIENT_PushTool(client, EditorTools_TileTargetSelect);
                    client->curEditingOrder_targeting = true;
                    client->tileTargetFound = false;
                }
                else
                {
                    CLIENT_PopTool(client);
                    client->curEditingOrder_targeting = false;
                }


            }
        }
        

        LOCAL_STR(drillStr, "DRILL")
        LOCAL_STR(pushStr, "PUSH")
        LOCAL_STR(pullStr, "PULL")
        LOCAL_STR(operateStr, "OPERATE")


        if(order->action == OrderAction_MINE)
            gui->fakeDummyBool = true;
        else
            gui->fakeDummyBool = false;

        toggle = &gui->fakeDummyBool;
        if(GUI_BUTTON(GUIID_PASS, GE2_ADD(origPos , ge_int2(100,20)), ge_int2(50,50), GuiFlags_Beveled, COLOR_GRAY, drillStr, NULL, toggle))
        {
            if(gui->passType == GuiStatePassType_Synced)
                order->action = OrderAction_MINE;
        }

        if(order->action == OrderAction_DROPOFF_MACHINE)
            gui->fakeDummyBool = true;
        else
            gui->fakeDummyBool = false;

        toggle = &gui->fakeDummyBool;
        if(GUI_BUTTON(GUIID_PASS, GE2_ADD(origPos , ge_int2(150,20)), ge_int2(50,50), GuiFlags_Beveled, COLOR_GRAY, pushStr, NULL, toggle))
        {
            if(gui->passType == GuiStatePassType_Synced)
                order->action = OrderAction_DROPOFF_MACHINE;
        }

        
        if(order->action == OrderAction_PICKUP_MACHINE)
            gui->fakeDummyBool = true;
        else
            gui->fakeDummyBool = false;

        toggle = &gui->fakeDummyBool;
        if(GUI_BUTTON(GUIID_PASS, GE2_ADD(origPos , ge_int2(200,20)), ge_int2(50,50), GuiFlags_Beveled, COLOR_GRAY, pullStr, NULL, toggle))
        {
            if(gui->passType == GuiStatePassType_Synced)
                order->action = OrderAction_PICKUP_MACHINE;
        }


        if(order->action == OrderAction_OPERATE_MACHINE)
            gui->fakeDummyBool = true;
        else
            gui->fakeDummyBool = false;

        toggle = &gui->fakeDummyBool;
        if(GUI_BUTTON(GUIID_PASS, GE2_ADD(origPos , ge_int2(250,20)), ge_int2(50,50), GuiFlags_Beveled, COLOR_GRAY, operateStr, NULL, toggle))
        {
            if(gui->passType == GuiStatePassType_Synced)
                order->action = OrderAction_OPERATE_MACHINE;
        }





        LOCAL_STR(delStr, "DELETE")
        if(GUI_BUTTON(GUIID_PASS, GE2_ADD(origPos , ge_int2(origSize.x-75,origSize.y/3)), ge_int2(75,origSize.y/3), GuiFlags_Beveled, COLOR_RED, delStr, NULL, NULL))
        {
            if(gui->passType == GuiStatePassType_Synced)
            {
                printf("deleting order by button\n");
                Order_DeleteOrder(ALL_CORE_PARAMS_PASS, order->ptr);
            }
        }
        LOCAL_STR(upStr, "UP")
        if(GUI_BUTTON(GUIID_PASS, origPos + ge_int2(origSize.x-75,0), ge_int2(75,origSize.y/3), GuiFlags_Beveled, COLOR_ORANGE, upStr, NULL, NULL))
        {
            if(gui->passType == GuiStatePassType_Synced)
            {
                Order* prevOrder;
                GE_OFFSET_TO_PTR(gameState->orders, order->prevExecutionOrder, prevOrder);

                Order* nextOrder;
                GE_OFFSET_TO_PTR(gameState->orders, order->nextExecutionOrder, nextOrder);
                
                if(nextOrder == prevOrder && nextOrder != NULL)
                {
                    return false;//list of 2
                }
                order->dirtyPathing = true;

                if(prevOrder != NULL)
                {
                    order->prevExecutionOrder = prevOrder->prevExecutionOrder;
                    prevOrder->nextExecutionOrder = order->nextExecutionOrder;
                    order->nextExecutionOrder = prevOrder->ptr;
                    

                    if(prevOrder->prevExecutionOrder != GE_OFFSET_NULL && prevOrder->prevExecutionOrder != order->ptr)
                    {
                        Order* prevPrevOrder;
                        GE_OFFSET_TO_PTR(gameState->orders, prevOrder->prevExecutionOrder, prevPrevOrder);

                        prevPrevOrder->nextExecutionOrder = order->ptr;
                        prevPrevOrder->dirtyPathing = true;
                    }
                    prevOrder->prevExecutionOrder = order->ptr;
                    prevOrder->dirtyPathing = true;

                    

                    if(nextOrder != NULL)
                    {
                        nextOrder->prevExecutionOrder = prevOrder->ptr;
                        nextOrder->dirtyPathing = true;
                    }
                }
            }
            printf("order moved Up.\n");
        }
        LOCAL_STR(downStr, "DOWN")
        if(GUI_BUTTON(GUIID_PASS, origPos + ge_int2(origSize.x-75,2*origSize.y/3), ge_int2(75,origSize.y/3), GuiFlags_Beveled, COLOR_ORANGE, downStr, NULL, NULL))
        {
            if(gui->passType == GuiStatePassType_Synced)
            {
                Order* prevOrder;
                GE_OFFSET_TO_PTR(gameState->orders, order->prevExecutionOrder, prevOrder);

                Order* nextOrder;
                GE_OFFSET_TO_PTR(gameState->orders, order->nextExecutionOrder, nextOrder);

                if(nextOrder == prevOrder && nextOrder != NULL)
                {
                    return false;//list of 2
                }
                order->dirtyPathing = true;
                if(nextOrder != NULL)
                {

                    nextOrder->prevExecutionOrder = order->prevExecutionOrder;
                    order->prevExecutionOrder = nextOrder->ptr;
                    order->nextExecutionOrder = nextOrder->nextExecutionOrder;
                    

                    if(nextOrder->nextExecutionOrder != GE_OFFSET_NULL && nextOrder->nextExecutionOrder != order->ptr)
                    {

                        Order* nextNextOrder;
                        GE_OFFSET_TO_PTR(gameState->orders, nextOrder->nextExecutionOrder, nextNextOrder);

                        nextNextOrder->prevExecutionOrder = order->ptr;
                        nextNextOrder->dirtyPathing = true;
                    }
                    nextOrder->nextExecutionOrder = order->ptr;
                    nextOrder->dirtyPathing = true;

                    if(prevOrder != NULL)
                    {
                        prevOrder->nextExecutionOrder = nextOrder->ptr;
                        prevOrder->dirtyPathing = true;
                    }
                }
            }
        }
        return true;

    }
    return false;
}

void OrderListGui(ALL_CORE_PARAMS, SyncedGui* gui, SynchronizedClientState* client)
{

        LOCAL_STR(orderWinStr, "Orders")
        if(GUI_BEGIN_WINDOW(GUIID_PASS, &gui->guiState.windowPositions[3],
        &gui->guiState.windowSizes[3] ,GuiFlags(0),  orderWinStr ))
        {
            LOCAL_STR(addstr, "ADD")
            if(GUI_BUTTON(GUIID_PASS, ge_int2(0,0), ge_int2(50,50), GuiFlags_Beveled, COLOR_GREEN, addstr, NULL, NULL))
            {

            }

            const ge_int entryHeight = 100;
            ge_int j = 0;
            for(ge_int i = 0; i < MAX_ORDERS; i++)
            {
                Order* order;
                GE_OFFSET_TO_PTR(gameState->orders, i, order);
                OrderEntryGui(ALL_CORE_PARAMS_PASS, GAMEGUI_PARAMS_PASS, ge_int2(0,(j+1)*entryHeight), ge_int2(gui->guiState.windowSizes[3].x, entryHeight), GuiFlags(0),  order, client);
                if(order->valid)
                    j++;
                
            }
            GUI_END_WINDOW(GUIID_PASS);
        }
}



void CommandCenterMachineGui(ALL_CORE_PARAMS, GAMEGUI_PARAMS, SynchronizedClientState* client, Machine* machine)
{
    GUI_COMMON_WIDGET_START()
    if(!goodStart)
        return ;

    Order* currentRootOrder;
    GE_OFFSET_TO_PTR(gameState->orders, machine->rootOrderPtr, currentRootOrder);
    if(currentRootOrder == NULL || !currentRootOrder->valid)
    {
        machine->rootOrderPtr = GE_OFFSET_NULL;
        GE_OFFSET_TO_PTR(gameState->orders, machine->rootOrderPtr, currentRootOrder);
    }



    LOCAL_STR(rstr, "Refresh")
    if(GUI_BUTTON(GUIID_PASS, ge_int2(100,0), ge_int2(100,50), GuiFlags_Beveled, COLOR_RED, rstr, NULL, NULL))
    {
            ge_offsetPtr curOrderPtr = machine->rootOrderPtr;

            while(curOrderPtr != GE_OFFSET_NULL)
            {
                Order* curOrder;
                GE_OFFSET_TO_PTR(gameState->orders, curOrderPtr, curOrder);

                curOrder->dirtyPathing = true;

                if(curOrder->nextExecutionOrder == machine->rootOrderPtr)
                    break;
                
                curOrderPtr = curOrder->nextExecutionOrder;
            }
    }



    LOCAL_STR(addstr, "New Order")
    if(GUI_BUTTON(GUIID_PASS, ge_int2(0,0), ge_int2(100,50), GuiFlags_Beveled, COLOR_GREEN, addstr, NULL, NULL))
    {
        if(gui->passType == GuiStatePassType_Synced)
        {
            ge_offsetPtr newOrderPTr = Order_GetNewOrder(ALL_CORE_PARAMS_PASS);
            Order* newOrder;
            GE_OFFSET_TO_PTR(gameState->orders, newOrderPTr, newOrder);


            if(currentRootOrder == NULL || !currentRootOrder->valid)
            {
                printf("new standalone order\n");
                machine->rootOrderPtr = newOrderPTr;
                newOrder->nextExecutionOrder = newOrderPTr;
                newOrder->prevExecutionOrder = newOrderPTr;
            }
            else
            {
                ge_offsetPtr endOrderPtr = currentRootOrder->prevExecutionOrder;
                Order* endOrder;
                GE_OFFSET_TO_PTR(gameState->orders, endOrderPtr, endOrder);

                endOrder->nextExecutionOrder = newOrderPTr;
                endOrder->dirtyPathing = true;
                currentRootOrder->prevExecutionOrder = newOrderPTr;
                currentRootOrder->dirtyPathing = true;
                
                newOrder->nextExecutionOrder = machine->rootOrderPtr;
                newOrder->prevExecutionOrder = endOrder->ptr;
            }

            

            newOrder->refCount = 1;



        }   
    }
    ge_int numOrders = 0;
    if(machine->rootOrderPtr != GE_OFFSET_NULL)
    {
        const ge_int entryHeight = 100;
        const ge_int entryGap = 10;
        ge_int totalHeight = machine->orderLen*(entryHeight+entryGap);

        if(GUI_SCROLLBOX_BEGIN(GUIID_PASS, ge_int2(0,50), ge_int2(origSize.x, origSize.y - 50), GuiFlags(0), ge_int2(1000,  totalHeight), &gui->guiState.scrollBoxes_x[0], &gui->guiState.scrollBoxes_y[0] ))
        {
            ge_offsetPtr firstOrder = machine->rootOrderPtr;
            ge_offsetPtr curOrder = machine->rootOrderPtr;

            ge_int j = 0;
            while( curOrder != GE_OFFSET_NULL )   
            {
                Order* order;
                GE_OFFSET_TO_PTR(gameState->orders, curOrder, order);

                OrderEntryGui(ALL_CORE_PARAMS_PASS, GAMEGUI_PARAMS_PASS, ge_int2(0,(j)*(entryHeight+entryGap)), ge_int2(origSize.x, entryHeight), GuiFlags(0),  order, client);
            
                numOrders++;

                if(order->valid)
                    j++;
                else
                    break;

                if(order->nextExecutionOrder == firstOrder)
                    break;
                curOrder = order->nextExecutionOrder;
            
            }

            GUI_SCROLLBOX_END(GUIID_PASS);
        }
            
        machine->orderLen = numOrders;
    }

}



void MachineGui(ALL_CORE_PARAMS, 
SyncedGui* gui,
SynchronizedClientState* client)
{
    if(client->selectedMachine != GE_OFFSET_NULL)
    {

        Machine* mach;
        GE_OFFSET_TO_PTR(gameState->machines, client->selectedMachine, mach);
        CL_CHECK_NULL(mach);

        MachineDesc* desc;
        GE_OFFSET_TO_PTR(gameState->machineDescriptions, mach->MachineDescPtr, desc);
        CL_CHECK_NULL(desc);

        MachineRecipe* recip;
        GE_OFFSET_TO_PTR(gameState->machineRecipes, mach->recipePtr, recip);

        LOCAL_STRL(mw, "Machine ------", mwlen); 
        CL_ITOA(client->selectedMachine, (mw)+8,mwlen-8, 10);


        
        gui->guiState.windowSizes[1] = ge_int2(600,600);


        if(GUI_BEGIN_WINDOW(GUIID_PASS, &gui->guiState.windowPositions[1],
            &gui->guiState.windowSizes[1],GuiFlags(0),  mw ))
        {
            

            if(desc->type == MachineTypes_COMMAND_CENTER)
            {
                CommandCenterMachineGui(ALL_CORE_PARAMS_PASS, GAMEGUI_PARAMS_PASS, ge_int2(0,0), gui->guiState.windowSizes[1],GuiFlags(0), client, mach);
            }
            else if(desc->type == MachineTypes_MINING_SITE)
            {

            }
            else
            {
                LOCAL_STRL(thinkingtxt2, "---", thinkingtxtLen); 
                float perc = ((float)mach->tickProgess/desc->processingTime);
                char* p = CL_ITOA(perc*100, thinkingtxt2, thinkingtxtLen, 10 );
                *(p+2) = '%'; if(thinkingtxt2[1] == '\0') thinkingtxt2[1] = ' ';
                GUI_LABEL(GUIID_PASS, ge_int2(0,0), ge_int2(perc*gui->guiState.windowSizes[1].x,50), GuiFlags(0), thinkingtxt2, ge_float3(0,0.7,0) );

                
                ge_int downDummy;
                LOCAL_STR(stateStrStart, "Start"); 
                LOCAL_STR(stateStrStop, "Stop"); 
                char* stateStr = stateStrStart;
                ge_float3 btnColor;

                if( mach->state == MachineState_Running )
                {
                    stateStr = stateStrStop;
                    btnColor = ge_float3(1.0,0.0,0.0);
                }
                else if( mach->state == MachineState_Idle )
                {
                    stateStr = stateStrStart;
                    btnColor = ge_float3(0.0,0.7,0.0);
                }

                if(GUI_BUTTON(GUIID_PASS, ge_int2(0,50), ge_int2(50,50), GuiFlags_Beveled, btnColor, stateStr, &downDummy, NULL))
                {
                    if(gui->passType == GuiStatePassType_Synced)
                    {    
                        if( mach->state == MachineState_Running)
                        {
                            mach->state = MachineState_Idle;
                        }
                        else if( mach->state == MachineState_Idle)
                        {
                            mach->state = MachineState_Running;
                        }

                        
                    }
                }
                if(GUI_BUTTON(GUIID_PASS, ge_int2(50,50), ge_int2(50,50), GuiFlags_Beveled,ge_float3(0,0,1.0), NULL, &downDummy, NULL))
                {
                    for(ge_int i = 0; i < 8; i++)
                    {
                        if(recip->inputTypes[i] != ItemType_INVALID_ITEM)
                            mach->inventoryIn.counts[recip->inputTypes[i]]+=20;
                    }
                }


                InventoryGui(ALL_CORE_PARAMS_PASS, GAMEGUI_PARAMS_PASS,  ge_int2(0,0), gui->guiState.windowSizes[1], GuiFlags(0) , &mach->inventoryIn );    
                InventoryGui(ALL_CORE_PARAMS_PASS, GAMEGUI_PARAMS_PASS,  ge_int2(0,100), gui->guiState.windowSizes[1], GuiFlags(0), &mach->inventoryOut);            
            }



           
            GUI_END_WINDOW(GUIID_PASS);
        }
    }
}


void GuiCode(ALL_CORE_PARAMS, 
SynchronizedClientState* client, 
ClientAction* clientAction, 
SyncedGui* gui, 
bool guiIsLocalClient, 
ge_int2 mouseLoc,
ge_int mouseState,
ge_int cliId)
{


    GUI_RESET(GUI_CORE_PARAMS_PASS, gui, mouseLoc, mouseState, gui->passType, guiIsLocalClient);

    ge_int downDummy;
    char btntxt[9] = "CLICK ME"; 
    btntxt[8] = '\0';
    

    //handle hidden tools
    if(client->curTool == EditorTools_TileTargetSelect)
    {
        for(ge_int i = 0; i < NUM_EDITOR_MENU_TABS; i++)
            gui->guiState.menuToggles[0] = false;
    }


    LOCAL_STR(noneTxt, "SELECT");
    if(GUI_BUTTON(GUIID_PASS, ge_int2{0 ,0}, ge_int2{100, 50}, GuiFlags_Beveled, GUI_BUTTON_COLOR_DEF, noneTxt, &downDummy, &(gui->guiState.menuToggles[0])) == 1)
    {
        client->curTool = EditorTools_Select;
        GUI_UpdateToggleGroup(gui->guiState.menuToggles, NUM_EDITOR_MENU_TABS, 0);
    }
    LOCAL_STR(deleteTxt, "DELETE");
    if(GUI_BUTTON(GUIID_PASS, ge_int2{100 ,0}, ge_int2{100, 50},GuiFlags_Beveled, GUI_BUTTON_COLOR_DEF, deleteTxt, &downDummy, &(gui->guiState.menuToggles[1])) == 1)
    {
        //printf("delete mode.");
        client->curTool = EditorTools_Delete;
        GUI_UpdateToggleGroup(gui->guiState.menuToggles, NUM_EDITOR_MENU_TABS, 1);
    }

    LOCAL_STR(createTxt, "CREATE\nCRUSHER");
    if(GUI_BUTTON(GUIID_PASS, ge_int2{200 ,0}, ge_int2{100, 50}, GuiFlags_Beveled, GUI_BUTTON_COLOR_DEF, createTxt, &downDummy, &(gui->guiState.menuToggles[2])) == 1)
    {
        //  printf("create mode");
        client->curTool = EditorTools_Create;
        client->curToolMachine = MachineTypes_CRUSHER;
        GUI_UpdateToggleGroup(gui->guiState.menuToggles, NUM_EDITOR_MENU_TABS, 2);
    }

    LOCAL_STR(createTxt3, "CREATE\nSMELTER");
    if(GUI_BUTTON(GUIID_PASS, ge_int2{300 ,0}, ge_int2{100, 50}, GuiFlags_Beveled, GUI_BUTTON_COLOR_DEF, createTxt3, &downDummy, &(gui->guiState.menuToggles[3])) == 1)
    {
        // printf("create mode");
        client->curTool = EditorTools_Create;
        client->curToolMachine = MachineTypes_SMELTER;

        GUI_UpdateToggleGroup(gui->guiState.menuToggles, NUM_EDITOR_MENU_TABS, 3);
    }

    LOCAL_STR(createTxt2, "CREATE\nCOMMAND CENTER");
    if(GUI_BUTTON(GUIID_PASS, ge_int2{400 ,0}, ge_int2{100, 50}, GuiFlags_Beveled, GUI_BUTTON_COLOR_DEF, createTxt2, &downDummy, &(gui->guiState.menuToggles[4])) == 1)
    {
        // printf("create mode");
        client->curTool = EditorTools_Create;
        client->curToolMachine = MachineTypes_COMMAND_CENTER;

        GUI_UpdateToggleGroup(gui->guiState.menuToggles, NUM_EDITOR_MENU_TABS, 4);
    }
    LOCAL_STR(createTxt4, "CREATE\nMINING SITE");
    if(GUI_BUTTON(GUIID_PASS, ge_int2{500 ,0}, ge_int2{100, 50}, GuiFlags_Beveled, GUI_BUTTON_COLOR_DEF, createTxt4, &downDummy, &(gui->guiState.menuToggles[5])) == 1)
    {
        // printf("create mode");
        client->curTool = EditorTools_Create;
        client->curToolMachine = MachineTypes_MINING_SITE;

        GUI_UpdateToggleGroup(gui->guiState.menuToggles, NUM_EDITOR_MENU_TABS, 5);
    }


    LOCAL_STRL(labeltxt, "DEEP", labeltxtLen); 
    GUI_LABEL(GUIID_PASS, ge_int2{0 ,50}, ge_int2{80 ,50}, GuiFlags(0), labeltxt, ge_float3(0.3,0.3,0.3));


    if(GUI_SLIDER_INT_VERTICAL(GUIID_PASS,  ge_int2{0 ,100}, ge_int2{80, 800}, GuiFlags(0), &client->mapZView, 0, mapDepth))
    {
        //printf("sliding, %d\n", client->mapZView);
    }

    LOCAL_STRL(labeltxt2, "BIRDS\nEYE", labeltxt2Len); 
    GUI_LABEL(GUIID_PASS, ge_int2{0 ,900}, ge_int2{80 ,50}, GuiFlags(0), labeltxt2, ge_float3(0.3,0.3,0.3));
        

    LOCAL_STRL(robotSelWindowStr, "Selected Robots", robotSelWindowStrLen); 
    if(GUI_BEGIN_WINDOW(GUIID_PASS,  &gui->guiState.windowPositions[0],
    &gui->guiState.windowSizes[0],GuiFlags(0),  robotSelWindowStr ))
    {
        if(GUI_SCROLLBOX_BEGIN(GUIID_PASS, ge_int2{0,0},
        ge_int2{10,10},
        GuiFlags_FillParent,
        ge_int2{1000,1000}, &gui->guiState.menuScrollx, &gui->guiState.menuScrolly))
        {
            //iterate selected peeps
            Peep* p;
            GE_OFFSET_TO_PTR(gameState->peeps, client->selectedPeepsLastIdx, p);
                
            ge_int i = 0;
            while(p != NULL)
            {

                LOCAL_STRL(header, "Miner: ", headerLen); 
                LOCAL_STRL(peeptxt, "Select", peeptxtLen); 
                GUI_LABEL(GUIID_PASS, ge_int2{0 ,50*i}, ge_int2{50, 50}, GuiFlags(0), header, ge_float3(0.3,0.3,0.3));
        
                if(GUI_BUTTON(GUIID_PASS, ge_int2{50 ,50*i}, ge_int2{50, 50},GuiFlags_Beveled,GUI_BUTTON_COLOR_DEF,  peeptxt, &downDummy, NULL))
                {
                    client->selectedPeepPrimary = p->ptr;
                }

                i++;    
                GE_OFFSET_TO_PTR(gameState->peeps, p->prevSelectionPeepPtr[cliId], p);
            }
            GUI_SCROLLBOX_END(GUIID_PASS);
        }
        
        GUI_END_WINDOW(GUIID_PASS);
    }

    //hover stats
    if((gui->passType == GuiStatePassType_NoLogic) && (GUI_MOUSE_ON_GUI(gui) == 0))
    {

        ge_int2 world_Q16;
        world_Q16.x = gameStateActions->mouseLocWorldx_Q16;
        world_Q16.y = gameStateActions->mouseLocWorldy_Q16;

        //world to map
        //get tile etc.
        ge_int3 mapcoord_whole;
        ge_int occluded;
        GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world_Q16, &mapcoord_whole, &occluded, client->mapZView+1);



        MapTile tileup = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, mapcoord_whole + (ge_int3)(0,0,1));
        MapTile tile = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, mapcoord_whole);
        MapTile tiledown = MapGetTileFromCoord(ALL_CORE_PARAMS_PASS, mapcoord_whole + (ge_int3)(0,0,-1));
        



        LOCAL_STRL(xtxt, "", xtxtLen); 
        //CL_ITOA(tile, xtxt, xtxtLen, 10 );
        //GUI_LABEL(GUIID_PASS, ge_int2{300,200}, ge_int2{100, 50}, xtxt, ge_float3(0.3,0.3,0.3));
        const ge_int widgx = 80;
        const ge_int widgy = 800;
        

        GUI_LABEL(GUIID_PASS, ge_int2(widgx-5,widgy-50-5) , ge_int2{50+10, 150+10}, GuiFlags(0), xtxt, ge_float3(0.3,0.3,0.3));
        ge_float2 uv = TileToUV(TileUnion{tileup});
        GUI_IMAGE(GUIID_PASS, ge_int2(widgx,widgy-50) , ge_int2{50, 50}, GuiFlags(0), uv, uv + MAP_TILE_UV_WIDTH_FLOAT2, ge_float3(1,1,1));

        uv = TileToUV(TileUnion{tile});
        GUI_IMAGE(GUIID_PASS, ge_int2(widgx,widgy) , ge_int2{50, 50}, GuiFlags(0), uv, uv + MAP_TILE_UV_WIDTH_FLOAT2, ge_float3(1,1,1));

        uv = TileToUV(TileUnion{tiledown});

        GUI_IMAGE(GUIID_PASS, ge_int2(widgx,widgy+50) , ge_int2{50, 50},GuiFlags(0),  uv, uv + MAP_TILE_UV_WIDTH_FLOAT2, ge_float3(1,1,1));

        
    }


    if(gameState->mapSearchers[0].state == AStarPathFindingProgress_Searching)
    {
        LOCAL_STRL(thinkingtxt, "FINDING PATH..", thinkingtxtLen); 
        GUI_LABEL(GUIID_PASS, ge_int2(400,100), ge_int2(150,50), GuiFlags(0), thinkingtxt, ge_float3(1.0,0,0) );
    }



    //selected machine
    MachineGui(ALL_CORE_PARAMS_PASS,  gui, client);
    
    //selected single peep
    PeepCommandGui(ALL_CORE_PARAMS_PASS, gui, client);

    //order list
    OrderListGui(ALL_CORE_PARAMS_PASS, gui, client);


    if(gui->passType == GuiStatePassType_Synced)
        printf("cli: %d, mapz: %d\n", cliId, client->mapZView);
    else{
        //  printf("(fakepass) cli: %d, mapz: %d\n", cliId, client->mapZView);
    }


    //selection box
    GUI_RESET_POST(GUI_CORE_PARAMS_PASS,  gui);




}




void game_hover_gui(ALL_CORE_PARAMS)
{

    ge_int cliId = gameStateActions->clientId;
    SynchronizedClientState* client =  ThisClient(ALL_CORE_PARAMS_PASS);
    ClientAction* clientAction = &gameStateActions->clientActions[cliId].action;
    ActionTracking* actionTracking = &gameStateActions->clientActions[cliId].tracking;
    SyncedGui* gui = &gameState->fakePassGui;
    gui->passType = GuiStatePassType_NoLogic;
    ge_int2 mouseLoc = ge_int2{gameStateActions->mouseLocx, gameStateActions->mouseLocy };
    ge_int mouseState = gameStateActions->mouseState;
    bool guiIsLocalClient = true;


    GuiCode(ALL_CORE_PARAMS_PASS, 
        client, 
        clientAction, 
        gui, 
        guiIsLocalClient, 
        mouseLoc, 
        mouseState,  
        cliId);
}




void game_apply_actions(ALL_CORE_PARAMS)
{
    //apply turns
    for (ge_int a = 0; a < gameStateActions->numActions; a++)
    {

        ge_int b = a;
        GuiStatePassType guiPass = GuiStatePassType_Synced;


        ClientAction* clientAction = &gameStateActions->clientActions[b].action;
        ActionTracking* actionTracking = &gameStateActions->clientActions[b].tracking;
        ge_int cliId = actionTracking->clientId;
        SynchronizedClientState* client = &gameState->clientStates[cliId];
        SyncedGui* gui = &gameState->clientStates[cliId].gui;
        

        //detect new clients
        if(gameState->numClients < cliId+1)
        {
            printf("New Client Connected!\n");
            gameState->numClients = cliId+1;
        }

        ge_int2 mouseLoc;
        ge_int mouseState;
        bool guiIsLocalClient = false;
        
        gui->passType = GuiStatePassType_Synced;
        mouseLoc.x = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_X];
        mouseLoc.y = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_Y];
        mouseState = clientAction->intParameters[CAC_MouseStateChange_Param_BUTTON_BITS];

        if(client == ThisClient(ALL_CORE_PARAMS_PASS))
            guiIsLocalClient = true;

        PrintMouseState( mouseState);
        printf("IsLocalClient: %d\n", guiIsLocalClient);
        printf("a = %d\n", a);

        GuiCode(ALL_CORE_PARAMS_PASS, 
        client, 
        clientAction, 
        gui, 
        guiIsLocalClient, 
        mouseLoc, 
        mouseState, 
        cliId);

        if(guiPass == GuiStatePassType_NoLogic)
            continue;

        
        if (clientAction->actionCode == ClientActionCode_MouseStateChange)
        {
            
            printf("Processing Action From Client: %d\n", cliId);
            ge_int buttons = clientAction->intParameters[CAC_MouseStateChange_Param_BUTTON_BITS];



            
            //end selection
            if(GE_BITGET_MF(buttons, MouseButtonBits_PrimaryPressed) && (GUI_MOUSE_ON_GUI(gui) == 0))
            {

                printf("Starting Drag Selection..\n");
                client->mouseGUIBegin.x = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_X];
                client->mouseGUIBegin.y = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_Y];
                client->mouseWorldBegin_Q16.x = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16];
                client->mouseWorldBegin_Q16.y = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16];

            }
            else if(GE_BITGET(buttons, MouseButtonBits_PrimaryReleased) && (gui->draggedOff == 0) && client->curTool == EditorTools_Select)
            {
                printf("Ending Drag Selection..\n");
                
                client->mouseGUIEnd.x = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_X];
                client->mouseGUIEnd.y = clientAction->intParameters[CAC_MouseStateChange_Param_GUI_Y];
                client->mouseWorldEnd_Q16.x = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16];
                client->mouseWorldEnd_Q16.y = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16];
                
                //sort the selection box
                ge_int nex = GE_MAX(client->mouseWorldEnd_Q16.x, client->mouseWorldBegin_Q16.x);
                ge_int ney  = GE_MAX(client->mouseWorldEnd_Q16.y, client->mouseWorldBegin_Q16.y);

                ge_int nsx  = GE_MIN(client->mouseWorldEnd_Q16.x, client->mouseWorldBegin_Q16.x);
                ge_int nsy  = GE_MIN(client->mouseWorldEnd_Q16.y, client->mouseWorldBegin_Q16.y);
                client->mouseWorldEnd_Q16.x = nex;
                client->mouseWorldEnd_Q16.y = ney;
                client->mouseWorldBegin_Q16.x = nsx;
                client->mouseWorldBegin_Q16.y = nsy;


                {
                    client->selectedPeepsLastIdx = GE_OFFSET_NULL;
                    client->selectedPeepPrimary = GE_OFFSET_NULL;
                    ge_int selectionCount = 0;
                    for (ge_uint pi = 0; pi < maxPeeps; pi++)
                    {


                        Peep* p = &gameState->peeps[pi];
                        p->prevSelectionPeepPtr[cliId] = GE_OFFSET_NULL;
                        p->nextSelectionPeepPtr[cliId] = GE_OFFSET_NULL;


                        if (p->stateBasic.faction == actionTracking->clientId)
                        {
                            if ((p->physics.base.pos_Q16.x > client->mouseWorldBegin_Q16.x)
                            && (p->physics.base.pos_Q16.x < client->mouseWorldEnd_Q16.x))
                            {

                                if ((p->physics.base.pos_Q16.y < client->mouseWorldEnd_Q16.y)
                                    && (p->physics.base.pos_Q16.y > client->mouseWorldBegin_Q16.y))
                                {
                                    if (PeepMapVisiblity(ALL_CORE_PARAMS_PASS, p, client->mapZView))
                                    {

                                        if (client->selectedPeepsLastIdx != GE_OFFSET_NULL)
                                        {
                                            gameState->peeps[client->selectedPeepsLastIdx].nextSelectionPeepPtr[cliId] = pi;
                                            p->prevSelectionPeepPtr[cliId] = client->selectedPeepsLastIdx;
                                            p->nextSelectionPeepPtr[cliId] = GE_OFFSET_NULL;
                                        }
                                        client->selectedPeepsLastIdx = pi;
                                        client->selectedPeepPrimary = pi;
                                        selectionCount++;
                                        PrintSelectionPeepStats(ALL_CORE_PARAMS_PASS, p);

                                    }

                                }
                            }
                        }

                    }

                    if(selectionCount != 1)
                    {
                        client->selectedPeepPrimary = GE_OFFSET_NULL;
                    }

                }

            }

            
            //command to location
            if(GE_BITGET(buttons, MouseButtonBits_SecondaryReleased) && (GUI_MOUSE_ON_GUI(gui) == 0) 
            && (gui->draggedOff == 0))
            {
                ge_uint curPeepIdx = client->selectedPeepsLastIdx;
                ge_int3 mapcoord;
                ge_int2 world2D;
                AStarPathFindingProgress pathFindProgress;

                ge_offsetPtr pathOPtr;
                if (curPeepIdx != GE_OFFSET_NULL)
                {
                    //Do an AStarSearch_IDA
                    Peep* curPeep = &gameState->peeps[curPeepIdx];
                    world2D.x = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16];
                    world2D.y = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16];
                    ge_int occluded;

                    GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world2D, &mapcoord, &occluded, client->mapZView);
                    mapcoord.z++;
                   
                    ge_int3 firstPeepMapCoord = GE3_WHOLE_Q(curPeep->posMap_Q16);




                    ge_int3 start;
                    start = firstPeepMapCoord;
                    
                    printf("start: ");GE3_PRINT(start);
                    printf("end: ");  GE3_PRINT(mapcoord);

                    //create order imeddiate, and queue up path creation.
                    Order* newMainOrder;
                    ge_offsetPtr mainPathOrderPtr = Order_GetNewOrder(ALL_CORE_PARAMS_PASS);
                    GE_OFFSET_TO_PTR(gameState->orders, mainPathOrderPtr, newMainOrder);
                    newMainOrder->valid = true;
                    newMainOrder->ptr = mainPathOrderPtr;
                    newMainOrder->pendingDelete = false;
                    newMainOrder->mapDest_Coord = mapcoord;
                    newMainOrder->prevExecutionOrder = GE_OFFSET_NULL;
                    newMainOrder->nextExecutionOrder = GE_OFFSET_NULL;
                    newMainOrder->pathToDestPtr = AStarPathStepsNextFreePathNode(&gameState->paths);
                    newMainOrder->aStarJobPtr = AStarJob_EnqueueJob(ALL_CORE_PARAMS_PASS);
                    

                    AStarJob* job;
                    GE_OFFSET_TO_PTR(gameState->mapSearchJobQueue, newMainOrder->aStarJobPtr, job);
                    job->startLoc = start;
                    job->endLoc = mapcoord;
                    job->pathPtr = newMainOrder->pathToDestPtr;










                    while (curPeepIdx != GE_OFFSET_NULL)
                    {
                        Peep* curPeep = &gameState->peeps[curPeepIdx];
                        

                        start = GE3_WHOLE_Q(curPeep->posMap_Q16);
                        
                        //create order for local paths
                        Order* newOrder;
                        ge_offsetPtr newOrderPtr = Order_GetNewOrder(ALL_CORE_PARAMS_PASS);
                        GE_OFFSET_TO_PTR(gameState->orders, newOrderPtr, newOrder);
                        newOrder->valid = true;
                        newOrder->ptr = newOrderPtr;
                        newOrder->pendingDelete = false;
                        newOrder->mapDest_Coord = firstPeepMapCoord;
                        newOrder->prevExecutionOrder = GE_OFFSET_NULL;
                        newOrder->nextExecutionOrder = GE_OFFSET_NULL;
                        newOrder->pathToDestPtr = AStarPathStepsNextFreePathNode(&gameState->paths);
                        newOrder->aStarJobPtr = AStarJob_EnqueueJob(ALL_CORE_PARAMS_PASS);


                        AStarJob* job;
                        GE_OFFSET_TO_PTR(gameState->mapSearchJobQueue, newOrder->aStarJobPtr, job);
                        job->startLoc = start;
                        job->endLoc = firstPeepMapCoord;
                        job->pathPtr = newOrder->pathToDestPtr;

                        Peep_DetachFromAllOrders(ALL_CORE_PARAMS_PASS, curPeep);
                        Peep_PushAssignOrder(ALL_CORE_PARAMS_PASS, curPeep, mainPathOrderPtr);
                        Peep_PushAssignOrder(ALL_CORE_PARAMS_PASS, curPeep, newOrderPtr);

                        curPeepIdx = curPeep->prevSelectionPeepPtr[cliId];
                    }

                }


            }
            
            //delete
            if(GE_BITGET(buttons, MouseButtonBits_PrimaryReleased) && (GUI_MOUSE_ON_GUI(gui) == 0) 
            && (gui->draggedOff == 0) && client->curTool == EditorTools_Delete)
            {
                ge_int2 world2DMouse;
                world2DMouse.x = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16];
                world2DMouse.y = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16];



                ge_int3 mapCoord;
                ge_int occluded;
                GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world2DMouse, &mapCoord, &occluded, client->mapZView+1);
                
                GE3_PRINT(mapCoord);
                MapDeleteTile(ALL_CORE_PARAMS_PASS, mapCoord);
               

            }

            //create
            if(GE_BITGET(buttons, MouseButtonBits_PrimaryReleased) && (GUI_MOUSE_ON_GUI(gui) == 0) 
            && (gui->draggedOff == 0) && client->curTool == EditorTools_Create)
            {
                ge_int2 world2DMouse;
                world2DMouse.x = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16];
                world2DMouse.y = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16];

                ge_int3 mapCoord;
                ge_int occluded;
                GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world2DMouse, &mapCoord, &occluded, client->mapZView+1);
                

                GE3_PRINT(mapCoord);
                if (mapCoord.z >= 0 && mapCoord.z < mapDepth-1) 
                {
                    ge_int3 mapCoordSpawn;
                    mapCoordSpawn.x = mapCoord.x;
                    mapCoordSpawn.y = mapCoord.y;
                    mapCoordSpawn.z = mapCoord.z+1;

                    ge_uint* tileData = &gameState->map.levels[mapCoordSpawn.z].data[mapCoordSpawn.x][mapCoordSpawn.y]; 

                    if(MapDataGetTile(*tileData) == MapTile_NONE)
                    {

                        ge_offsetPtr machinePtr = Machine_CreateMachine(ALL_CORE_PARAMS_PASS);
                        gameState->map.levels[mapCoordSpawn.z].machinePtr[mapCoordSpawn.x][mapCoordSpawn.y] = machinePtr;

                        Machine* machine;
                        GE_OFFSET_TO_PTR(gameState->machines, machinePtr, machine);
                        CL_CHECK_NULL(machine);

                        machine->valid = true;
                        machine->mapTilePtr = GE3_CAST<ge_offsetPtrShort3>(mapCoordSpawn);
                        machine->MachineDescPtr = client->curToolMachine;
                        machine->recipePtr = gameState->validMachineRecipes[client->curToolMachine][0];


                        MachineDesc* machDesc;
                        GE_OFFSET_TO_PTR(gameState->machineDescriptions, machine->MachineDescPtr, machDesc);
                        CL_CHECK_NULL(machDesc);

                        *tileData = machDesc->tile;
                        GE_BITSET(*tileData, MapTileFlags_Explored);
                    }
                    


                    MapBuildTileView3Area(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y);
                    MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y);
                    MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x + 1, mapCoord.y);
                    MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x - 1, mapCoord.y);
                    MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y + 1);
                    MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y - 1);
                }

            }

            //select tile
            if(GE_BITGET(buttons, MouseButtonBits_PrimaryReleased) && (GUI_MOUSE_ON_GUI(gui) == 0) 
            && (gui->draggedOff == 0) && (client->curTool == EditorTools_Select || client->curTool == EditorTools_TileTargetSelect))
            {
                ge_int2 world2DMouse;
                world2DMouse.x = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_X_Q16];
                world2DMouse.y = clientAction->intParameters[CAC_MouseStateChange_Param_WORLD_Y_Q16];

                ge_int3 mapCoord;
                ge_int occluded;
                GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world2DMouse, &mapCoord, &occluded, client->mapZView+1);
                

                
                client->selectedMapCoord = GE3_CAST<ge_short3>(mapCoord);

                ge_offsetPtr machinePtr = gameState->map.levels[mapCoord.z].machinePtr[mapCoord.x][mapCoord.y];



                if(machinePtr != GE_OFFSET_NULL)
                {
                    Machine* machine;
                    GE_OFFSET_TO_PTR(gameState->machines, machinePtr, machine);
                    printf("machine selected\n");
                }

                if(client->curTool == EditorTools_Select)
                    client->selectedMachine = machinePtr;

                if(client->curTool == EditorTools_TileTargetSelect)
                {
                    printf("target selected.\n");
                    client->tileTargetMapCoord = mapCoord;
                    client->tileTargetMapCoord.z++;
                    client->tileTargetFound = true;
                }

            }




        }



       
   
    }

    gameStateActions->numActions = 0;
}

ge_int3 MapTileWholeToMapTileCenterQ16(ge_int x, ge_int y, ge_int z)
{
    ge_int3 mapCoordsTileCenter_Q16 = ge_int3{GE_TO_Q(x) + (GE_TO_Q(1) >> 1),
    GE_TO_Q(y) + (GE_TO_Q(1) >> 1) ,
    GE_TO_Q(z) + (GE_TO_Q(1) >> 1) };
    return mapCoordsTileCenter_Q16;
}

void MapCreateSlope(ALL_CORE_PARAMS, ge_int x, ge_int y)
{
    ge_int3 world_Q16;
    ge_int3 mapCoords2D_Q16 = MapTileWholeToMapTileCenterQ16(x, y, 0);


    MapToWorld(mapCoords2D_Q16, &world_Q16);
    ge_int2 world2d_Q16 = ge_int2{world_Q16.x, world_Q16.y };

    ge_int3 mapCoordWhole;//top tile
    ge_int occluded;
    GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world2d_Q16,
        &mapCoordWhole, &occluded, mapDepth - 1);

    ge_uint* tileData = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, mapCoordWhole);
    //do 3x3 kernel test


    //offsets[22] = ge_int3{ 1, 1, 0 };
    //offsets[23] = ge_int3{ -1, 1, 0 };
    //offsets[24] = ge_int3{ 1, -1, 0 };
    //offsets[25] = ge_int3{ -1, -1, 0 };

    ge_uint* data22 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE3_ADD(mapCoordWhole, staticData->directionalOffsets[22]));
    MapTile tile22 = MapDataGetTile(*data22);
    if (tile22 == MapTile_NONE)
    {
        GE_BITSET(*tileData, MapTileFlags_LowCornerBTMRIGHT);
    }
    ge_uint* data24 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE3_ADD(mapCoordWhole, staticData->directionalOffsets[24]));
    MapTile tile24 = MapDataGetTile(*data24);
    if (tile24 == MapTile_NONE)
    {
        GE_BITSET(*tileData, MapTileFlags_LowCornerTPRIGHT);
    }
    ge_uint* data23 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE3_ADD(mapCoordWhole, staticData->directionalOffsets[23]));
    MapTile tile23 = MapDataGetTile(*data23);
    if (tile23 == MapTile_NONE)
    {
        GE_BITSET(*tileData, MapTileFlags_LowCornerBTMLEFT);
    }
    ge_uint* data25 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE3_ADD(mapCoordWhole, staticData->directionalOffsets[25]));
    MapTile tile25 = MapDataGetTile(*data25);
    if (tile25 == MapTile_NONE)
    {
        GE_BITSET(*tileData, MapTileFlags_LowCornerTPLEFT);
    }








    //offsets[0] = ge_int3{ 1, 0, 0 };
    //offsets[1] = ge_int3{ -1, 0, 0 };
    //offsets[2] = ge_int3{ 0, -1, 0 };
    //offsets[3] = ge_int3{ 0, 1, 0 };
    //offsets[4] = ge_int3{ 0, 0, 1 };
    //offsets[5] = ge_int3{ 0, 0, -1 };



    ge_uint* data0 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE3_ADD(mapCoordWhole, staticData->directionalOffsets[0]));
    MapTile tile0 = MapDataGetTile(*data0);
    if (tile0 == MapTile_NONE)
    {
        GE_BITSET(*tileData, MapTileFlags_LowCornerBTMRIGHT);
        GE_BITSET(*tileData, MapTileFlags_LowCornerTPRIGHT);
    }
    ge_uint* data1 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE3_ADD(mapCoordWhole, staticData->directionalOffsets[1]));
    MapTile tile1 = MapDataGetTile(*data1);
    if (tile1 == MapTile_NONE)
    {
        GE_BITSET(*tileData, MapTileFlags_LowCornerBTMLEFT);
        GE_BITSET(*tileData, MapTileFlags_LowCornerTPLEFT);
    }
    ge_uint* data2 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE3_ADD(mapCoordWhole, staticData->directionalOffsets[2]));
    MapTile tile2 = MapDataGetTile(*data2);
    if (tile2 == MapTile_NONE)
    {
        GE_BITSET(*tileData, MapTileFlags_LowCornerTPRIGHT);
        GE_BITSET(*tileData, MapTileFlags_LowCornerTPLEFT);
    }
    ge_uint* data3 = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, GE3_ADD(mapCoordWhole, staticData->directionalOffsets[3]));
    MapTile tile3 = MapDataGetTile(*data3);
    if (tile3 == MapTile_NONE)
    {
        GE_BITSET(*tileData, MapTileFlags_LowCornerBTMRIGHT);
        GE_BITSET(*tileData, MapTileFlags_LowCornerBTMLEFT);
    }


    
    if(MapDataLowCornerCount(*tileData) == 4)
        MapDataSetTile(tileData, MapTile_NONE);

    #ifdef ALL_EXPLORED
        GE_BITSET(*tileData, MapTileFlags_Explored);
    #endif

}

void MapCreate(ALL_CORE_PARAMS, ge_int x, ge_int y)
{
    //printf("Creating Map..\n");

    ge_int i = 0;

            ge_int perlin_z_Q16 = GE2_PERLIN_Q( ge_int2(GE_TO_Q(x), GE_TO_Q(y)), GE_TO_Q(1) >> 6, 8, 0) ;

            #ifdef FLATMAP
            perlin_z_Q16 = GE_TO_Q(1) >> 1;
            #endif

           
            for (ge_int z = 0; z < mapDepth; z++)
            {
                ge_int zPerc_Q16 = GE_DIV_Q(GE_TO_Q(z), GE_TO_Q(mapDepth));
                ge_int depthFromSurface = perlin_z_Q16 - zPerc_Q16;
                MapTile tileType = MapTile_NONE;
                if (zPerc_Q16 < perlin_z_Q16)
                {
                    tileType = MapTile_Rock;

                    if (GE_UNIFORM_RANDOM_RANGE(x * y * z, 0, 20) == 1)
                    {
                        tileType = MapTile_IronOre;
                    }
                    else if (GE_UNIFORM_RANDOM_RANGE(x * y * z + 1, 0, 20) == 1)
                    {
                        tileType = MapTile_GoldOre;
                    }
                    else if (GE_UNIFORM_RANDOM_RANGE(x * y * z + 2, 0, 100) == 1)
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

                //ge_int depthPerc_Q16 = GE_TO_Q(2) + perlin_z_Q16 - zPerc_Q16;
                //depthPerc_Q16 = GE_DIV_Q(depthPerc_Q16, GE_TO_Q(3));
                ////GE_PRINT_Q(depthPerc_Q16*100);

                //if (depthPerc_Q16 * 100 > GE_TO_Q(90))
                //{
                //    tileType = MapTile_DiamondOre;
                //}
                //else if (depthPerc_Q16 * 100 > GE_TO_Q(80))
                //{
                //    tileType = MapTile_CopperOre;
                //}
                //else if (depthPerc_Q16 * 100 > GE_TO_Q(75))
                //{
                //    tileType = MapTile_Rock;
                //}
                //else if (depthPerc_Q16 * 100 > GE_TO_Q(70))
                //{
                //    tileType = MapTile_Dirt;
                //}
                //else if (depthPerc_Q16 * 100 > GE_TO_Q(65))
                //{
                //    tileType = MapTile_DarkGrass;
                //}

                gameState->map.levels[z].machinePtr[x][y] = GE_OFFSET_NULL;

                ge_uint* data = &gameState->map.levels[z].data[x][y];
                *data = tileType;
                #ifdef ALL_EXPLORED
                GE_BITSET(*data, MapTileFlags_Explored);
                #endif

                ge_uint cpy = *data;
                BITBANK_SET_SUBNUMBER_UINT(&cpy, MapTileFlags_RotBit1, 2, GE_UNIFORM_RANDOM_RANGE(x*y,0,4));
                *data = cpy;

                i++;
            }





}
void MapCreate2(ALL_CORE_PARAMS, ge_int x, ge_int y)
{
    //MapCreateSlope(ALL_CORE_PARAMS_PASS, x, y);
    MapBuildTileView(ALL_CORE_PARAMS_PASS, x, y);
    MapUpdateShadow(ALL_CORE_PARAMS_PASS, x, y);

}

void StartupTests()
{
  printf("StartupTests Tests------------------------------------------------------:\n");
  if(0){
  printf("Speed Tests:\n");

    ge_int s = 0;
    for (ge_ulong i = 0; i < 1000; i++)
    {
        ge_int3 a = ge_int3{ GE_TO_Q(i), GE_TO_Q(2), GE_TO_Q(i*2) };
        ge_int3 b = ge_int3{ GE_TO_Q(i*2), GE_TO_Q(i), GE_TO_Q(i) };

        ge_int3 c = GE3_MUL_Q(a, b);
        s += c.x + c.y + c.z;
    }

  }
  if(1)
  {
      FixedPointTests();
  }

  if(0)
  {

    printf("Triangle Tests\n");

    ge_int3 point = ge_int3{GE_TO_Q(1), GE_TO_Q(0), GE_TO_Q(1) };
    Triangle3DHeavy tri;
    tri.base.verts_Q16[0] = ge_int3{ GE_TO_Q(-1), GE_TO_Q(-1), GE_TO_Q(0) };
    tri.base.verts_Q16[1] = ge_int3{ GE_TO_Q( 1), GE_TO_Q(-1), GE_TO_Q(0) };
    tri.base.verts_Q16[2] = ge_int3{ GE_TO_Q(-1), GE_TO_Q( 1), GE_TO_Q(0) };

    Triangle3DMakeHeavy(&tri);

    ge_int dist;
    ge_int3 closestPoint = Triangle3DHeavy_ClosestPoint(&tri, point, &dist);
    printf("closest point: ");
    GE3_PRINT_Q(closestPoint);
    printf("Dist: ");
    GE_PRINT_Q(dist);
  }


  if(0)
  {
    printf("Convex Hull Tests:\n");


    ConvexHull hull;    
    ge_uint tileData = 1;
    MapTileConvexHull_From_TileData(&hull, &tileData);

    printf("convex hull tris:\n");
    for (ge_int i = 0; i < 14; i++)
    {
        GE3_PRINT_Q(hull.triangles[i].base.verts_Q16[0]);
        GE3_PRINT_Q(hull.triangles[i].base.verts_Q16[1]);
        GE3_PRINT_Q(hull.triangles[i].base.verts_Q16[2]);
    }


    ge_int3 p = ge_int3{GE_TO_Q(0),GE_TO_Q(0),GE_TO_Q(2)};
    ge_int3 nearestPoint = MapTileConvexHull_ClosestPointToPoint(&hull, p);

    printf("nearest point: ");
    GE3_PRINT_Q(nearestPoint);

    p = ge_int3{GE_TO_Q(0),GE_TO_Q(0),GE_TO_Q(0)};
    ge_ubyte inside = MapTileConvexHull_PointInside(&hull, p);
    printf("should be inside(1): %d\n", inside);

    p = ge_int3{GE_TO_Q(0),GE_TO_Q(1),GE_TO_Q(0)};
    inside = MapTileConvexHull_PointInside(&hull, p);
    printf("should be outside(0): %d\n", inside);
  }
    printf("End Tests-----------------------------------------------------------------\n");

}








void MapExplorerSpawn(ALL_CORE_PARAMS, MapExplorerAgent* agent)
{
    ge_int3 randomTileLoc;
    randomTileLoc.x = GE_UNIFORM_RANDOM_RANGE((ge_int)agent, 0, mapDim);
    randomTileLoc.y = GE_UNIFORM_RANDOM_RANGE((ge_int)agent, 0, mapDim);
    randomTileLoc.z = GE_UNIFORM_RANDOM_RANGE((ge_int)agent, 0, mapDepth);


    ge_uint* data = MapGetDataPointerFromCoord(ALL_CORE_PARAMS_PASS, randomTileLoc);
    if(MapDataGetTile(*data) == MapTile_NONE && GE_BITGET(*data, MapTileFlags_Explored))
    {
        
    }
}

void AStarPathStepsInit(ALL_CORE_PARAMS, AStarPathSteps* steps)
{
    for (ge_int i = 0; i < ASTARPATHSTEPSSIZE; i++)
    {
        AStarInitPathNode(&gameState->paths.pathNodes[i]);
    }

    //paths initialize
    gameState->paths.nextListIdx = 0;
    for(ge_int i = 0; i < ASTAR_MAX_PATHS; i++)
        gameState->paths.pathStarts[i] = GE_OFFSET_NULL;

}

void CLIENT_InitClientState(SynchronizedClientState* client)
{
    client->selectedMachine = GE_OFFSET_NULL;
    client->selectedPeepPrimary = GE_OFFSET_NULL;

}


void CLIENT_InitClientStates(ALL_CORE_PARAMS)
{
    for(ge_int i = 0; i < MAX_CLIENTS; i++)
    {
        SynchronizedClientState* client = &gameState->clientStates[i];
        CLIENT_InitClientState(client);
        SyncedGui* gui = &client->gui;

        GuiState_Init(&gui->guiState);
    }
    GuiState_Init(&gameState->fakePassGui.guiState);
}



void MakeItemStrings(char* strings)
{
    //char a[ITEMTYPE_STRING_MAX_LENGTH] = "Iron Dust\0";
    //strings[0] = a;

    //ItemTypeStrings[ITEMTYPE_STRING_MAX_LENGTH][ItemTypes_NUMITEMS];

   // LOCAL_STR(a, "Iron Dust");


    //strings[1] = '\0';
    //strings[1*ITEMTYPE_STRING_MAX_LENGTH] = "Iron Dust\0";
   // strings[2*ITEMTYPE_STRING_MAX_LENGTH] = "Iron Bar\0";
    //strings[3*ITEMTYPE_STRING_MAX_LENGTH] = "Rock Dust\0";
}


void game_init_single(ALL_CORE_PARAMS)
{
    printf("Game Initializing...\n");


    printf("ASTARBFS Size: %d\n", sizeof(AStarSearch_BFS));

    printf("Initializing StaticData Buffers..\n");
    MakeCardinalDirectionOffsets(&staticData->directionalOffsets[0]);
   // MakeItemStrings(staticData->ItemTypeStrings);



    printf("Initializing GUI..\n");
    GUI_INIT_STYLE(GUI_CORE_PARAMS_PASS);

    printf("Initializing Default Client States..\n");
    CLIENT_InitClientStates(ALL_CORE_PARAMS_PASS);



    printf("Startup Tests..\n");
    StartupTests();


    printf("Creating Machines Types\n");
    Machine_InitDescriptions(ALL_CORE_PARAMS_PASS);
    printf("Creating Machine Recipes\n");
    Machine_InitRecipes(ALL_CORE_PARAMS_PASS);
    printf("Mapping Item Tiles\n");   
    InitItemTypeTiles(ALL_CORE_PARAMS_PASS);

    gameState->numClients = 1;
    gameStateActions->pauseState = 0;
    ThisClient(ALL_CORE_PARAMS_PASS)->mapZView = mapDepth-1;
    ThisClient(ALL_CORE_PARAMS_PASS)->mapZView_1 = 0;

    for (ge_int secx = 0; secx < SQRT_MAXSECTORS; secx++)
    {
        for (ge_int secy = 0; secy < SQRT_MAXSECTORS; secy++)
        {
            gameState->sectors[secx][secy].ptr.x = secx;
            gameState->sectors[secx][secy].ptr.y = secy;
            gameState->sectors[secx][secy].lock = 0;
            gameState->sectors[secx][secy].empty = true; 
            for(ge_int j = 0; j < MAX_PEEPS_PER_SECTOR; j++)
            {
                gameState->sectors[secx][secy].peepPtrs[j] = GE_OFFSET_NULL;
            }
        }
    }
    printf("Sectors Initialized.\n");




    AStarSearch_BFS_Instantiate(&gameState->mapSearchers[0]);
}

void game_init_multi(ALL_CORE_PARAMS)
{


    ge_ulong chunkSize = (mapDim*mapDim) / GAME_UPDATE_WORKITEMS;
    for (ge_ulong i = 0; i < chunkSize+1; i++)
    {
        ge_ulong idx = threadIdx+GAME_UPDATE_WORKITEMS*i;
        if (idx < (mapDim*mapDim)) {

            MapCreate(ALL_CORE_PARAMS_PASS, idx % mapDim, idx / mapDim);
        }
    }



}
void game_init_multi2(ALL_CORE_PARAMS)
{
    ge_ulong chunkSize = (mapDim*mapDim) / GAME_UPDATE_WORKITEMS;
    for (ge_ulong i = 0; i < chunkSize+1; i++)
    {
        ge_ulong idx = threadIdx+GAME_UPDATE_WORKITEMS*i;
        if (idx < (mapDim*mapDim)) {
            MapCreate2(ALL_CORE_PARAMS_PASS, idx % mapDim, idx / mapDim);
        }
    }


}
void game_init_single2(ALL_CORE_PARAMS)
{
    AStarPathStepsInit(ALL_CORE_PARAMS_PASS, &gameState->paths);



    printf("AStarTests:\n");
    //printf("AStarTests1:\n");
    //AStarSearch_BFS_Instantiate(&gameState->mapSearchers[0]);
    ////test AStarHeap
    //for (ge_int x = 0; x < mapDim; x++)
    //{
    //    for (ge_int y = 0; y < mapDim; y++)
    //    {
    //        AStarNode* node = &gameState->mapSearchers[0].details[x][y][0];
    //        node->h_Q16 = x * y;
    //        node->g_Q16 = x * y;
    //        AStarOpenHeapInsert(&gameState->mapSearchers[0], node);
    //    }
    //}

    //do
    //{
    //    AStarNode* node = AStarOpenHeapRemove(&gameState->mapSearchers[0]);
    //    AStarPrintNodeStats(node);
    //} while (gameState->mapSearchers[0].openHeapSize);

    printf("AStarTests2:\n");
    //test AStar

    // ge_short3 start = (ge_short3){ 0,0,mapDepth - 2 };
    // ge_short3 end = (ge_short3){ mapDim - 1,mapDim - 1,1 };
    // AStarSearch_BFS_Routine(ALL_CORE_PARAMS_PASS, &gameState->mapSearchers[0], start, end, CL_INTMAX);

    printf("initializing peeps..\n");
    const ge_int spread = 500;
    for (ge_uint p = 0; p < maxPeeps; p++)
    {
        gameState->peeps[p].ptr = p;

        gameState->peeps[p].physics.base.pos_Q16.x = GE_UNIFORM_RANDOM_RANGE(p, GE_TO_Q<16>(-spread), GE_TO_Q<16>(spread));
        gameState->peeps[p].physics.base.pos_Q16.y = GE_UNIFORM_RANDOM_RANGE(p + 1, GE_TO_Q<16>(-spread), GE_TO_Q<16>(spread));
        //GE_PRINT_Q<16>(GE_TO_Q<16>(-spread));
        GE2_PRINT_Q<16>(gameState->peeps[p].physics.base.pos_Q16);

        ge_int3 mapcoord;
        ge_int2 world2D;
        world2D.x = gameState->peeps[p].physics.base.pos_Q16.x;
        world2D.y = gameState->peeps[p].physics.base.pos_Q16.y;
        ge_int occluded;

        GetMapTileCoordFromWorld2D(ALL_CORE_PARAMS_PASS, world2D, &mapcoord, &occluded, mapDepth - 1);
        //printf("%d\n", mapcoord.z);
        mapcoord.z += 2;
        mapcoord = GE3_TO_Q(mapcoord);

        ge_int3 worldCoord;
        MapToWorld(mapcoord, &worldCoord);
                
        gameState->peeps[p].physics.base.pos_Q16.z = worldCoord.z;

        ge_int3 pmap_Q16;
        WorldToMap(gameState->peeps[p].physics.base.pos_Q16, &pmap_Q16);

        gameState->peeps[p].posMap_Q16 = pmap_Q16;
        gameState->peeps[p].lastGoodPosMap_Q16 = gameState->peeps[p].posMap_Q16;


        gameState->peeps[p].mapCoord = GE3_CAST<ge_offsetPtrShort3>(GE3_WHOLE_Q(gameState->peeps[p].posMap_Q16));
        gameState->peeps[p].mapCoord_1 = GE_OFFSET_NULL_SHORT_3D;


        gameState->peeps[p].physics.shape.radius_Q16 = GE_TO_Q(1);
        GE_BITCLEAR(gameState->peeps[p].stateBasic.bitflags0, PeepState_BitFlags_deathState);
        GE_BITSET(gameState->peeps[p].stateBasic.bitflags0, PeepState_BitFlags_valid);
        GE_BITSET(gameState->peeps[p].stateBasic.bitflags0, PeepState_BitFlags_visible);
        gameState->peeps[p].stateBasic.health = 10;
        gameState->peeps[p].stateBasic.deathState = 0;
        gameState->peeps[p].stateBasic.buriedGlitchState = 0;
        gameState->peeps[p].stateBasic.orderStackIdx = -1;

      
        

        gameState->peeps[p].physics.base.v_Q16 = ge_int3{ 0,0,0 };
        gameState->peeps[p].physics.base.vel_add_Q16 = ge_int3{ 0,0,0 };
        gameState->peeps[p].physics.base.pos_post_Q16 = ge_int3{ 0,0,0 };

        gameState->peeps[p].sectorPtr = GE_OFFSET_NULL_2D;

        gameState->peeps[p].minDistPeepPtr = GE_OFFSET_NULL;
        gameState->peeps[p].minDistPeep_Q16 = (1 << 30);
        gameState->peeps[p].physics.drive.target_x_Q16 = gameState->peeps[p].physics.base.pos_Q16.x;
        gameState->peeps[p].physics.drive.target_y_Q16 = gameState->peeps[p].physics.base.pos_Q16.y;
        gameState->peeps[p].physics.drive.targetPathNodeOPtr = GE_OFFSET_NULL;


        gameState->peeps[p].stateBasic.faction = GE_UNIFORM_RANDOM_RANGE(p,0,4);


        for (ge_int i = 0; i < MAX_CLIENTS; i++)
        {
            gameState->clientStates[i].selectedPeepsLastIdx = GE_OFFSET_NULL;


            CL_CHECKED_ARRAY_SET(gameState->peeps[p].nextSelectionPeepPtr, MAX_CLIENTS, i, GE_OFFSET_NULL)
            CL_CHECKED_ARRAY_SET(gameState->peeps[p].prevSelectionPeepPtr, MAX_CLIENTS, i, GE_OFFSET_NULL)
        }

    }

    printf("Peeps Initialized.\n");








}



void UpdateMapExplorer(ALL_CORE_PARAMS,  MapExplorerAgent* agent)
{



}


void PeepDraw(ALL_CORE_PARAMS, Peep* peep)
{

    ge_float3 drawColor;
    float drawPosX = (float)((float)peep->physics.base.pos_Q16.x / (1 << 16));
    float drawPosY = (float)((float)peep->physics.base.pos_Q16.y / (1 << 16));

    float brightFactor = 0.6f;
    if (peep->stateBasic.faction == 0)
    {
        drawColor.x = 0.0f;
        drawColor.y = 1.0f;
        drawColor.z = 1.0f;
    }
    else if(peep->stateBasic.faction == 1)
    {
        drawColor.x = 1.0f;
        drawColor.y = 1.0f;
        drawColor.z = 0.0f;
    }
    else if (peep->stateBasic.faction == 2)
    {
        drawColor.x = 0.0f;
        drawColor.y = 1.0f;
        drawColor.z = 0.0f;
    }
    else if (peep->stateBasic.faction == 3)
    {
        drawColor.x = 1.0f;
        drawColor.y = 0.0f;
        drawColor.z = 1.0f;
    }

    if (gameState->clientStates[gameStateActions->clientId].peepRenderSupport[peep->ptr].render_selectedByClient)
    {
        brightFactor = 1.0f;
        gameState->clientStates[gameStateActions->clientId].peepRenderSupport[peep->ptr].render_selectedByClient = 0;
    }
    if ( GE_BITGET(peep->stateBasic.bitflags0, PeepState_BitFlags_deathState) )
    {
        brightFactor = 0.6f;
        drawColor.x = 0.5f;
        drawColor.y = 0.5f;
        drawColor.z = 0.5f;
    }
    if (!GE_BITGET(peep->stateBasic.bitflags0, PeepState_BitFlags_visible))
    {
        drawPosX = 99999;
        drawPosY = 99999;
    }


    if(peep->physics.base.pos_Q16.z < 0)
    {   brightFactor = 1.0f;
        drawColor.x = 1.0f;
        drawColor.y = 0.0f;
        drawColor.z = 0.0f;
    }

    if(peep->stateBasic.buriedGlitchState > 0)
    { 
        brightFactor = 1.0f;
        drawColor.x = 0.0f;
        drawColor.y = 1.0f;
        drawColor.z = 0.0f;
    }

    peepVBOBuffer[peep->ptr * (GameGraphics::PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 0] = drawPosX;
    peepVBOBuffer[peep->ptr * (GameGraphics::PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 1] = drawPosY;

    peepVBOBuffer[peep->ptr * (GameGraphics::PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 2] = drawColor.x * brightFactor;
    peepVBOBuffer[peep->ptr * (GameGraphics::PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 3] = drawColor.y * brightFactor;
    peepVBOBuffer[peep->ptr * (GameGraphics::PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 4] = drawColor.z * brightFactor;

    peepVBOBuffer[peep->ptr * (GameGraphics::PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 5] = peep->physics.base.CS_angle_rad;
}



void game_updatepre1(ALL_CORE_PARAMS)
{


    // Get the index of the current element to be processed
    ge_int globalid = threadIdx;




    ge_ulong chunkSize = (maxPeeps) / GAME_UPDATE_WORKITEMS;
    for (ge_ulong i = 0; i < chunkSize+1; i++)
    {
        ge_ulong idx = globalid+GAME_UPDATE_WORKITEMS*i;
        if (idx < maxPeeps) {
            Peep* p = &gameState->peeps[idx]; 
            PeepPreUpdate2(ALL_CORE_PARAMS_PASS, p);
        }
    }


    if(globalid == 0)
    {
        
        LINES_ClearAll(ALL_CORE_PARAMS_PASS);


        if (ThisClient(ALL_CORE_PARAMS_PASS)->mapZView != ThisClient(ALL_CORE_PARAMS_PASS)->mapZView_1)
        {
            ThisClient(ALL_CORE_PARAMS_PASS)->updateMap = true;
            ThisClient(ALL_CORE_PARAMS_PASS)->mapZView_1 = ThisClient(ALL_CORE_PARAMS_PASS)->mapZView;
        }
    }
}


void game_update(ALL_CORE_PARAMS)
{

    {
    //printf("weee. %d\n", threadIdx);

    // Get the index of the current element to be processed
    ge_int globalid = threadIdx;


    ge_ulong chunkSize = (maxPeeps) / GAME_UPDATE_WORKITEMS;
    for (ge_ulong i = 0; i < chunkSize+1; i++)
    {
        ge_ulong idx = globalid+GAME_UPDATE_WORKITEMS*i;
        if (idx < maxPeeps) {

            Peep* p = &gameState->peeps[idx]; 
            PeepUpdate(ALL_CORE_PARAMS_PASS, p);
            PeepDraw(ALL_CORE_PARAMS_PASS, p);
        }
    }

    chunkSize = (MAX_MACHINES) / GAME_UPDATE_WORKITEMS;
    for (ge_ulong i = 0; i < chunkSize+1; i++)
    {
        ge_ulong idx = globalid+GAME_UPDATE_WORKITEMS*i;
        if ( idx < MAX_MACHINES)
        {
            Machine* m = &gameState->machines[idx]; 
            MachineUpdate(ALL_CORE_PARAMS_PASS, m);
        }
    }





    //update map view
    if (ThisClient(ALL_CORE_PARAMS_PASS)->updateMap)
    {
        
        ge_ulong chunkSize = (mapDim * mapDim) / GAME_UPDATE_WORKITEMS;

        for (ge_ulong i = 0; i < chunkSize+1; i++)
        {
            ge_ulong xyIdx = globalid+GAME_UPDATE_WORKITEMS*i;

            
            if (xyIdx < (mapDim * mapDim))
            {
                MapBuildTileView(ALL_CORE_PARAMS_PASS, xyIdx % mapDim, xyIdx / mapDim);
                MapUpdateShadow(ALL_CORE_PARAMS_PASS, xyIdx % mapDim, xyIdx / mapDim);
            }
        }
    }

    //update map explorers
    MapExplorerAgent* explorer = &gameState->explorerAgents[globalid];
    UpdateMapExplorer(ALL_CORE_PARAMS_PASS, explorer);

    }
}
void game_update2(ALL_CORE_PARAMS)
{


    // Get the index of the current element to be processed
    ge_int globalid = threadIdx;




    //reset searches
    AStarSearch_BFS* search = &gameState->mapSearchers[0];
    if(search->state == AStarPathFindingProgress_ResetReady)
    {
        ge_ulong chunkSize = (mapDim * mapDim * mapDepth) / GAME_UPDATE_WORKITEMS;
    
        for (ge_ulong i = 0; i < chunkSize+1; i++)
        {
            ge_long xyzIdx = globalid+GAME_UPDATE_WORKITEMS*i;
            
            if (xyzIdx < (mapDim * mapDim* mapDepth))
            {

                ge_long z = xyzIdx % mapDepth;
                ge_long y = (xyzIdx / mapDepth) % mapDim;
                ge_long x = xyzIdx / (mapDim * mapDepth); 




                if(x > mapDim)
                    printf("X>mapDim\n");
                if(y > mapDim)
                    printf("Y>mapDim\n");
                if(z > mapDepth)
                    printf("Z>mapDepth\n");

                if(xyzIdx == (mapDim * mapDim* mapDepth)-1)
                    printf("RESETING SEARCH\n");

                AStarSearch_BFS_InstantiateParrallel(search, xyzIdx, x, y, z);
            }
        }
    }

    if(globalid == 0)
    if(ThisClient(ALL_CORE_PARAMS_PASS)->updateMap)
    {
        ThisClient(ALL_CORE_PARAMS_PASS)->updateMap = false;
    }

}


void game_post_update( ALL_CORE_PARAMS )
{
    // Get the index of the current element to be processed
    ge_int globalid = threadIdx;



    if(globalid == 0)
    {
        //set selected peeps to highlight.
        ge_uint curPeepIdx = gameState->clientStates[gameStateActions->clientId].selectedPeepsLastIdx;
        PeepRenderSupport peepRenderSupport[maxPeeps];
        while (curPeepIdx != GE_OFFSET_NULL)
        {
            Peep* p = &gameState->peeps[curPeepIdx];
            gameState->clientStates[gameStateActions->clientId].peepRenderSupport[curPeepIdx].render_selectedByClient = 1;
                    
            curPeepIdx = p->prevSelectionPeepPtr[gameStateActions->clientId];
        }

        //should be reset after gameupdate_2
        if(gameState->mapSearchers[0].state == AStarPathFindingProgress_ResetReady)
            gameState->mapSearchers[0].state = AStarPathFindingProgress_Ready;

        //update jobs
        AStarJob_UpdateJobs(ALL_CORE_PARAMS_PASS);






        //draw debug lines on paths
        #ifdef DEBUG_PATHS


        AStarPathSteps* paths = &gameState->paths;

        for(ge_int i = 0; i < ASTAR_MAX_PATHS; i++)
        {
            ge_offsetPtr pathStartOPtr = paths->pathStarts[i];
            if(pathStartOPtr != GE_OFFSET_NULL)
            {
                AStarPathNode* node;
                GE_OFFSET_TO_PTR(paths->pathNodes, pathStartOPtr, node)

                while(node != NULL && node->nextOPtr != GE_OFFSET_NULL)
                {
                    AStarPathNode* nodeNext;
                    GE_OFFSET_TO_PTR(paths->pathNodes, node->nextOPtr, nodeNext);
                    CL_CHECK_NULL(nodeNext);

                    ge_int3 worldCoord_Q16;
                    MapToWorld(( node->mapCoord_Q16 ), &worldCoord_Q16);

                    ge_int3 worldCoordNext_Q16;
                    MapToWorld(( nodeNext->mapCoord_Q16 ), &worldCoordNext_Q16);

                    
                    ge_float2 worldCoordsFloat = (ge_float2)(GE_Q_TO_FLOAT(worldCoord_Q16.x),GE_Q_TO_FLOAT(worldCoord_Q16.y));
                    ge_float2 worldCoordsNextFloat = (ge_float2)(GE_Q_TO_FLOAT(worldCoordNext_Q16.x),GE_Q_TO_FLOAT(worldCoordNext_Q16.y));

                
                    LINES_DrawLineWorld(ALL_CORE_PARAMS_PASS, worldCoordsFloat, worldCoordsNextFloat, ge_float3(GE_UNIFORM_RANDOM_RANGE(i, 0, 1000)/1000.0f, GE_UNIFORM_RANDOM_RANGE(i+1, 0, 1000)/1000.0f, 1.0f));


                    GE_OFFSET_TO_PTR(paths->pathNodes, node->nextOPtr, node)
                }
            }
        }

        #endif
    }



    ge_ulong chunkSize = (MAX_ORDERS) / GAME_UPDATE_WORKITEMS;
    for (ge_ulong pi = 0; pi < chunkSize+1; pi++)
    {
        ge_ulong idx = globalid+(GAME_UPDATE_WORKITEMS)*pi;
        if (idx < MAX_ORDERS)
        {
            Order* order;
            CL_CHECKED_ARRAY_GET_PTR(gameState->orders, MAX_ORDERS, idx, order)
            

            if(order->valid && order->pendingDelete)
            {
                if(order->nextExecutionOrder == GE_OFFSET_NULL && order->prevExecutionOrder == GE_OFFSET_NULL)
                {
                    printf("deleting expired and dangling order\n");
                    Order_DeleteOrder(ALL_CORE_PARAMS_PASS, idx);
                }
            }

            if(order->valid && order->refCount <= 0)
            {
                order->pendingDelete = true;
            }
        }
    }






}

void game_preupdate_1(ALL_CORE_PARAMS) {


    ge_int globalid = threadIdx;

    if (globalid >= GAME_UPDATE_WORKITEMS)
        return;
   

    ge_ulong chunkSize = (maxPeeps) / GAME_UPDATE_WORKITEMS;
    for (ge_ulong pi = 0; pi < chunkSize+1; pi++)
    {
        ge_ulong idx = globalid+(GAME_UPDATE_WORKITEMS)*pi;
        if (idx < maxPeeps)
        {
            Peep* peep;
            CL_CHECKED_ARRAY_GET_PTR(gameState->peeps, maxPeeps, idx, peep)
            CL_CHECK_NULL(peep)
            
            ge_int x = ((peep->physics.base.pos_Q16.x >> 16) / (SECTOR_SIZE));
            ge_int y = ((peep->physics.base.pos_Q16.y >> 16) / (SECTOR_SIZE));

            peep->sectorPtr.x = x + SQRT_MAXSECTORS / 2;
            peep->sectorPtr.y = y + SQRT_MAXSECTORS / 2;

            MapSector* sector;
            GE_OFFSET_TO_PTR_2D(gameState->sectors, peep->sectorPtr, sector);
            sector->empty = false;
        }

    }


    //parrallel order Pre update



}


void game_preupdate_2(ALL_CORE_PARAMS) {


    // Get the index of the current element to be processed
    ge_int globalid = threadIdx;

    if (globalid >= GAME_UPDATE_WORKITEMS)
        return;
   

    ge_uint chunkSize = (SQRT_MAXSECTORS*SQRT_MAXSECTORS) / GAME_UPDATE_WORKITEMS;



    for (ge_ulong pi = 0; pi < chunkSize+1; pi++)
    {
        ge_ulong idx = globalid+(GAME_UPDATE_WORKITEMS)*pi;
        if (idx < SQRT_MAXSECTORS*SQRT_MAXSECTORS)
        {
            ge_offsetPtr2 xy;
            xy.x = idx % SQRT_MAXSECTORS;
            xy.y = idx / SQRT_MAXSECTORS;

            volatile MapSector* mapSector;
            GE_OFFSET_TO_PTR_2D(gameState->sectors, xy, mapSector);
            CL_CHECK_NULL(mapSector)

            if(mapSector->empty)
                continue;

            //clear non-present peeps
            for(ge_int j = 0; j < MAX_PEEPS_PER_SECTOR; j++)
            {
                if(mapSector->peepPtrs[j] == GE_OFFSET_NULL)
                    continue;
                Peep* peep;
                GE_OFFSET_TO_PTR(gameState->peeps, mapSector->peepPtrs[j], peep)

                if(!GE2_EQUAL(peep->sectorPtr, xy))
                {
                    mapSector->peepPtrs[j] = GE_OFFSET_NULL;
                }
            }



            for(ge_int i = mapSector->chunkStart; (i < mapSector->chunkStart + maxPeeps/16) && i < maxPeeps; i++)
            {   
                Peep* peep;
                CL_CHECKED_ARRAY_GET_PTR(gameState->peeps, maxPeeps, i, peep)
                CL_CHECK_NULL(peep)
                
                if(GE2_EQUAL(peep->sectorPtr, xy) == 0)
                    continue;

                //check if its already in the list
                if(mapSector->peepPtrs[peep->sectorListIdx] == i)
                    continue;

                peep->sectorListIdx = mapSector->ptrIterator;
                mapSector->peepPtrs[mapSector->ptrIterator] = i;
                mapSector->ptrIterator++;
                if(mapSector->ptrIterator >= MAX_PEEPS_PER_SECTOR)
                {
                    mapSector->ptrIterator = 0;
                }
            }


            mapSector->chunkStart += maxPeeps/16;
            if(mapSector->chunkStart >= maxPeeps)
                mapSector->chunkStart = 0;
        }
    }


}


























}