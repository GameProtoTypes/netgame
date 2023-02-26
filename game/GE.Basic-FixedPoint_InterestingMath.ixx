module;

#include <type_traits>
#include <numbers>
#include <gcem.hpp>
#include <glm.hpp>


export module GE.Basic:FixedPoint_InterestingMath;

import :Types;
import :FixedPoint;

export namespace GE {

//Interp from x1 to x2 with perc 
template<int Q = DEFAULT_Q, typename GE_Q_T>
constexpr GE_Q_T GE_LINEAR_INTERP_1D_Q(GE_Q_T x1, GE_Q_T x2, GE_Q_T perc)
{
   return x1 + GE_MUL_Q<Q>((x2 - x1), perc);
}

//Cubic Interp from x1 to x2 with perc
template<int Q = DEFAULT_Q, typename GE_Q_T>
constexpr GE_Q_T GE_CUBIC_INTERP_1D_Q(GE_Q_T x1, GE_Q_T x2, GE_Q_T perc)
{
    GE_Q_T p = perc;
    GE_Q_T perc_cubic = GE_MUL_Q<Q>(GE_MUL_Q<Q>(p, p), (GE_TO_Q<Q>(3) - GE_MUL_Q<Q>(GE_TO_Q<Q>(2), p)));
    return GE_LINEAR_INTERP_1D_Q(x1, x2, (ge_int)perc_cubic);
}


template <class GE2_T = ge_int2>
constexpr ge_int GE_FAST_NOISE_2D(GE2_T v, ge_int seed)
{
    int tmp = perlin_hash_numbers[(v.y + seed) % 256];
    return perlin_hash_numbers[(tmp + v.x) % 256];
}


template<int Q = DEFAULT_Q, typename GE2_Q_T = ge_int2>
ge_int GE2_NOISE_SMOOTH_Q(GE2_Q_T ge2, ge_int seed)
{
    GE2_Q_T v_int = GE2_WHOLE_Q<Q>(ge2);

    ge_int x_frac_Q16 = ge2.x - GE_TO_Q<Q>(v_int.x);
    ge_int y_frac_Q16 = ge2.y - GE_TO_Q<Q>(v_int.y);
    
    ge_int s = GE_FAST_NOISE_2D(GE2_Q_T(v_int.x,     v_int.y)    , seed);
    ge_int t = GE_FAST_NOISE_2D(GE2_Q_T(v_int.x + 1, v_int.y)    , seed);
    ge_int u = GE_FAST_NOISE_2D(GE2_Q_T(v_int.x,     v_int.y + 1), seed);
    ge_int v = GE_FAST_NOISE_2D(GE2_Q_T(v_int.x + 1, v_int.y + 1), seed);
    ge_int low_Q;
    ge_int high_Q;
    low_Q  = GE_CUBIC_INTERP_1D_Q<Q>(GE_TO_Q<Q>(s), GE_TO_Q<Q>(t), x_frac_Q16);
    high_Q = GE_CUBIC_INTERP_1D_Q<Q>(GE_TO_Q<Q>(u), GE_TO_Q<Q>(v), x_frac_Q16);

    return GE_CUBIC_INTERP_1D_Q(low_Q, high_Q, y_frac_Q16);
}

template<int Q = DEFAULT_Q, typename GE2_Q_T = ge_int2, typename GE_Q_T = ge_int>
GE_Q_T GE2_PERLIN_Q(GE2_Q_T ge2, GE_Q_T freq, GE_Q_T depth, GE_Q_T seed)
{
    GE_Q_T xa = GE_MUL_Q<Q>(ge2.x, freq);
    GE_Q_T ya = GE_MUL_Q<Q>(ge2.y, freq);
    GE_Q_T amp = GE_TO_Q<Q>(1);
    GE_Q_T fin = 0;
    GE_Q_T div = 0;

    ge_int i;
    for (i = 0; i < depth; i++)
    {
        div += GE_MUL_Q<Q>(GE_TO_Q<Q>(256) , amp);
        fin += GE_MUL_Q<Q>(GE2_NOISE_SMOOTH_Q(ge_int2(xa, ya), seed) , amp);
        amp = GE_DIV_Q<Q>(amp, GE_TO_Q<Q>(2));
        xa = GE_MUL_Q<Q>(xa, GE_TO_Q<Q>(2));
        ya = GE_MUL_Q<Q>(ya, GE_TO_Q<Q>(2));
    }

    return  GE_DIV_Q<Q>(fin, div);
}


}