

#include "common.h"



void Print_GE_INT2(ge_int2 v)
{
    printf("{%d,%d}\n", v.x, v.y);
}
void Print_GE_INT3(ge_int3 v)
{
    printf("{%d,%d,%d}\n", v.x, v.y, v.z);
}
void Print_GE_SHORT3(ge_short3 v)
{
    printf("{%d,%d,%d}\n", v.x, v.y, v.z);
}

inline cl_uint BITBANK_GET_SUBNUMBER_UINT(cl_uint bank, cl_int lsbBitIdx, cl_int numBits)
{
    cl_uint mask = 0;
    for (int i = 0; i < numBits; i++)
    {
        mask |= (1 << (i + lsbBitIdx));
    }


    return (bank & mask) >> lsbBitIdx;
}
inline void BITBANK_SET_SUBNUMBER_UINT(cl_uint* bank, cl_int lsbBitIdx, cl_int numBits, cl_uint number)
{
    int i = 0;
    

    cl_uint mask = 0;
    for (int i = 0; i < numBits; i++)
    {
        mask |= (1 << (i + lsbBitIdx));
    }

    *bank &= ~mask;//clear the region
    *bank |= ((number << lsbBitIdx) & mask);
}



///----------------------------------------------------------------------------------------------------------------
///
/// 

void MakeCardinalDirectionOffsets(ge_int3* offsets)
{
    offsets[0] = (ge_int3){ 1, 0, 0 };
    offsets[1] = (ge_int3){ -1, 0, 0 };
    offsets[2] = (ge_int3){ 0, -1, 0 };
    offsets[3] = (ge_int3){ 0, 1, 0 };
    offsets[4] = (ge_int3){ 0, 0, 1 };
    offsets[5] = (ge_int3){ 0, 0, -1 };

    offsets[6] = (ge_int3){ 1, 0, -1 };
    offsets[7] = (ge_int3){ 0, 1, -1 };
    offsets[8] = (ge_int3){ 1, 1, -1 };
    offsets[9] = (ge_int3){ -1, 0, -1 };
    offsets[10] = (ge_int3){ 0, -1, -1 };
    offsets[11] = (ge_int3){ -1, -1, -1 };
    offsets[12] = (ge_int3){ 1, -1, -1 };
    offsets[13] = (ge_int3){ -1, 1, -1 };

    offsets[14] = (ge_int3){ 1, 0, 1 };
    offsets[15] = (ge_int3){ 0, 1, 1 };
    offsets[16] = (ge_int3){ 1, 1, 1 };
    offsets[17] = (ge_int3){ -1, 0, 1 };
    offsets[18] = (ge_int3){ 0, -1, 1 };
    offsets[19] = (ge_int3){ -1, -1, 1 };
    offsets[20] = (ge_int3){ 1, -1, 1 };
    offsets[21] = (ge_int3){ -1, 1, 1 };

    offsets[22] = (ge_int3){ 1, 1, 0 };
    offsets[23] = (ge_int3){ -1, 1, 0 };
    offsets[24] = (ge_int3){ 1, -1, 0 };
    offsets[25] = (ge_int3){ -1, -1, 0 };
}

cl_uchar GE_INT3_SINGLE_ENTRY(ge_int3 a)
{
    int s = 0;
    if (a.x != 0)
        s++;
    if (a.y != 0)
        s++;
    if (a.z != 0)
        s++;

    if (s == 1)
        return 1;
    else
        return 0;
}


