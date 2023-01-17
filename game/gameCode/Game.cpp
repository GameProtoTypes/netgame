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






