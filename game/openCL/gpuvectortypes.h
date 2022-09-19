#pragma once


typedef int2 ge_int2;
typedef int3 ge_int3;
typedef int4 ge_int4;
typedef uint2 ge_uint2;
typedef uint3 ge_uint3;
typedef uint4 ge_uint4;
typedef short3 ge_short3;



typedef unsigned int offsetPtr;
typedef ge_uint2 offsetPtr2;
typedef ge_uint3 offsetPtr3;

#define INT2_ZERO(a) (a.x == 0 && a.y == 0 ).
#define INT3_ZERO(a) (a.x == 0 && a.y == 0 && a.z == 0 )

#define INT2_ADD(a,b) ((ge_int2){a.x+b.x,a.y+b.y})
#define INT3_ADD(a,b) ((ge_int3){a.x+b.x,a.y+b.y,a.z+b.z})
#define INT4_ADD(a,b) ((ge_int4){a.x+b.x,a.y+b.y,a.z+b.z, a.w+b.w})

#define INT2_SUB(a,b) ((ge_int2){a.x-b.x,a.y-b.y})
#define INT3_SUB(a,b) ((ge_int3){a.x-b.x,a.y-b.y,a.z-b.z})
#define INT4_SUB(a,b) ((ge_int4){a.x-b.x,a.y-b.y,a.z-b.z, a.w-b.w})



#define INT2_NEG(a) ((ge_int2){-a.x, -a.y            })
#define INT3_NEG(a) ((ge_int3){-a.x, -a.y, -a.z      })
#define INT4_NEG(a) ((ge_int4){-a.x, -a.y, -a.z, -a.w})

#define INT3_LFTSHIFT(a,s) ((ge_int3){a.x<<s,a.y<<s,a.z<<s})
#define INT4_LFTSHIFT(a,s) ((ge_int4){a.x<<s,a.y<<s,a.z<<s,a.w<<s})


#define VECTOR2_EQUAL(a,b) ((a.x == b.x) && (a.y == b.y))
#define VECTOR3_EQUAL(a,b) ((a.x == b.x) && (a.y == b.y) && (a.z == b.z))



#define SHORT3_TO_INT3(a) ((ge_int3){a.x, a.y, a.z})






void Print_GE_INT2(ge_int2 v);
void Print_GE_INT3(ge_int3 v);
void Print_GE_SHORT3(ge_short3 v);

