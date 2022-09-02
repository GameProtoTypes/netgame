#pragma once

//#include "random_generators/isaac.cl"
//#include "random_generators/xorshift1024.cl"
#include "random_generators/tyche.cl"


//get uniform random from min to max. max excluded.
int RandomRange(int seed, int min, int max) {

    tyche_state state;
    tyche_seed(&state, seed);
    uint sample = tyche_uint(state);
    int x = sample % (max - min) + min;
   // printf("%d,%d,  x: %d\n",min,max, x);
    return x;
}
