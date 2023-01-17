#pragma once

#include <type_traits>


#define DEFAULT_Q (16)

typedef int16_t ge_short;
typedef uint16_t ge_ushort;
typedef int32_t ge_int;
typedef uint32_t ge_uint;
typedef int64_t ge_long;
typedef uint64_t ge_ulong;
typedef float ge_float;

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


template <int Q = DEFAULT_Q, typename T = ge_int, typename Q_T = ge_int>
constexpr Q_T GE_TO_Q(const T& val)
{
    if constexpr (std::is_integral<T>::value)
        return static_cast<Q_T>(val << Q);
    else if constexpr (std::is_floating_point<T>::value)
        return static_cast<Q_T>(val * (1 << Q));
}

template <int Q = DEFAULT_Q, typename T= ge_int, typename SafeType = ge_long>
constexpr T GE_MUL_Q(const T& valA, const T& valB)
{
    return T((((SafeType)valA)*((SafeType)valB)) >> Q);
}
template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename T= ge_int, typename SafeType = ge_long>
constexpr T GE_MUL_Q(const T& valA, const T& valB)
{
    return T((((SafeType)valA)*((SafeType)valB)) >> ((Q_A+Q_B)-Q_OUT));
}
template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename T= ge_int2, typename SafeType = ge_long>
constexpr T GE2_MUL_Q(const T& ge2A, const T& ge2B)
{
    return T( (((SafeType)ge2A.x)*((SafeType)ge2B.x)) >> ((Q_A+Q_B)-Q_OUT), 
              (((SafeType)ge2A.y)*((SafeType)ge2B.y)) >> ((Q_A+Q_B)-Q_OUT) );
}
template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_SCALAR = DEFAULT_Q, typename T= ge_int2,  typename SCALAR_T= ge_int, typename SafeType = ge_long>
constexpr T GE2_MUL_Q(const T& ge2A, const SCALAR_T& scalar)
{
    return T( (((SafeType)ge2A.x)*((SafeType)scalar)) >> ((Q_A+Q_SCALAR)-Q_OUT), 
              (((SafeType)ge2A.y)*((SafeType)scalar)) >> ((Q_A+Q_SCALAR)-Q_OUT) );
}


// template <int Q = DEFAULT_Q, typename T= ge_int, typename SafeType = ge_long>
// constexpr T GE_DIV_Q(const T& valA, const T& valB)
// {
//     return T((((SafeType)valA)<<Q)/((SafeType)valB));
// }

template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename T = ge_int, typename SafeType = ge_long>
constexpr T GE_DIV_Q(const T& valA, const T& valB)
{
    return T(GE_SIGNED_SHIFT<SafeType,-(Q_OUT - Q_A)>(((((SafeType)valA)<<(Q_B))/(SafeType(valB)))) );
}

template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename T = ge_int2, typename SafeType = ge_long>
constexpr T GE2_DIV_Q(const T& ge2A, const T& ge2B)
{
    return T( GE_SIGNED_SHIFT<SafeType,-(Q_OUT - Q_A)>(((((SafeType)ge2A.x)<<(Q_B))/(SafeType(ge2B.x)))),
              GE_SIGNED_SHIFT<SafeType,-(Q_OUT - Q_A)>(((((SafeType)ge2A.y)<<(Q_B))/(SafeType(ge2B.y))))  );
}

template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename T = ge_int2, typename SCALAR_T = ge_int, typename SafeType = ge_long>
constexpr T GE2_DIV_Q(const T& ge2A, const SCALAR_T& scalar)
{
    return T( GE_SIGNED_SHIFT<SafeType,-(Q_OUT - Q_A)>(((((SafeType)ge2A.x)<<(Q_B))/(SafeType(scalar)))),
              GE_SIGNED_SHIFT<SafeType,-(Q_OUT - Q_A)>(((((SafeType)ge2A.y)<<(Q_B))/(SafeType(scalar))))  );
}

