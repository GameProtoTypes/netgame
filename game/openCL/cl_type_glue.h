#pragma once

//typedefs to use cL_**** types and _t types from host api direct in opencl c



typedef char        cl_char;
typedef uchar        cl_uchar;
typedef short       cl_short;
typedef ushort        cl_ushort;
typedef int        cl_int;
typedef uint        cl_uint;
typedef long        cl_long;
typedef ulong        cl_ulong;

typedef half       cl_half;
typedef float                   cl_float;
typedef double                  cl_double;

typedef float2 cl_float2;
typedef float3 cl_float3;
typedef float4 cl_float4;

typedef int2 cl_int2;
typedef int3 cl_int3;
typedef int4 cl_int4;

typedef uint2 cl_uint2;
typedef uint3 cl_uint3;
typedef uint4 cl_uint4;


typedef long2 cl_long2;
typedef long3 cl_long3;
typedef long4 cl_long4;








typedef int int32_t;
typedef uint uint32_t;
typedef char int8_t;
typedef uchar uint8_t;
typedef long int64_t;
typedef ulong uint64_t;




#define CL_VECTOR2_EQUAL(a,b) ((a.x == b.x) && (a.y == b.y))

#define CL_INTMAX (0x7FFFFFFF)

