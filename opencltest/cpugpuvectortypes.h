#pragma once

struct ge_int2
{
	int x;
	int y;
}typedef ge_int2;

struct ge_int3
{
	int x;
	int y;
	int z;
}typedef ge_int3;

struct ge_short3
{
	short x;
	short y;
	short z;
}typedef ge_short3;



struct ge_uint2
{
	unsigned int x;
	unsigned int y;
}typedef ge_uint2;

struct ge_int4
{
	int x;
	int y;
	int z;
	int w;
}typedef ge_int4;



#define GE_INT2_ADD(a,b) ((ge_int2){a.x+b.x,a.y+b.y})
#define GE_INT3_ADD(a,b) ((ge_int3){a.x+b.x,a.y+b.y,a.z+b.z})
#define GE_INT4_ADD(a,b) ((ge_int4){a.x+b.x,a.y+b.y,a.z+b.z, a.w+b.w})

#define GE_INT2_NEG(a) ((ge_int2){-a.x, -a.y            })
#define GE_INT3_NEG(a) ((ge_int3){-a.x, -a.y, -a.z      })
#define GE_INT4_NEG(a) ((ge_int4){-a.x, -a.y, -a.z, -a.w})

#define GE_INT3_LFTSHIFT(a,s) ((ge_int3){a.x<<s,a.y<<s,a.z<<s})
#define GE_INT4_LFTSHIFT(a,s) ((ge_int4){a.x<<s,a.y<<s,a.z<<s,a.w<<s})


#define GE_VECTOR2_EQUAL(a,b) ((a.x == b.x) && (a.y == b.y))
#define GE_VECTOR3_EQUAL(a,b) ((a.x == b.x) && (a.y == b.y) && (a.z == b.z))



#define GE_SHORT3_TO_INT3(a) ((ge_int3){a.x, a.y, a.z})





void Print_GE_INT2(ge_int2 v);
void Print_GE_INT3(ge_int3 v);
void Print_GE_SHORT3(ge_short3 v);