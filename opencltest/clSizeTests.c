#include "peep.h"

__kernel void size_tests(__global SIZETESTSDATA* data)
{

    data->gameStateStructureSize = sizeof(GameState);


}