// Ret 1: [2,0,2],[-1,-1,0],[4,4,4],[3,0,0] etc
// Ret 0: [2,0,1],[1,-1,1],[2,3,4],[0,0,0] etc
cl_uchar GE_INT3_WHACHAMACOLIT1_ENTRY(ge_int3 a)
{
    if (GE_INT3_ZERO(a))
        return 0;

    int n = a.x + a.y + a.z;
    int s =0;
    if (a.x != 0)
        s++;
    if (a.y != 0)
        s++;
    if (a.z != 0)
        s++;

    int f = 0;
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



cl_uchar BaryCentric_In_Triangle_Q16(ge_int3 baryCoords)
{
    if (baryCoords.x >= 0 && baryCoords.x <= TO_Q16(1))
    {
        if (baryCoords.y >= 0 && baryCoords.y <= TO_Q16(1))
        {
            if (baryCoords.z >= 0 && baryCoords.z <= TO_Q16(1))
            {
                return 1;
            }
        }
    }
    return 0;
}


int SOME_INTERNAL_CORDIST(int x, int y)
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

ge_int3 Triangle3D_ToBaryCentric(Triangle3DHeavy* triangle, ge_int3 point)
{
    ge_int3 U = triangle->u_Q16;
    ge_int3 V = triangle->v_Q16;
    ge_int3 W = GE_INT3_SUB(point, triangle->base.verts_Q16[0]);


    long d00 = GE_INT3_DOT_PRODUCT_Q16(U , U );
    long d01 = GE_INT3_DOT_PRODUCT_Q16(U , V);
    long d11 = GE_INT3_DOT_PRODUCT_Q16(V , V);
    long d20 = GE_INT3_DOT_PRODUCT_Q16(W , U);
    long d21 = GE_INT3_DOT_PRODUCT_Q16(W , V);
    long denom = MUL_PAD_Q16( d00 , d11 ) - MUL_PAD_Q16(d01 , d01);
    long u, v, w;
    v = DIV_PAD_Q16((MUL_PAD_Q16(d11 , d20) - MUL_PAD_Q16(d01 , d21)) , denom);
    w = DIV_PAD_Q16((MUL_PAD_Q16(d00 , d21) - MUL_PAD_Q16(d01 , d20)) , denom);
    u = (TO_Q16(1) - v ) - w;
    return (ge_int3) {
        u, v, w
    };
}

ge_int3 Triangle3DHeavy_ClosestPoint(Triangle3DHeavy* triangle, ge_int3 point_Q16, int* dist_Q16)
{   
    ge_int3 P1 = triangle->base.verts_Q16[0];
    ge_int3 P2 = triangle->base.verts_Q16[1];
    ge_int3 P3 = triangle->base.verts_Q16[2];

    
    //printf("P1: ");
    //Print_GE_INT3_Q16(P1);
    //printf("P2: ");
    //Print_GE_INT3_Q16(P2);
    //printf("P3: ");
    //Print_GE_INT3_Q16(P3);


    ge_int3 P_prime;
    ge_int3 P_prime_bary;

    int Nmag;
    ge_int3 N_n = GE_INT3_NORMALIZE_Q16(triangle->normal_Q16, &Nmag);
    //printf("N_n: ");
    //Print_GE_INT3_Q16(N_n);


    ge_int3 W = GE_INT3_SUB(point_Q16, P1);

    //printf("W: ");
    //Print_GE_INT3_Q16(W);


    int dot = GE_INT3_DOT_PRODUCT_Q16(W, N_n);
    //printf("dot: ");
    //PrintQ16(dot);
    ge_int3 term2 = GE_INT3_SCALAR_MUL_Q16(dot, GE_INT3_NEG(N_n));
    P_prime = GE_INT3_ADD(point_Q16, term2);


    //Triangle2DHeavy_ProjectedPoint(triangle, point_Q16, &P_prime_bary, &P_prime);

    //printf("P_prime: ");
    //Print_GE_INT3_Q16(P_prime);

    P_prime_bary = Triangle3D_ToBaryCentric(triangle, P_prime);
    //printf("P_prime_bary: ");
    //Print_GE_INT3_Q16(P_prime_bary);
    cl_uchar onSurface = BaryCentric_In_Triangle_Q16(P_prime_bary);

    if (onSurface == 1)
    {
        *dist_Q16 = ge_length_v3_Q16(GE_INT3_SUB(point_Q16, P_prime));

        return P_prime;
    }

    ge_int3 P1_P_prime = GE_INT3_SUB(P_prime, P1);
    ge_int3 P2_P_prime = GE_INT3_SUB(P_prime, P2);
    ge_int3 P3_P_prime = GE_INT3_SUB(P_prime, P3);

    ge_int3 R1 = GE_INT3_SUB(P1, P2);
    ge_int3 R2 = GE_INT3_SUB(P2, P3);
    ge_int3 R3 = GE_INT3_SUB(P3, P1);

    int R1_mag;
    ge_int3 R1_N = GE_INT3_NORMALIZE_Q16(R1, &R1_mag);
    int R2_mag;
    ge_int3 R2_N = GE_INT3_NORMALIZE_Q16(R2, &R2_mag);
    int R3_mag;
    ge_int3 R3_N = GE_INT3_NORMALIZE_Q16(R3, &R3_mag);


    ge_int3 R1_N_PERP = GE_VECTOR3_ROTATE_ABOUT_AXIS_POS90_Q16(R1_N, triangle->normal_Q16);
    ge_int3 R2_N_PERP = GE_VECTOR3_ROTATE_ABOUT_AXIS_POS90_Q16(R2_N, triangle->normal_Q16);
    ge_int3 R3_N_PERP = GE_VECTOR3_ROTATE_ABOUT_AXIS_POS90_Q16(R3_N, triangle->normal_Q16);


    int DOT1 = GE_INT3_DOT_PRODUCT_Q16(P1_P_prime, R3_N_PERP);
    int DOT2 = GE_INT3_DOT_PRODUCT_Q16(P2_P_prime, R1_N_PERP);
    int DOT3 = GE_INT3_DOT_PRODUCT_Q16(P3_P_prime, R2_N_PERP);


    ge_int3 D1R3 = GE_INT3_SCALAR_MUL_Q16(DOT1, R3_N_PERP);
    ge_int3 D2R1 = GE_INT3_SCALAR_MUL_Q16(DOT2, R1_N_PERP);
    ge_int3 D3R2 = GE_INT3_SCALAR_MUL_Q16(DOT3, R2_N_PERP);


    //p_prime projected to 3 edges
    ge_int3 P_prime_C1 = GE_INT3_SUB(P_prime, D1R3);
    ge_int3 P_prime_C2 = GE_INT3_SUB(P_prime, D2R1);
    ge_int3 P_prime_C3 = GE_INT3_SUB(P_prime, D3R2);


    //clamp C points to edge limits
    int Z1i = GE_INT3_DOT_PRODUCT_Q16(P1_P_prime, R3_N);
    int Z2i = GE_INT3_DOT_PRODUCT_Q16(P2_P_prime, R1_N);
    int Z3i = GE_INT3_DOT_PRODUCT_Q16(P3_P_prime, R2_N);

    int Z1 = R3_mag - Z1i;
    int Z2 = R1_mag - Z2i;
    int Z3 = R2_mag - Z3i;


    int CD1 = SOME_INTERNAL_CORDIST(Z1, Z1i);
    int CD2 = SOME_INTERNAL_CORDIST(Z2, Z2i);
    int CD3 = SOME_INTERNAL_CORDIST(Z3, Z3i);


    ge_int3 J1 = GE_INT3_SCALAR_MUL_Q16(CD1, R3_N);
    ge_int3 J2 = GE_INT3_SCALAR_MUL_Q16(CD2, R1_N);
    ge_int3 J3 = GE_INT3_SCALAR_MUL_Q16(CD3, R2_N);

    ge_int3 L1 = GE_INT3_ADD(J1, P_prime_C1);
    ge_int3 L2 = GE_INT3_ADD(J2, P_prime_C2);
    ge_int3 L3 = GE_INT3_ADD(J3, P_prime_C3);

    //get closest L to P_prime
    int L1D = ge_length_v3_Q16( GE_INT3_SUB(L1, P_prime) );
    int L2D = ge_length_v3_Q16( GE_INT3_SUB(L2, P_prime) );
    int L3D = ge_length_v3_Q16( GE_INT3_SUB(L3, P_prime) );


    if (L1D < L2D && L1D < L2D)
    {
        *dist_Q16 = ge_length_v3_Q16(GE_INT3_SUB(point_Q16, L1));

        return L1;
    }
    else if (L2D < L1D && L2D < L3D)
    {
        *dist_Q16 = ge_length_v3_Q16(GE_INT3_SUB(point_Q16, L2));

        return L2;
    }
    else
    {
        *dist_Q16 = ge_length_v3_Q16(GE_INT3_SUB(point_Q16, L3));

        return L3;
    }

}



void Triangle3DMakeHeavy(Triangle3DHeavy* triangle)
{
    triangle->u_Q16 = GE_INT3_SUB(triangle->base.verts_Q16[1], triangle->base.verts_Q16[0]);//P_2 - P_1
    triangle->v_Q16 = GE_INT3_SUB(triangle->base.verts_Q16[2], triangle->base.verts_Q16[0]);//P_3 - P_1


    triangle->normal_Q16 =  GE_INT3_CROSS_PRODUCT_Q16(triangle->u_Q16, triangle->v_Q16);

    if (GE_INT3_DOT_PRODUCT_Q16(triangle->normal_Q16, triangle->normal_Q16) < (1 >> 5))
        printf("Warning! Small triangle!\n");

}
void Triangle3D_Make2Face(Triangle3DHeavy* triangle1, Triangle3DHeavy* triangle2, ge_int3* fourCorners)
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



void ParticleUpdate(ALL_CORE_PARAMS, Particle* p)
{
    p->pos.x = ADD_QMP32(p->pos.x, p->vel.x);
    p->pos.y = ADD_QMP32(p->pos.y, p->vel.y);
}




void PrintSelectionPeepStats(ALL_CORE_PARAMS, Peep* p)
{
///    Print_GE_INT3_Q16(p->physics.base.pos_Q16);
    Peep* peep = p;
    MapTile data[22];
    ge_int3 tileCenters_Q16[22];
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 0, 0 }, & data[0], & tileCenters_Q16[0]); printf("{ 1, 0, 0 }: %d\n", data[0]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 0, 0 }, & data[1], & tileCenters_Q16[1]); printf("{ -1, 0, 0 }: %d\n", data[1]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, -1, 0 }, & data[2], & tileCenters_Q16[2]); printf("{ 0, -1, 0 }: %d\n", data[2]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 1, 0 }, & data[3], & tileCenters_Q16[3]); printf("{ 0, 1, 0 }: %d\n", data[3]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 0, 1 }, & data[4], & tileCenters_Q16[4]); printf("{ 0, 0, 1 }: %d\n", data[4]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 0, -1 }, & data[5], & tileCenters_Q16[5]); printf("{ 0, 0, -1 }: %d\n", data[5]);

    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 0, -1 }, & data[6], & tileCenters_Q16[6]); printf("{ 1, 0, -1 }: %d\n", data[6]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 1, -1 }, & data[7], & tileCenters_Q16[7]); printf("{ 0, 1, -1 }: %d\n", data[7]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 1, -1 }, & data[8], & tileCenters_Q16[8]); printf("{ 1, 1, -1 }: %d\n", data[8]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 0, -1 }, & data[9], & tileCenters_Q16[9]); printf("{ -1, 0, -1 }: %d\n", data[9]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, -1, -1 }, & data[10], & tileCenters_Q16[10]); printf("{ 0, -1, -1 }: %d\n", data[10]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, -1, -1 }, & data[11], & tileCenters_Q16[11]); printf("{ -1, -1, -1 }: %d\n", data[11]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, -1, -1 }, & data[12], & tileCenters_Q16[12]); printf("{ 1, -1, -1 }: %d\n", data[12]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 1, -1 }, & data[13], & tileCenters_Q16[13]); printf("{ -1, 1, -1 }: %d\n", data[13]);

    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 0, 1 }, & data[14], & tileCenters_Q16[14]); printf("{ 1, 0, 1 }: %d\n", data[14]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, 1, 1 }, & data[15], & tileCenters_Q16[15]); printf("{ 0, 1, 1 }: %d\n", data[15]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, 1, 1 }, & data[16], & tileCenters_Q16[16]); printf("{ 1, 1, 1 }: %d\n", data[16]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 0, 1 }, & data[17], & tileCenters_Q16[17]); printf("{ -1, 0, 1 }: %d\n", data[17]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 0, -1, 1 }, & data[18], & tileCenters_Q16[18]); printf("{ 0, -1, 1 }: %d\n", data[18]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, -1, 1 }, & data[19], & tileCenters_Q16[19]); printf("{ -1, -1, 1 }: %d\n", data[19]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { 1, -1, 1 }, & data[20], & tileCenters_Q16[20]); printf("{ 1, -1, 1 }: %d\n", data[20]);
    //PeepGetMapTile(ALL_CORE_PARAMS_PASS, peep, (ge_int3) { -1, 1, 1 }, & data[21], & tileCenters_Q16[21]); printf("{ -1, 1, 1 }: %d\n", data[21]);
}


