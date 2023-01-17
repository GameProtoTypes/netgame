#include "gameCodeCommon.h"


void StartupTests()
{

    int s = 0;
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


    ge_int a = GE_TO_Q<24>(1.0f);
    ge_int b = GE_TO_Q<24>(2.0f);
    ge_int c = GE_DIV_Q<16,24,24>(a,b);
    GE_PRINT_Q<16>(length);
    //GE_PRINT(GE_SIGNED_SHIFT<ge_int, -1>(a));




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

    v = (ge_int2)(GE_TO_Q(1), GE_TO_Q(1));
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




void Kernel_A(ALL_CORE_PARAMS) 
{

    printf("tick: %d, threadIdx: %d\n", gameState->tickidx, threadIdx );
    StartupTests();


 
}
void Kernel_B(ALL_CORE_PARAMS) 
{



}
void Kernel_C(ALL_CORE_PARAMS) 
{


}
void Kernel_D(ALL_CORE_PARAMS) 
{


}






