module;

#include <type_traits>
#include <numbers>
#include <gcem.hpp>
#include <glm/glm.hpp>

#ifdef _DEBUG
    #define GE_FIXEDPOINT_CHECK_OVERFLOW
    #define GE_FIXEDPOINT_CHECK_CAST
#endif

export module GE.Basic:FixedPoint;

import :Types;

export namespace GE {

template <typename Q_T = ge_int>
constexpr int GE_BIGGEST_Q(ge_float magnitude)
{
    ge_float maxMag = gcem::abs(magnitude);
    int WHOLE_BITS = (int)gcem::ceil( gcem::log2( gcem::floor(maxMag)));
    if(WHOLE_BITS == 0)
        WHOLE_BITS = 1;
    
    if(std::is_signed<Q_T>())
        WHOLE_BITS++;


    return sizeof(Q_T)*8 - WHOLE_BITS;
}


template <int Q = DEFAULT_Q, typename T = ge_int, typename Q_T = ge_int>
constexpr Q_T GE_TO_Q(const T& val)
{
    if constexpr (std::is_integral<T>::value)
        return static_cast<Q_T>(val << Q);
    else if constexpr (std::is_floating_point<T>::value)
        return static_cast<Q_T>(val * (1 << Q));
}


template <int Q = DEFAULT_Q, typename GE3_T = ge_int3, typename GE3_Q_T = ge_int3, typename GE_Q_T = ge_int>
constexpr GE3_Q_T GE3_TO_Q(const GE3_T& ge3)
{
    // if constexpr (std::is_integral<GE3_T::x>::value)
    //     return GE3_Q_T(
    //         ge3.x << Q, 
    //         ge3.y << Q,
    //         ge3.z << Q);
    // else if constexpr (std::is_floating_point<GE3_T::x>::value)
    //     return GE3_Q_T<Q, ge_float3, GE3_Q_T, GE_Q_T>(
    //         ge3.x * (1 << Q),
    //         ge3.y * (1 << Q),
    //         ge3.z * (1 << Q));
}
template <int Q = DEFAULT_Q, typename GE3_Q_T = ge_int3, typename GE_Q_T = ge_int>
constexpr GE3_Q_T GE3_TO_Q(const ge_float3& ge3)
{
        return GE3_Q_(
            ge3.x * (1 << Q),
            ge3.y * (1 << Q),
            ge3.z * (1 << Q));
}
template <int Q = DEFAULT_Q, typename GE3_Q_T = ge_int3, typename GE_Q_T = ge_int>
constexpr GE3_Q_T GE3_TO_Q(const ge_int3& ge3)
{
        return GE3_Q_T(
            ge3.x << Q, 
            ge3.y << Q,
            ge3.z << Q);
}

template <typename INTEGER_T>
constexpr void GE_CHECK_INT_MULTIPLY_OVERFLOW(const INTEGER_T& valA, const INTEGER_T& valB)
{
    INTEGER_T a = valA * valB;
    if (a != 0) {
        INTEGER_T d = a / valB;
        if (d != valA)
        {
            assert(0);
        }
    }
}

template <typename PADDED_INTEGER_T, typename RESULT_T>
constexpr void GE_CHECK_INT_CAST_NUMERIC(PADDED_INTEGER_T& value)
{
    if (PADDED_INTEGER_T(RESULT_T(value)) != value)
    {
        assert(0);
    }
}


template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename GE_Q_TA = ge_int, typename GE_Q_TB = ge_int>
constexpr GE_Q_TA GE_MUL_Q(const GE_Q_TA& valA, const GE_Q_TB& valB)
{
    typedef GETypeConversions<GE_Q_TA>::paddedBaseType PADDED_T;

#ifdef GE_FIXEDPOINT_CHECK_OVERFLOW
    PADDED_T valAP = valA;
    PADDED_T valBP = valB;
    GE_CHECK_INT_MULTIPLY_OVERFLOW<PADDED_T>(valAP, valBP);
#endif
    const ge_int shift = ((Q_A + Q_B) - Q_OUT);
    PADDED_T resPadded = (((PADDED_T)valA) * ((PADDED_T)valB)) >> shift;

#ifdef GE_FIXEDPOINT_CHECK_CAST
    GE_CHECK_INT_CAST_NUMERIC<PADDED_T, GE_Q_TA>(resPadded);
#endif
    return GE_Q_TA(resPadded);

}


template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename T = ge_int2>
constexpr T GE2_MUL_Q(const T& ge2A, const T& ge2B)
{
    typedef GETypeConversions<T>::paddedBaseType PADDED_T;

#ifdef GE_FIXEDPOINT_CHECK_OVERFLOW
    PADDED_T valAPX = ge2A.x;
    PADDED_T valBPX = ge2B.x;
    GE_CHECK_INT_MULTIPLY_OVERFLOW<PADDED_T>(valAPX, valBPX);

    PADDED_T valAPY = ge2A.y;
    PADDED_T valBPY = ge2B.y;
    GE_CHECK_INT_MULTIPLY_OVERFLOW<PADDED_T>(valAPY, valBPY);

#endif
    const ge_int shift = ((Q_A + Q_B) - Q_OUT);
    PADDED_T x = (( (PADDED_T)ge2A.x) * ((PADDED_T)ge2B.x )) >> shift;
    PADDED_T y = (( (PADDED_T)ge2A.y) * ((PADDED_T)ge2B.y )) >> shift;


#ifdef GE_FIXEDPOINT_CHECK_CAST
    GE_CHECK_INT_CAST_NUMERIC<PADDED_T, GETypeConversions<T>::baseType >(x);
    GE_CHECK_INT_CAST_NUMERIC<PADDED_T, GETypeConversions<T>::baseType >(y);
#endif

    return T(x, y);
}
template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_SCALAR = DEFAULT_Q, typename T = ge_int2,  typename SCALAR_T = ge_int>
constexpr T GE2_MUL_Q(const T& ge2A, const SCALAR_T& scalar)
{

    typedef GETypeConversions<T>::paddedBaseType PADDED_T;

#ifdef GE_FIXEDPOINT_CHECK_OVERFLOW
    PADDED_T valAPX = ge2A.x;
    PADDED_T valBPX = scalar;
    GE_CHECK_INT_MULTIPLY_OVERFLOW<PADDED_T>(valAPX, valBPX);

    PADDED_T valAPY = ge2A.y;
    PADDED_T valBPY = scalar;
    GE_CHECK_INT_MULTIPLY_OVERFLOW<PADDED_T>(valAPY, valBPY);

#endif


    const ge_int shift = ((Q_A + Q_SCALAR) - Q_OUT);

    PADDED_T x = (((PADDED_T)ge2A.x) * ((PADDED_T)scalar)) >> shift;
    PADDED_T y = (((PADDED_T)ge2A.y) * ((PADDED_T)scalar)) >> shift;

#ifdef GE_FIXEDPOINT_CHECK_CAST
    GE_CHECK_INT_CAST_NUMERIC<PADDED_T, GETypeConversions<T>::baseType>(x);
    GE_CHECK_INT_CAST_NUMERIC<PADDED_T, GETypeConversions<T>::baseType>(y);
#endif

    return T(x,y);

}


template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename T= ge_int3>
constexpr T GE3_MUL_Q(const T& ge3A, const T& ge3B)
{
    typedef GETypeConversions<T>::paddedBaseType PADDED_T;
    const ge_int shift = ((Q_A + Q_B) - Q_OUT);

#ifdef GE_FIXEDPOINT_CHECK_OVERFLOW
    PADDED_T valAPX = ge3A.x;
    PADDED_T valBPX = ge3B.x;
    GE_CHECK_INT_MULTIPLY_OVERFLOW<PADDED_T>(valAPX, valBPX);

    PADDED_T valAPY = ge3A.y;
    PADDED_T valBPY = ge3B.y;
    GE_CHECK_INT_MULTIPLY_OVERFLOW<PADDED_T>(valAPY, valBPY);

    PADDED_T valAPZ = ge3A.z;
    PADDED_T valBPZ = ge3B.z;
    GE_CHECK_INT_MULTIPLY_OVERFLOW<PADDED_T>(valAPZ, valBPZ);

#endif
    PADDED_T x = (((PADDED_T)ge3A.x) * ((PADDED_T)ge3B.x)) >> shift;
    PADDED_T y = (((PADDED_T)ge3A.y) * ((PADDED_T)ge3B.y)) >> shift;
    PADDED_T z = (((PADDED_T)ge3A.z) * ((PADDED_T)ge3B.z)) >> shift;


#ifdef GE_FIXEDPOINT_CHECK_CAST
    GE_CHECK_INT_CAST_NUMERIC<PADDED_T, GETypeConversions<T>::baseType>(x);
    GE_CHECK_INT_CAST_NUMERIC<PADDED_T, GETypeConversions<T>::baseType>(y);
    GE_CHECK_INT_CAST_NUMERIC<PADDED_T, GETypeConversions<T>::baseType>(z);
#endif

    return T( x,y,z);
}

template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_SCALAR = DEFAULT_Q, typename T= ge_int3,  typename SCALAR_T= ge_int>
constexpr T GE3_MUL_Q(const T& ge3A, const SCALAR_T& scalar)
{
    typedef GETypeConversions<T>::paddedBaseType PADDED_T;
    const ge_int shift = ((Q_A + Q_SCALAR) - Q_OUT);

    return T( (((PADDED_T)ge3A.x)*((PADDED_T)scalar)) >> shift,
              (((PADDED_T)ge3A.y)*((PADDED_T)scalar)) >> shift,
              (((PADDED_T)ge3A.z)*((PADDED_T)scalar)) >> shift);
}



template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename T = ge_int>
constexpr T GE_DIV_Q(const T& valA, const T& valB)
{
    typedef GETypeConversions<T>::paddedBaseType PADDED_T;

    return T(GE_SIGNED_SHIFT<PADDED_T,-(Q_OUT - Q_A)>(((((PADDED_T)valA)<<(Q_B))/(PADDED_T(valB)))) );
}

template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename T = ge_int2>
constexpr T GE2_DIV_Q(const T& ge2A, const T& ge2B)
{
    typedef GETypeConversions<T>::paddedBaseType PADDED_T;

    return T( GE_SIGNED_SHIFT<PADDED_T,-(Q_OUT - Q_A)>(((((PADDED_T)ge2A.x)<<(Q_B))/(PADDED_T(ge2B.x)))),
              GE_SIGNED_SHIFT<PADDED_T,-(Q_OUT - Q_A)>(((((PADDED_T)ge2A.y)<<(Q_B))/(PADDED_T(ge2B.y))))  );
}

template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename T = ge_int2, typename SCALAR_T = ge_int>
constexpr T GE2_DIV_Q(const T& ge2A, const SCALAR_T& scalar)
{
    typedef GETypeConversions<T>::paddedBaseType PADDED_T;

    return T( GE_SIGNED_SHIFT<PADDED_T,-(Q_OUT - Q_A)>(((((PADDED_T)ge2A.x)<<(Q_B))/(PADDED_T(scalar)))),
              GE_SIGNED_SHIFT<PADDED_T,-(Q_OUT - Q_A)>(((((PADDED_T)ge2A.y)<<(Q_B))/(PADDED_T(scalar))))  );
}

template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename T = ge_int3, typename SCALAR_T = ge_int>
constexpr T GE3_DIV_Q(const T& ge3A, const SCALAR_T& scalar)
{
    typedef GETypeConversions<T>::paddedBaseType PADDED_T;

    return T( GE_SIGNED_SHIFT<PADDED_T,-(Q_OUT - Q_A)>((((static_cast<PADDED_T>(ge3A.x))<<(Q_B))/(PADDED_T(scalar)))),
              GE_SIGNED_SHIFT<PADDED_T,-(Q_OUT - Q_A)>((((static_cast<PADDED_T>(ge3A.y))<<(Q_B))/(PADDED_T(scalar)))),
              GE_SIGNED_SHIFT<PADDED_T,-(Q_OUT - Q_A)>((((static_cast<PADDED_T>(ge3A.z))<<(Q_B))/(PADDED_T(scalar))))   );
}
template <int Q_OUT = DEFAULT_Q, int Q_A = DEFAULT_Q, int Q_B = DEFAULT_Q, typename T = ge_int3>
constexpr T GE3_DIV_Q(const T& ge3A, const T& ge3B)
{
    typedef GETypeConversions<T>::paddedBaseType PADDED_T;

    return T( GE_SIGNED_SHIFT<PADDED_T,-(Q_OUT - Q_A)>((((static_cast<PADDED_T>(ge3A.x))<<(Q_B))/(PADDED_T(ge3B.x)))),
              GE_SIGNED_SHIFT<PADDED_T,-(Q_OUT - Q_A)>((((static_cast<PADDED_T>(ge3A.y))<<(Q_B))/(PADDED_T(ge3B.y)))),
              GE_SIGNED_SHIFT<PADDED_T,-(Q_OUT - Q_A)>((((static_cast<PADDED_T>(ge3A.z))<<(Q_B))/(PADDED_T(ge3B.z))))   );
}


//whole part of a Q* as an int.
template <int Q = DEFAULT_Q, typename T= ge_int>
constexpr T GE_WHOLE_Q(const T& valA)
{
    return T(valA>>Q);
}

//whole part of a Q* as an int.
template <int Q = DEFAULT_Q, typename GE2_Q_T= ge_int2>
constexpr GE2_Q_T GE2_WHOLE_Q(const GE2_Q_T& ge2)
{
    return GE2_Q_T(ge2.x>>Q, ge2.y>>Q);
}

//whole part of a Q* as an int.
template <int Q = DEFAULT_Q, typename GE3_Q_T= ge_int3>
constexpr GE3_Q_T GE3_WHOLE_Q(const GE3_Q_T& ge3)
{
    return GE3_Q_T(ge3.x>>Q,ge3.y>>Q,ge3.z>>Q);
}



//x with 0 fractional part
template <int Q = DEFAULT_Q, typename T= ge_int>
constexpr T GE_WHOLE_ONLY_Q(const T& valA)
{
    return ((valA>>Q) << Q);
}

//x with 0 fractional part
template <int Q = DEFAULT_Q, typename GE2_Q_T= ge_int2>
constexpr GE2_Q_T GE2_WHOLE_ONLY_Q(const GE2_Q_T& ge2)
{
    return ((ge2.x>>Q) << Q, (ge2.y>>Q) << Q);
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

template <typename T = ge_int3>
ge_int GE3_ENTRY_COUNT(T ge3)
{
    int s = 0;
    if (ge3.x != 0)
        s++;
    if (ge3.y != 0)
        s++;
    if (ge3.z != 0)
        s++;

    return s;
}
template <typename T = ge_int3>
ge_ubyte GE3_SINGLE_ENTRY(T ge3)
{
    int s = 0;
    if (ge3.x != 0)
        s++;
    if (ge3.y != 0)
        s++;
    if (ge3.z != 0)
        s++;

    if (s == 1)
        return 1;
    else
        return 0;
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
    ge_long3 squared = GE3_MUL_Q<Q,Q,Q, ge_long3>(v_1, v_1);

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
    if(length == 0)
        return GE3_Q_T(0,0,0);
    GE3_Q_T normalized = GE3_DIV_Q<Q,Q,Q, GE3_Q_T, GE_LEN_Q_T>(ge3, length);
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
decltype(GE3_Q_T::x) GE3_DOT_PRODUCT_Q(GE3_Q_T ge3A, GE3_Q_T ge3B) {
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

// void PrintBits(int const size, void const* const ptr)
// {
//     unsigned char* b = (unsigned char*)ptr;
//     unsigned char byte;
//     int i, j;

//     for (i = size - 1; i >= 0; i--) {
//         for (j = 7; j >= 0; j--) {
//             byte = (b[i] >> j) & 1;
//             printf("%u", byte);
//         }
//     }
//     printf("\n");
// }

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


}