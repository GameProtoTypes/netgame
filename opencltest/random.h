#pragma once

#include "random_generators/isaac.cl"


//get uniform random from min to max. max excluded.
int RandomRange(int seed, int min, int max) {

    isaac_state state;
    isaac_seed(&state, seed);
    uint sample = isaac_uint(state);
    int x = sample % (max - min-1) + min;
    printf("%d,%d,  x: %d\n",min,max, x);
    return x;
}