void GetMapTileCoordWithViewFromWorld2D(ALL_CORE_PARAMS, ge_int2 world_Q16, ge_int3* mapcoord_whole, int* occluded, int zViewRestrictLevel)
{
    ge_int3 wrld_Q16;
    wrld_Q16.x = world_Q16.x;
    wrld_Q16.y = world_Q16.y;
    WorldToMap(wrld_Q16, &(*mapcoord_whole));
    (*mapcoord_whole).x = WHOLE_Q16((*mapcoord_whole).x);
    (*mapcoord_whole).y = WHOLE_Q16((*mapcoord_whole).y);


    MapTileCoordClamp(mapcoord_whole);

    for (int z = zViewRestrictLevel; z >= 0; z--)
    {
        
        MapTile tile = gameState->map.levels[z].data[(*mapcoord_whole).x][(*mapcoord_whole).y];

        if (tile != MapTile_NONE)
        {
            (*mapcoord_whole).z = z;
            if (z == gameStateActions->mapZView-1)
            {
                *occluded = 1;
            }
            else if (z == gameStateActions->mapZView)
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


__kernel void game_apply_actions(ALL_CORE_PARAMS)
{
    cl_uint curPeepIdx = gameState->clientStates[gameStateActions->clientId].selectedPeepsLastIdx;
    PeepRenderSupport peepRenderSupport[MAX_PEEPS];
    while (curPeepIdx != OFFSET_NULL)
    {
        Peep* p = &gameState->peeps[curPeepIdx];

        gameState->clientStates[gameStateActions->clientId].peepRenderSupport[curPeepIdx].render_selectedByClient = 1;

        curPeepIdx = p->prevSelectionPeepIdx[gameStateActions->clientId];
    }




    //apply turns
    for (int32_t a = 0; a < gameStateActions->numActions; a++)
    {
        ClientAction* clientAction = &gameStateActions->clientActions[a].action;
        ActionTracking* actionTracking = &gameStateActions->clientActions[a].tracking;
        cl_uchar cliId = actionTracking->clientId;
        SynchronizedClientState* client = &gameState->clientStates[cliId];

        if (clientAction->actionCode == ClientActionCode_DoSelect)
        {
            client->selectedPeepsLastIdx = OFFSET_NULL;
            for (cl_uint pi = 0; pi < MAX_PEEPS; pi++)
            {
                Peep* p = &gameState->peeps[pi];

                if (p->stateBasic.faction == actionTracking->clientId)
                    if ((p->physics.base.pos_Q16.x > clientAction->intParameters[CAC_DoSelect_Param_StartX_Q16])
                        && (p->physics.base.pos_Q16.x < clientAction->intParameters[CAC_DoSelect_Param_EndX_Q16]))
                    {

                        if ((p->physics.base.pos_Q16.y < clientAction->intParameters[CAC_DoSelect_Param_StartY_Q16])
                            && (p->physics.base.pos_Q16.y > clientAction->intParameters[CAC_DoSelect_Param_EndY_Q16]))
                        {
                            
                            if (PeepMapVisiblity(ALL_CORE_PARAMS_PASS, p, clientAction->intParameters[CAC_DoSelect_Param_ZMapView]))
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
        else if (clientAction->actionCode == ClientActionCode_CommandToLocation)
        {
            
            cl_uint curPeepIdx = client->selectedPeepsLastIdx;
            ge_int3 mapcoord;
            ge_int2 world2D;
            cl_uchar pathFindSuccess;
            AStarPathNode* path;
            if (curPeepIdx != OFFSET_NULL)
            {
                //Do an AStarSearch
                Peep* curPeep = &gameState->peeps[curPeepIdx];
                world2D.x = clientAction->intParameters[CAC_CommandToLocation_Param_X_Q16];
                world2D.y = clientAction->intParameters[CAC_CommandToLocation_Param_Y_Q16];
                int occluded;

                GetMapTileCoordWithViewFromWorld2D(ALL_CORE_PARAMS_PASS, world2D, &mapcoord, &occluded, gameStateActions->mapZView);
                AStarSearchInstantiate(&gameState->mapSearchers[0]);
                ge_int3 start = GE_INT3_WHOLE_Q16(curPeep->posMap_Q16);
                mapcoord.z++;
                ge_int3 end = mapcoord;
                Print_GE_INT3(start);
                Print_GE_INT3(end);
                pathFindSuccess = AStarSearchRoutine(ALL_CORE_PARAMS_PASS, &gameState->mapSearchers[0], start, end, CL_INTMAX);
                if (pathFindSuccess != 0)
                {
                    printf("--------------------------\n");
                    AStarPrintSearchPathFrom(&gameState->mapSearchers[0], start);
                    printf("--------------------------\n");
                    AStarPrintSearchPathTo(&gameState->mapSearchers[0], end);
                    printf("--------------------------\n");


                    path = AStarFormPathSteps(ALL_CORE_PARAMS_PASS , &gameState->mapSearchers[0], &gameState->paths);
                    CL_CHECK_NULL(path);
                    AStarPrintPath(path);
                    printf("--------------------------\n");
                }
            }

            while (curPeepIdx != OFFSET_NULL)
            {
                Peep* curPeep = &gameState->peeps[curPeepIdx];

                if (pathFindSuccess != 0)
                {
                    curPeep->physics.drive.next = path;
                    ge_int3 worldloc;
                    MapToWorld(curPeep->physics.drive.next->mapCoord_Q16, &worldloc);
                    curPeep->physics.drive.target_x_Q16 = worldloc.x;
                    curPeep->physics.drive.target_y_Q16 = worldloc.y;
                    curPeep->physics.drive.drivingToTarget = 1;

                    //restrict comms to new channel
                    curPeep->comms.orders_channel = RandomRange(client->selectedPeepsLastIdx, 0, 10000);
                    curPeep->comms.message_TargetReached = 0;
                    curPeep->comms.message_TargetReached_pending = 0;
                }
                curPeepIdx = curPeep->prevSelectionPeepIdx[cliId];
            }
        }
        else if (clientAction->actionCode == ClientActionCode_CommandTileDelete)
        {
            printf("Got the command\n");

            ge_int2 world2DMouse;
            world2DMouse.x = clientAction->intParameters[CAC_CommandTileDelete_Param_X_Q16];
            world2DMouse.y = clientAction->intParameters[CAC_CommandTileDelete_Param_Y_Q16];

            ge_int3 mapCoord;
            int occluded;
            GetMapTileCoordWithViewFromWorld2D(ALL_CORE_PARAMS_PASS, world2DMouse, &mapCoord, &occluded, gameStateActions->mapZView+1);
            if (mapCoord.z > 0) 
            {
                gameState->map.levels[mapCoord.z].data[mapCoord.x][mapCoord.y] = MapTile_NONE;

                MapBuildTileView(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y);

                MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y);
                MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x + 1, mapCoord.y);
                MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x - 1, mapCoord.y);
                MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y + 1);
                MapUpdateShadow(ALL_CORE_PARAMS_PASS, mapCoord.x, mapCoord.y - 1);
            }

        }


    }

    gameStateActions->numActions = 0;
}

ge_int3 


__kernel void game_init_single(ALL_CORE_PARAMS)
{
    printf("Game Initializing...\n");




    printf("Initializing StaticData Buffer..\n");
    MakeCardinalDirectionOffsets(&staticData->directionalOffsets[0]);

    printf("Speed Tests:\n");



    int s = 0;
    for (cl_ulong i = 0; i < 0; i++)
    {
        ge_int3 a = (ge_int3){ TO_Q16(i), TO_Q16(2), TO_Q16(i*2) };
        ge_int3 b = (ge_int3){ TO_Q16(i*2), TO_Q16(i), TO_Q16(i) };

        ge_int3 c = GE_INT3_MUL_Q16(a, b);
        s += c.x + c.y + c.z;
    }





    printf("End Tests: %d\n", s);

    fixedPointTests();

    printf("Triangle Tests\n");


    ge_int3 point = (ge_int3){TO_Q16(1), TO_Q16(0), TO_Q16(1) };
    Triangle3DHeavy tri;
    tri.base.verts_Q16[0] = (ge_int3){ TO_Q16(-1), TO_Q16(-1), TO_Q16(0) };
    tri.base.verts_Q16[1] = (ge_int3){ TO_Q16( 1), TO_Q16(-1), TO_Q16(0) };
    tri.base.verts_Q16[2] = (ge_int3){ TO_Q16(-1), TO_Q16( 1), TO_Q16(0) };

    Triangle3DMakeHeavy(&tri);

    int dist;
    ge_int3 closestPoint = Triangle3DHeavy_ClosestPoint(&tri, point, &dist);
    printf("CP: ");
    Print_GE_INT3_Q16(closestPoint);
    printf("Dist: ");
    PrintQ16(dist);









    printf("Convex Hull Tests:\n");



    ConvexHull hull;    
    cl_int tileData = 1;
    MapTileConvexHull_From_TileData(&hull, &tileData);

    printf("convex hull tris:\n");
    for (int i = 0; i < 14; i++)
    {
        Print_GE_INT3_Q16(hull.triangles[i].base.verts_Q16[0]);
        Print_GE_INT3_Q16(hull.triangles[i].base.verts_Q16[1]);
        Print_GE_INT3_Q16(hull.triangles[i].base.verts_Q16[2]);
    }


    ge_int3 p = (ge_int3){TO_Q16(0),TO_Q16(0),TO_Q16(2)};
    ge_int3 nearestPoint = MapTileConvexHull_ClosestPoint(&hull, p);

    printf("np: ");
    Print_GE_INT3_Q16(nearestPoint);








    printf("End Tests\n");

    gameState->numClients = 1;
    gameStateActions->pauseState = 0;
    gameStateActions->mapZView = MAPDEPTH-1;
    gameStateActions->mapZView_1 = 0;

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





 
}

__kernel void game_init_multi(ALL_CORE_PARAMS)
{
    int globalid = get_global_id(0);
    int localid = get_local_id(0);

    MapCreate(ALL_CORE_PARAMS_PASS, globalid % MAPDIM, globalid / MAPDIM);

}
__kernel void game_init_multi2(ALL_CORE_PARAMS)
{
    int globalid = get_global_id(0);
    int localid = get_local_id(0);

    MapCreate2(ALL_CORE_PARAMS_PASS, globalid % MAPDIM, globalid / MAPDIM);
}
__kernel void game_init_single2(ALL_CORE_PARAMS)
{
    for (int i = 0; i < ASTARPATHSTEPSSIZE; i++)
    {
        AStarInitPathNode(&gameState->paths.pathNodes[i]);
    }
    gameState->paths.nextListIdx = 0;



    printf("AStarTests:\n");
    //printf("AStarTests1:\n");
    //AStarSearchInstantiate(&gameState->mapSearchers[0]);
    ////test AStarHeap
    //for (int x = 0; x < MAPDIM; x++)
    //{
    //    for (int y = 0; y < MAPDIM; y++)
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
    AStarSearchInstantiate(&gameState->mapSearchers[0]);
    ge_int3 start = (ge_int3){ 0,0,MAPDEPTH - 2 };
    ge_int3 end = (ge_int3){ MAPDIM - 1,MAPDIM - 1,1 };
    AStarSearchRoutine(ALL_CORE_PARAMS_PASS, &gameState->mapSearchers[0], start, end, CL_INTMAX);

    const int spread = 500;
    for (cl_uint p = 0; p < MAX_PEEPS; p++)
    {
        gameState->peeps[p].Idx = p;
        gameState->peeps[p].physics.base.pos_Q16.x = RandomRange(p, -spread << 16, spread << 16);
        gameState->peeps[p].physics.base.pos_Q16.y = RandomRange(p + 1, -spread << 16, spread << 16);


        ge_int3 mapcoord;
        ge_int2 world2D;
        world2D.x = gameState->peeps[p].physics.base.pos_Q16.x;
        world2D.y = gameState->peeps[p].physics.base.pos_Q16.y;
        int occluded;

        GetMapTileCoordWithViewFromWorld2D(ALL_CORE_PARAMS_PASS, world2D, &mapcoord, &occluded, MAPDEPTH - 1);
        //printf("%d\n", mapcoord.z);
        mapcoord.z += 2;
        mapcoord = GE_INT3_TO_Q16(mapcoord);

        ge_int3 worldCoord;
        MapToWorld(mapcoord, &worldCoord);
        gameState->peeps[p].physics.base.pos_Q16.z = worldCoord.z;
        WorldToMap(gameState->peeps[p].physics.base.pos_Q16, &gameState->peeps[p].posMap_Q16);
        gameState->peeps[p].lastGoodPosMap_Q16 = gameState->peeps[p].posMap_Q16;

        gameState->peeps[p].physics.shape.radius_Q16 = TO_Q16(1);
        BITCLEAR(gameState->peeps[p].stateBasic.bitflags0, PeepState_BitFlags_deathState);
        BITSET(gameState->peeps[p].stateBasic.bitflags0, PeepState_BitFlags_valid);
        BITSET(gameState->peeps[p].stateBasic.bitflags0, PeepState_BitFlags_visible);
        gameState->peeps[p].stateBasic.health = 10;
        gameState->peeps[p].stateBasic.deathState = 0;

        gameState->peeps[p].physics.base.v_Q16 = (ge_int3){ 0,0,0 };
        gameState->peeps[p].physics.base.vel_add_Q16 = (ge_int3){ 0,0,0 };
        gameState->peeps[p].physics.base.pos_post_Q16 = (ge_int3){ 0,0,0 };


        gameState->peeps[p].minDistPeepIdx = OFFSET_NULL;
        gameState->peeps[p].minDistPeep_Q16 = (1 << 30);
        gameState->peeps[p].mapSectorIdx = GE_OFFSET_NULL_2D;
        gameState->peeps[p].mapSector_pendingIdx = GE_OFFSET_NULL_2D;
        gameState->peeps[p].nextSectorPeepIdx = OFFSET_NULL;
        gameState->peeps[p].prevSectorPeepIdx = OFFSET_NULL;
        gameState->peeps[p].physics.drive.target_x_Q16 = gameState->peeps[p].physics.base.pos_Q16.x;
        gameState->peeps[p].physics.drive.target_y_Q16 = gameState->peeps[p].physics.base.pos_Q16.y;
        gameState->peeps[p].physics.drive.drivingToTarget = 0;

        if (gameState->peeps[p].physics.base.pos_Q16.x > 0)
            gameState->peeps[p].stateBasic.faction = 0;
        else
            gameState->peeps[p].stateBasic.faction = 1;


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

        PeepAssignToSector_Detach(ALL_CORE_PARAMS_PASS, p);
        PeepAssignToSector_Insert(ALL_CORE_PARAMS_PASS, p);

    }

    printf("Peep Sector Assigment Finished\n");




    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        Particle* p = &gameState->particles[i];

        p->pos.x = FloatToQMP32((float)RandomRange(i, -spread, spread));
        p->pos.y = FloatToQMP32((float)RandomRange(i + 1, -spread, spread));

        p->vel.x = FloatToQMP32(((float)RandomRange(i, -1000, 1000)) * 0.001f);
        p->vel.y = FloatToQMP32(((float)RandomRange(i + 1, -1000, 1000)) * 0.001f);
    }


}
void PeepDraw(ALL_CORE_PARAMS, Peep* peep)
{
    float3 drawColor;
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

    if (gameState->clientStates[gameStateActions->clientId].peepRenderSupport[peep->Idx].render_selectedByClient)
    {
        brightFactor = 1.0f;
        gameState->clientStates[gameStateActions->clientId].peepRenderSupport[peep->Idx].render_selectedByClient = 0;
    }
    if ( BITGET(peep->stateBasic.bitflags0, PeepState_BitFlags_deathState) )
    {
        brightFactor = 0.6f;
        drawColor.x = 0.5f;
        drawColor.y = 0.5f;
        drawColor.z = 0.5f;
    }
    if (!BITGET(peep->stateBasic.bitflags0, PeepState_BitFlags_visible))
    {
        drawPosX = 99999;
        drawPosY = 99999;
    }


    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 0] = drawPosX;
    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 1] = drawPosY;

    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 2] = drawColor.x * brightFactor;
    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 3] = drawColor.y * brightFactor;
    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 4] = drawColor.z * brightFactor;

    peepVBOBuffer[peep->Idx * (PEEP_VBO_INSTANCE_SIZE / sizeof(float)) + 5] = peep->physics.base.CS_angle_rad;
}

