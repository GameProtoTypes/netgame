#include <type_traits>
#include <numbers>
#include <gcem.hpp>
#include <glm.hpp>


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

typedef ge_uint ge_offsetPtr;
typedef ge_uint2 ge_offsetPtr2;
typedef ge_uint3 ge_offsetPtr3;
typedef ge_ushort3 ge_offsetPtrShort3;

#define GE_OFFSET_NULL (0xFFFFFFFF)
#define GE_OFFSET_NULL_2D  ((offsetPtr2){0xFFFFFFFF , 0xFFFFFFFF})
#define GE_OFFSET_NULL_3D  ((offsetPtr3){0xFFFFFFFF , 0xFFFFFFFF,  0xFFFFFFFF})

#define GE_OFFSET_NULL_SHORT_3D  ((offsetPtrShort3){0xFFFF , 0xFFFF,  0xFFFF})

template <class T>
constexpr T GE_CLAMP(const T& ge, const T& min, const T& max)
{
    return glm::clamp<T>(ge,min,max);
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





template <typename Q_T>
void GE_PRINT(Q_T v)
{
    if constexpr (std::is_same<Q_T, ge_int>())
    {
        printf("{%d}\n", v);
    }
    else if constexpr (std::is_same<Q_T, ge_long>())
    {
        printf("{%d}\n", v);
    }
    else if constexpr (std::is_same<Q_T, ge_short>())
    {
        printf("{%d}\n", v);
    }
    else if constexpr (std::is_same<Q_T, ge_float>())
    {
        printf("{%f}\n", v);
    }
}

template <typename Q_T>
void GE2_PRINT(Q_T v)
{
    if constexpr (std::is_same<Q_T, ge_int2>())
    {
        printf("{%d,%d}\n", v.x, v.y);
    }
    else if constexpr (std::is_same<Q_T, ge_long2>())
    {
        printf("{%d,%d}\n", v.x, v.y);
    }
    else if constexpr (std::is_same<Q_T, ge_short2>())
    {
        printf("{%d,%d}\n", v.x, v.y);
    }
    else if constexpr (std::is_same<Q_T, ge_float2>())
    {
        printf("{%f,%f}\n", v.x, v.y);
    }
}

}