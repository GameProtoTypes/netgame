module;

#include <type_traits>
#include <numbers>
#include <gcem.hpp>


#include <ext.hpp>
#include <cstdlib>


export module GE.Basic:Types; // defines a module partition, Point, that's part of the module BasicPlane.Figures



export namespace GE {

const int DEFAULT_Q = (16);

typedef int16_t ge_short;
typedef uint16_t ge_ushort;
typedef int32_t ge_int;
typedef uint32_t ge_uint;
typedef int64_t ge_long;
typedef uint64_t ge_ulong;
typedef float ge_float;
typedef int8_t ge_byte;
typedef uint8_t ge_ubyte;

struct ge_int2
{
    ge_int x,y;
};
struct ge_int3
{
    ge_int x,y,z;
};
struct ge_int4
{
    ge_int x,y,z,w;
};
struct ge_long2
{
    ge_long x,y;
};
struct ge_long3
{
    ge_long x,y,z;
};
struct ge_long4
{
    ge_long x,y,z,w;
};

struct ge_ulong2
{
    ge_ulong x,y;
};
struct ge_ulong3
{
    ge_ulong x,y,z;
};
struct ge_ulong4
{
    ge_ulong x,y,z,w;
};


struct ge_uint2
{
    ge_uint x,y;
};
struct ge_uint3
{
    ge_uint x,y,z;
};
struct ge_uint4
{
    ge_uint x,y,z,w;
};

struct ge_short2
{
    ge_short x,y;
};
struct ge_short3
{
    ge_short x,y,z;
};
struct ge_ushort3
{
    ge_ushort x,y,z;
};

struct ge_float2
{
    ge_float x,y;
};
struct ge_float3
{
    ge_float x,y,z;
};
struct ge_float4
{
    ge_float x,y,z,w;
};


//specializations

//Base Types
template<typename T>
struct GETypeConversions;



template<>
struct GETypeConversions<ge_int> {
    typedef ge_int baseType;
    typedef ge_long paddedType;
    typedef ge_long paddedBaseType;
};
template<>
struct GETypeConversions<ge_uint> {
    typedef ge_uint baseType;
    typedef ge_ulong paddedType;
    typedef ge_ulong paddedBaseType;
};





template<>
struct GETypeConversions<ge_int2> {
    typedef ge_int baseType;
    typedef ge_long2 paddedType;
    typedef ge_long  paddedBaseType;
};

template<>
struct GETypeConversions<ge_int3> {
    typedef ge_int baseType;
    typedef ge_long3 paddedType;
    typedef ge_long  paddedBaseType;
};

template<>
struct GETypeConversions<long> {
    typedef long baseType;
    typedef long long paddedType;
    typedef long long paddedBaseType;
};

template<>
struct GETypeConversions<ge_long> {
    typedef ge_long baseType;
    typedef ge_long paddedType;
    typedef ge_long paddedBaseType;
};

template<>
struct GETypeConversions<ge_long2> {
    typedef ge_long baseType;
    typedef ge_long2 paddedType;
    typedef ge_long paddedBaseType;
};
template<>
struct GETypeConversions<ge_long3> {
    typedef ge_long baseType;
    typedef ge_long3 paddedType;
    typedef ge_long paddedBaseType;
};













typedef ge_uint ge_offsetPtr;
typedef ge_uint2 ge_offsetPtr2;
typedef ge_uint3 ge_offsetPtr3;
typedef ge_ushort3 ge_offsetPtrShort3;

const ge_offsetPtr GE_OFFSET_NULL = (0xFFFFFFFF);
const ge_offsetPtr2 GE_OFFSET_NULL_2D  = {0xFFFFFFFF , 0xFFFFFFFF};
const ge_offsetPtr3 GE_OFFSET_NULL_3D  {0xFFFFFFFF , 0xFFFFFFFF,  0xFFFFFFFF};

const ge_offsetPtrShort3 GE_OFFSET_NULL_SHORT_3D = {0xFFFF , 0xFFFF,  0xFFFF};





const ge_int perlin_hash_numbers[] = { 208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
                     185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
                     9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
                     70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
                     203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
                     164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
                     228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
                     232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
                     193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
                     101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
                     135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
                     114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219 };






template <class T>
constexpr T GE_CLAMP(const T& ge, const T& min, const T& max)
{
    return glm::clamp<T>(ge,min,max);
}

template <class T>
constexpr T GE_MAX(const T& geA, const T& geB)
{
    return glm::max<T>(geA,geB);
}
template <class T>
constexpr T GE_MIN(const T& geA, const T& geB)
{
    return glm::min<T>(geA,geB);
}

template <class T>
constexpr T GE_ABS(const T& geX)
{
    return glm::abs<T>(geX);
}

template <class T>
constexpr T GE_SIN(const T& geX)
{
    return glm::sin(geX);
}


template <class T>
constexpr T GE_ATAN(const T& geX, const T& geY)
{
    return glm::atan<T>(geX,geY);
}


#define TYCHE_ROT(a,b,bits) (((a) << (b)) | ((a) >> (bits - (b))))
template < class T, class UNSIGNED_T >
T GE_UNIFORM_RANDOM_RANGE( UNSIGNED_T seed, const T& min, const T& max)
{
    UNSIGNED_T a = seed;
	a = TYCHE_ROT(a * a, 16, sizeof(UNSIGNED_T)*8);
    a = TYCHE_ROT(a * a, 12, sizeof(UNSIGNED_T)*8);
    a = TYCHE_ROT(a * a, 8 , sizeof(UNSIGNED_T)*8);
    a = TYCHE_ROT(a * a, 7 , sizeof(UNSIGNED_T)*8);
    // a = TYCHE_ROT(a * a, 5 , sizeof(UNSIGNED_T)*8);
    // a = TYCHE_ROT(a * a, 3 , sizeof(UNSIGNED_T)*8);
    // a = TYCHE_ROT(a * a, 2 , sizeof(UNSIGNED_T)*8);

    return T(UNSIGNED_T(a)%(max-min)) + min;
}






template <class T>
constexpr T GE2_ADD(const T& a, const T& b)
{
    return T{a.x+b.x, a.y+b.y};
}
template <class T>
constexpr T GE2_SUB(const T& a, const T& b)
{
    return T{a.x-b.x, a.y-b.y};
}
template <class T>
constexpr T GE2_MUL(const T& a, const T& b)
{
    return T{a.x*b.x, a.y*b.y};
}
template <class T>
constexpr T GE2_DIV(const T& a, const T& b)
{
    return T{a.x/b.x, a.y/b.y};
}
template <class T, class ScalarT>
constexpr T GE2_DIV(const T& a, const ScalarT& b)
{
    return T{a.x/b, a.y/b};
}
template <class T>
constexpr bool GE2_ISZERO(const T& ge2)
{
    return (ge2.x == 0 && ge2.y == 0);
}





constexpr ge_int2 operator+(const ge_int2& ge2A,const ge_int2& ge2B)
{
    return GE2_ADD(ge2A,ge2B);
}
constexpr ge_short2 operator+(const ge_short2& ge2A, const ge_short2& ge2B)
{
    return GE2_ADD(ge2A, ge2B);
}
constexpr ge_long2 operator+(const ge_long2& ge2A, const ge_long2& ge2B)
{
    return GE2_ADD(ge2A, ge2B);
}
constexpr ge_float2 operator+(const ge_float2& ge2A, const ge_float2& ge2B)
{
    return GE2_ADD(ge2A, ge2B);
}





template <class T>
constexpr T GE3_ADD(const T& a, const T& b)
{
    return T{a.x+b.x, a.y+b.y, a.z+b.z};
}
template <class T>
constexpr T GE3_SUB(const T& a, const T& b)
{
    return T{a.x-b.x, a.y-b.y, a.z-b.z};
}
template <class T>
constexpr T GE3_MUL(const T& a, const T& b)
{
    return T{a.x*b.x, a.y*b.y, a.z*b.z};
}
template <class T, typename SCALAR_T>
constexpr T GE3_MUL(const T& a, const SCALAR_T& scalar)
{
    return T{a.x*scalar, a.y*scalar, a.z*scalar};
}

template <class T>
constexpr T GE3_DIV(const T& a, const T& b)
{
    return T{a.x/b.x, a.y/b.y, a.z/b.z};
}
template <class T,  class ScalarT>
constexpr T GE3_DIV(const T& a, const ScalarT& b)
{
    return T{a.x/b, a.y/b, a.z/b};
}
template <class T>
constexpr bool GE3_ISZERO(const T& ge3)
{
    return (ge3.x == 0 && ge3.y == 0 && ge3.z == 0);
}

template <class GE3_T>
GE3_T operator*(const GE3_T& ge3A,const GE3_T& ge3B)
{
    return GE3_MUL(ge3A,ge3B);
}
template <class GE3_T, class GE_T>
GE3_T operator*(const GE3_T& ge3A,const GE_T& ge)
{
    return GE3_MUL(ge3A,ge);
}



ge_int3 operator+(const ge_int3& ge3A, const ge_int3& ge3B)
{
    return GE3_ADD(ge3A, ge3B);
}



template <class T>
constexpr T GE4_ADD(const T& a, const T& b)
{
    return T{a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w};
}
template <class T>
constexpr T GE4_SUB(const T& a, const T& b)
{
    return T{a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w};
}
template <class T>
constexpr T GE4_MUL(const T& a, const T& b)
{
    return T{a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w};
}
template <class T>
constexpr T GE4_DIV(const T& a, const T& b)
{
    return T{a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w};
}
template <class T>
constexpr bool GE4_ISZERO(const T& ge4)
{
    return (ge4.x == 0 && ge4.y == 0 && ge4.z == 0 && ge4.z == 0);
}


template <typename T, typename FROMT>
constexpr T GE2_CAST(const FROMT& ge2 )
{
    return T(ge2.x, ge2.y);
}
template <typename T, typename FROMT>
constexpr T GE3_CAST(const FROMT& ge3 )
{
    return T(ge3.x, ge3.y, ge3.z);
}
template <typename T, typename FROMT>
constexpr T GE4_CAST(const FROMT& ge4 )
{
    return T(ge4.x, ge4.y, ge4.z, ge4.w);
}




template <typename T>
constexpr T GE4_NEG(const T& ge4)
{
    return T{-ge4.x, -ge4.y, -ge4.z, -ge4.w};
}
template <typename T>
constexpr T GE3_NEG(const T& ge3)
{
    return T{-ge3.x, -ge3.y, -ge3.z};
}
template <typename T>
constexpr T GE2_NEG(const T& ge2)
{
    return T{-ge2.x, -ge2.y};
}


template <typename T, int shift>
constexpr T GE_LEFTSHIFT(const T& ge)
{
    return T{ge.x << shift};
}
template <typename T, int shift>
constexpr T GE2_LEFTSHIFT(const T& ge2)
{
    return T{ge2.x << shift, ge2.y << shift};
}
template <typename T, int shift>
constexpr T GE3_LEFTSHIFT(const T& ge3)
{
    return T{ge3.x << shift, ge3.y << shift,ge3.z << shift};
}
template <int shift, typename T>
constexpr T GE4_LEFTSHIFT(const T& ge4)
{
    return T{ge4.x << shift, ge4.y << shift, ge4.z << shift, ge4.w << shift};
}

template <typename T, int shift>
constexpr T GE_SIGNED_SHIFT(const T& ge)
{
    if constexpr (shift < 0)
        return T{ge << -shift};
    else
        return T{ge >> shift};
}

template <typename GE2_T_A = ge_int2, typename GE2_T_B = ge_int2>
constexpr bool GE2_EQUAL(const GE2_T_A& a, const GE2_T_B& b)
{
    return ((a.x == b.x) && (a.y == b.y));
}
template <typename GE3_T_A = ge_int3, typename GE3_T_B = ge_int3>
constexpr bool GE3_EQUAL(const GE3_T_A& a, const GE3_T_B& b)
{
    return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
}

template <typename GE4_T= ge_int4>
constexpr bool GE4_EQUAL(const GE4_T& a, const GE4_T& b)
{
    return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w));
}