template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename T = ge_int3, typename SCALAR_T = ge_int, typename SafeType = ge_long>
constexpr T GE3_DIV_Q(const T& ge3A, const SCALAR_T& scalar)
{
    return T( GE_SIGNED_SHIFT<SafeType,-(Q_OUT - Q_A)>(((((SafeType)ge3A.x)<<(Q_B))/(SafeType(scalar)))),
              GE_SIGNED_SHIFT<SafeType,-(Q_OUT - Q_A)>(((((SafeType)ge3A.y)<<(Q_B))/(SafeType(scalar)))),
              GE_SIGNED_SHIFT<SafeType,-(Q_OUT - Q_A)>(((((SafeType)ge3A.z)<<(Q_B))/(SafeType(scalar))))   );
}


//whole part of a Q* as an int.
template <int Q = DEFAULT_Q, typename T= ge_int>
constexpr T GE_WHOLE_Q(const T& valA)
{
    return (valA>>Q);
}
//x with 0 fractional part
template <int Q = DEFAULT_Q, typename T= ge_int>
constexpr T GE_WHOLE_ONLY_Q(const T& valA)
{
    return ((valA>>Q) << Q);
}


// GE_SQRT_LONG computes the squrare root of a 64bit integer and returns
// a 64bit integer value. It requires that v is positive.
constexpr ge_ulong GE_SQRT_ULONG(const ge_ulong& val) {
    ge_ulong b = ((ge_ulong)1) << 62, q = 0, r = val;
    while (b > r)
        b >>= 2;
    while (b > 0) {
        ge_ulong t = q + b;
        q >>= 1;
        if (r >= t) {
            r -= t;
            q += b;
        }
        b >>= 2;
    }
    return q;
}



//positive numbers returns TO_Q16(1) 
//negative numbers returns TO_Q16(-1)
//0 returns TO_Q16(1) 
template <int Q = DEFAULT_Q, typename GE_Q_T= ge_int>
GE_Q_T GE_SIGN_MAG_Q(const GE_Q_T& val)
{
    return ((((((val) >> (sizeof(GE_Q_T)*8-1)) & 0x1)*-2)+1) << Q);
}

template <int Q = DEFAULT_Q, typename GE2_Q_T= ge_int2, typename T= ge_int>
T GE2_LENGTH_Q(const GE2_Q_T& ge2)
{
    ge_long2 v_1 = ge_long2(ge2.x, ge2.y);
    ge_long2 squared = GE2_MUL_Q<Q,Q,Q, ge_long2>(v_1 , v_1);

    ge_long innerSum = squared.x + squared.y;
    ge_long len = GE_SQRT_ULONG(innerSum) << (Q/2);
    return (T)len;
}

template <int Q = DEFAULT_Q, typename GE3_Q_T= ge_int3, typename T = ge_int>
T GE3_LENGTH_Q(const GE3_Q_T& ge3)
{
    ge_long3 v_1 = ge_long3(ge3.x, ge3.y, ge3.z);
    ge_long3 squared = GE3_MUL_Q<Q, ge_long3>(v_1, v_1, v_1);

    ge_long innerSum = squared.x + squared.y + squared.z;
    ge_long len = GE_SQRT_ULONG(innerSum) << (Q/2);
    return (T)len;
}


template <int Q = DEFAULT_Q, typename GE2_Q_T = ge_int2, typename T = ge_int>
T GE2_DISTANCE_Q(const GE2_Q_T& a, const GE2_Q_T& b)
{
    return GE2_LENGTH_Q<Q,GE2_Q_T,T>(GE2_SUB(a,b));
}
template <int Q = DEFAULT_Q, typename GE3_Q_T= ge_int3, typename T = ge_int>
T GE3_DISTANCE_Q(const GE3_Q_T& a, const GE3_Q_T& b)
{
    return GE3_LENGTH_Q<Q,GE3_Q_T,T>(GE3_SUB(a,b));
}