void ParticleDraw(ALL_CORE_PARAMS, Particle* particle, cl_uint idx)
{
    float3 drawColor;
    float drawPosX = (float)((float)particle->pos.x.number / (1 << particle->pos.x.q));
    float drawPosY = (float)((float)particle->pos.y.number / (1 << particle->pos.y.q));


    drawColor.x = 1.0f;
    drawColor.y = 1.0f;
    drawColor.z = 1.0f;
  

    float brightFactor = 1.0f;

    particleVBOBuffer[idx * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 0] = drawPosX;
    particleVBOBuffer[idx * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 1] = drawPosY;

    particleVBOBuffer[idx * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 2] = drawColor.x * brightFactor;
    particleVBOBuffer[idx * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 3] = drawColor.y * brightFactor;
    particleVBOBuffer[idx * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 4] = drawColor.z * brightFactor;

    particleVBOBuffer[idx * (PARTICLE_VBO_INSTANCE_SIZE / sizeof(float)) + 5] = 0;
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

    if (globalid < MAX_PARTICLES) {
        Particle* p = &gameState->particles[globalid];
        ParticleUpdate(ALL_CORE_PARAMS_PASS, p);
        ParticleDraw(ALL_CORE_PARAMS_PASS, p, globalid);

    }


    //update map view
    if (gameStateActions->mapZView != gameStateActions->mapZView_1)
    {
        cl_uint chunkSize = (MAPDIM * MAPDIM) / GAME_UPDATE_WORKITEMS;
        if (chunkSize == 0)
            chunkSize = 1;


        for (cl_ulong i = 0; i < chunkSize; i++)
        {
            cl_ulong xyIdx = i + globalid * chunkSize;
            
            if (xyIdx < (MAPDIM * MAPDIM))
            {
                MapBuildTileView(ALL_CORE_PARAMS_PASS, xyIdx % MAPDIM, xyIdx / MAPDIM);
                MapUpdateShadow(ALL_CORE_PARAMS_PASS, xyIdx % MAPDIM, xyIdx / MAPDIM);
            }
        }
    }
    
}


__kernel void game_post_update_single( ALL_CORE_PARAMS)
{

    gameStateActions->mapZView_1 = gameStateActions->mapZView;



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

            PeepAssignToSector_Detach(ALL_CORE_PARAMS_PASS, p);

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

            PeepAssignToSector_Insert(ALL_CORE_PARAMS_PASS, p);

            atomic_dec(lock);
        }
    }
}


__kernel void size_tests(__global SIZETESTSDATA* data)
{

    data->gameStateStructureSize = sizeof(GameState);

}