template <typename GE4_T = ge_int4, typename FROMT = ge_int3>
constexpr GE4_T GE3_TO_GE4_ZERO_PAD(const FROMT& ge3)
{
    return GE4_T{ge3.x, ge3.y, ge3.z, 0};
}
template <typename GE4_T = ge_int4, typename FROMT = ge_int2>
constexpr GE4_T GE2_TO_GE4_ZERO_PAD(const FROMT& ge2)
{
    return GE4_T{ge2.x, ge2.y, 0, 0};
}
template <typename GE4_T = ge_int4, typename FROMT = ge_int>
constexpr GE4_T GE_TO_GE4_ZERO_PAD(const FROMT& ge)
{
    return GE4_T{ge.x, 0, 0, 0};
}

template <typename GE3_T = ge_int3, typename FROMT = ge_int2>
constexpr GE3_T GE2_TO_GE3_ZERO_PAD(const FROMT& ge2)
{
    return GE3_T{ge2.x, ge2.y, 0};
}
template <typename GE3_T = ge_int4, typename FROMT = ge_int>
constexpr GE3_T GE_TO_GE3_ZERO_PAD(const FROMT& ge)
{
    return GE3_T{ge.x, 0, 0};
}

template <typename GE2_T = ge_int2, typename FROMT = ge_int>
constexpr GE2_T GE_TO_GE2_ZERO_PAD(const FROMT& ge)
{
    return GE2_T{ge.x, 0};
}



