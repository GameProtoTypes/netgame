#pragma once

#include "cpugpuvectortypes.h"
#include "cl_type_glue.h"
#include "fixedpoint_opencl.h"
#include "randomcl.h"
#include "perlincl.h"



#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

#define ALL_CORE_PARAMS  __global StaticData* staticData, \
__global GameState* gameState,\
__global GameStateActions* gameStateActions, \
__global float* peepVBOBuffer, \
__global float* particleVBOBuffer, \
__global cl_uchar* mapTile1VBO, \
__global cl_uint* mapTile1AttrVBO, \
__global cl_uint* mapTile1OtherAttrVBO, \
__global cl_uchar* mapTile2VBO, \
__global cl_uint* mapTile2AttrVBO, \
__global cl_uint* mapTile2OtherAttrVBO


#define ALL_CORE_PARAMS_PASS  staticData, \
gameState, \
gameStateActions, \
peepVBOBuffer, \
particleVBOBuffer, \
mapTile1VBO, \
mapTile1AttrVBO, \
mapTile1OtherAttrVBO, \
mapTile2VBO, \
mapTile2AttrVBO, \
mapTile2OtherAttrVBO



#include "peep.h"