template <int Q = DEFAULT_Q, typename GE2_Q_T = ge_int2, typename GE_LEN_Q_T = ge_int>
GE2_Q_T GE2_NORMALIZED_Q(const GE2_Q_T& ge2, GE_LEN_Q_T& length)
{
    length = GE2_LENGTH_Q<Q, GE2_Q_T, GE_LEN_Q_T>(ge2);
    GE2_Q_T normalized = GE2_DIV_Q<Q,Q,Q, GE2_Q_T>(ge2, length);
    return normalized;
}

template <int Q = DEFAULT_Q, typename GE3_Q_T = ge_int3, typename GE_LEN_Q_T = ge_int>
GE3_Q_T GE3_NORMALIZED_Q(const GE3_Q_T& ge3, GE_LEN_Q_T& length)
{
    length = GE3_LENGTH_Q<Q, GE3_Q_T, GE_LEN_Q_T>(ge3);
    GE3_Q_T normalized = GE3_DIV_Q<Q,Q,Q, GE3_Q_T>(GE3_ADD, length);
    return normalized;
}


template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename GE3_Q_T = ge_int3>
GE3_Q_T GE3_CROSS_PRODUCT_Q(GE3_Q_T ge3A, GE3_Q_T ge3B) 
{
    ge_int x = GE_MUL_Q<Q_OUT,Q_A, Q_B>(ge3A.y, ge3B.z) - GE_MUL_Q<Q_OUT,Q_A, Q_B>(ge3A.z, ge3B.y);
    ge_int y = GE_MUL_Q<Q_OUT,Q_A, Q_B>(-ge3A.x, ge3B.z) + GE_MUL_Q<Q_OUT,Q_A, Q_B>(ge3A.z, ge3B.x);
    ge_int z = GE_MUL_Q<Q_OUT,Q_A, Q_B>(ge3A.x, ge3B.y) - GE_MUL_Q<Q_OUT,Q_A, Q_B>(ge3A.y, ge3B.x);

    return GE3_Q_T{x, y, z };
}



template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename Q_T = ge_int, typename GE2_Q_T = ge_int2>
Q_T GE2_DOT_PRODUCT_Q(GE2_Q_T ge2A, GE2_Q_T ge2B) {
    return  GE_MUL_Q<Q_OUT,Q_A, Q_B>(ge2A.x, ge2B.x) +
            GE_MUL_Q<Q_OUT,Q_A, Q_B>(ge2A.y, ge2B.y);
}


template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename GE3_Q_T = ge_int3>
GE3_Q_T GE3_DOT_PRODUCT_Q(GE3_Q_T ge3A, GE3_Q_T ge3B) {
    return  GE_MUL_Q<Q_OUT,Q_A, Q_B>(ge3A.x, ge3B.x) +
            GE_MUL_Q<Q_OUT,Q_A, Q_B>(ge3A.y, ge3B.y) + 
            GE_MUL_Q<Q_OUT,Q_A, Q_B>(ge3A.z, ge3B.z);
}





//project a onto b direction, also return the scalar.  b is also normalized.
template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename GE2_Q_T = ge_int2>
GE2_Q_T GE2_PROJECTED_Q(const GE2_Q_T& ge2A, GE2_Q_T& ge2B, ge_int& out_scalar)
{
    ge_int len;
    ge2B = GE2_NORMALIZED_Q(ge2B, len);

    out_scalar = GE2_DOT_PRODUCT_Q<Q_OUT, Q_A, Q_B, ge_int, GE2_Q_T>(ge2A, ge2B);

    return GE2_MUL_Q(ge2B, out_scalar);
}