#ifndef INT_MAX
#define INT_MAX (2147483647)
#endif





template <typename GE_T>
void GE_PRINT(GE_T v)
{
    if constexpr (std::is_same<GE_T, ge_int>())
    {
        printf("{%d}\n", v);
    }
    else if constexpr (std::is_same<GE_T, ge_long>())
    {
        printf("{%d}\n", v);
    }
    else if constexpr (std::is_same<GE_T, ge_short>())
    {
        printf("{%d}\n", v);
    }
    else if constexpr (std::is_same<GE_T, ge_float>())
    {
        printf("{%f}\n", v);
    }
}

template <typename GE_T>
void GE2_PRINT(GE_T v)
{
    if constexpr (std::is_same<GE_T, ge_int2>())
    {
        printf("{%d,%d}\n", v.x, v.y);
    }
    else if constexpr (std::is_same<GE_T, ge_long2>())
    {
        printf("{%d,%d}\n", v.x, v.y);
    }
    else if constexpr (std::is_same<GE_T, ge_short2>())
    {
        printf("{%d,%d}\n", v.x, v.y);
    }
    else if constexpr (std::is_same<GE_T, ge_float2>())
    {
        printf("{%f,%f}\n", v.x, v.y);
    }
}

template <typename GE_T>
void GE3_PRINT(GE_T v)
{
    if constexpr (std::is_same<GE_T, ge_int3>())
    {
        printf("{%d,%d,%d}\n", v.x, v.y, v.z);
    }
    else if constexpr (std::is_same<GE_T, ge_long3>())
    {
        printf("{%d,%d,%d}\n", v.x, v.y, v.z);
    }
    else if constexpr (std::is_same<GE_T, ge_short3>())
    {
        printf("{%d,%d,%d}\n", v.x, v.y, v.z);
    }
    else if constexpr (std::is_same<GE_T, ge_float3>())
    {
        printf("{%f,%f,%f}\n", v.x, v.y, v.z);
    }
}



}