//v rotated about axis 90 degrees - axis does not need to be normalized.
template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename GE3_Q_T = ge_int3>
inline GE3_Q_T GE3_ROTATE_ABOUT_AXIS_POS90_Q(GE3_Q_T v, GE3_Q_T axis)
{
    GE3_Q_T a = v;
    GE3_Q_T b = axis;

    int a_dot_b = GE3_DOT_PRODUCT_Q<Q_OUT, Q_A, Q_B>(a, b);
    int b_dot_b = GE3_DOT_PRODUCT_Q<Q_OUT, Q_A, Q_B>(b, b);

    int s = GE_DIV_Q<Q_OUT, Q_OUT, Q_OUT>(a_dot_b, b_dot_b);
    GE3_Q_T a_par_b;
    a_par_b.x = GE_MUL_Q<Q_OUT, Q_OUT, Q_OUT>(s, b.x);
    a_par_b.y = GE_MUL_Q<Q_OUT, Q_OUT, Q_OUT>(s, b.y);
    a_par_b.z = GE_MUL_Q<Q_OUT, Q_OUT, Q_OUT>(s, b.z);

    GE3_Q_T a_orth_b = GE3_SUB(a, a_par_b);
    ge_int a_orth_b_mag = GE3_LENGTH_Q<Q_OUT>(a_orth_b);

    GE3_Q_T w = GE3_CROSS_PRODUCT_Q<Q_OUT, Q_B, Q_OUT>(b, a_orth_b);
    ge_int w_mag;
    GE3_Q_T w_norm = GE3_NORMALIZED_Q<Q_OUT,GE3_Q_T>(w, w_mag);

    GE3_Q_T v_rot;
    v_rot.x = GE_MUL_Q<Q_OUT,Q_OUT,Q_OUT>(a_orth_b_mag, w_norm.x) + a_par_b.x;
    v_rot.y = GE_MUL_Q<Q_OUT,Q_OUT,Q_OUT>(a_orth_b_mag, w_norm.y) + a_par_b.y;
    v_rot.z = GE_MUL_Q<Q_OUT,Q_OUT,Q_OUT>(a_orth_b_mag, w_norm.z) + a_par_b.z;

    return v_rot;
}






template <int Q = DEFAULT_Q, typename Q_T = ge_int, typename FLOAT_T = ge_float>
constexpr FLOAT_T GE_Q_TO_FLOAT(const Q_T& val)
{
    Q_T signBitMask = (0x1 << (sizeof(Q_T) * 8 - 1));
    Q_T sign = (val >> (sizeof(Q_T)*8 - 1)) & 0x1;
    Q_T bits = val;
    if (sign)
    {
        bits = ~bits;
        bits &= (~signBitMask);
    }
    FLOAT_T number = (FLOAT_T)((FLOAT_T)bits / (1 << Q));
    if (sign)
    {
        number *= -1;
    }
    return number;
}

void PrintBits(int const size, void const* const ptr)
{
    unsigned char* b = (unsigned char*)ptr;
    unsigned char byte;
    int i, j;

    for (i = size - 1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    printf("\n");
}

template <int Q = DEFAULT_Q, typename Q_T = ge_int>
void GE_PRINT_Q(Q_T val)
{
    printf("%f\n", GE_Q_TO_FLOAT<Q, Q_T, float>(val));
}

template <int Q = DEFAULT_Q, typename GE2_Q_T = ge_int2, typename GE_Q_T = ge_int>
void GE2_PRINT_Q(GE2_Q_T ge2)
{
    printf("%f,%f\n", GE_Q_TO_FLOAT<Q, GE_Q_T, float>(ge2.x),
                      GE_Q_TO_FLOAT<Q, GE_Q_T, float>(ge2.y));
}

template <int Q = DEFAULT_Q, typename GE3_Q_T = ge_int3, typename GE_Q_T = ge_int>
void GE3_PRINT_Q(GE3_Q_T ge3)
{
    printf("%f,%f,%f\n", GE_Q_TO_FLOAT<Q, GE_Q_T, float>(ge3.x),
                      GE_Q_TO_FLOAT<Q, GE_Q_T, float>(ge3.y),
                      GE_Q_TO_FLOAT<Q, GE_Q_T, float>(ge3.z));
}






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

