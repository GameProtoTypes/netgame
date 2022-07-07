#pragma once
#include "cl_type_glue.h"


#define TO_Q16(x) (x << 16)//convert int decimal to Q16
#define MUL_Q16(x,y) (((x)*(y)) >> 16)//x*y where X is Q16 and Y is Q16 returns Q16. 
#define DIV_Q16(x,y) (((x<<16)/(y)))//x/y where X is Q16 and Y is Q16 returns Q16.  warning: ensure X has enough bits
#define WHOLE_Q16(x) (x >> 16) //whole part of a Q16 as an int.
#define WHOLE_ONLY_Q16(x) ((x >> 16) << 16) //x with no fractional part

#define MUL_PAD_Q16(x,y) ((((cl_long)(x))*((cl_long)(y))) >> 16)
#define DIV_PAD_Q16(x,y) (((((cl_long)x)<<16)/(y)))



static int perlin_hash_numbers[] = { 208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
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

cl_long FracMask(int Q)
{
    cl_ulong fractMask = 0;
    for (int i = 0; i < Q; i++)
        fractMask |= (1 << i);
    return fractMask;
}


cl_long DoubleToFixed(double floatingNumber, int Q)
{
    double dummy;
    double fraction = fract(floatingNumber, &dummy);
    cl_long fracBits = 0;

    for (int i = 0; i < Q; i++)
    {
        fraction *= 2;
        if (fraction >= 1.0)
        {
            fracBits |= (1 << ((Q - 1) - i));
            fraction -= (cl_long)fraction;
        }
    }
    cl_long wholeBits = ((cl_long)floatingNumber) * (1<<Q);
    return wholeBits | fracBits;
}
cl_long FloatToFixed(float floatingNumber, int Q)
{
    return DoubleToFixed((double)floatingNumber, Q);
}

float FixedToFloat(cl_long fixedPoint, int Q)
{
    cl_ulong signBitMask = (1 << (sizeof(cl_long) * 8 - 1));

    cl_ulong sign = fixedPoint & signBitMask;
    fixedPoint = fixedPoint & (~signBitMask);

    if (sign)
    {
        fixedPoint = ~fixedPoint;
        fixedPoint &= (~signBitMask);

        fixedPoint += (1 << Q);
    }

    float number = (float)((float)fixedPoint / (1 << Q));

    if (sign)
    {
        number *= -1;
    }

    return number;
}

void PrintQ16(cl_long fixed_Q16)
{
    printf("%f\n", FixedToFloat(fixed_Q16, 16));
}


// sqrt_i64 computes the squrare root of a 64bit integer and returns
// a 64bit integer value. It requires that v is positive.
cl_long sqrt_i64(cl_long v) {
    ulong b = ((ulong)1) << 62, q = 0, r = v;
    while (b > r)
        b >>= 2;
    while (b > 0) {
        ulong t = q + b;
        q >>= 1;
        if (r >= t) {
            r -= t;
            q += b;
        }
        b >>= 2;
    }
    return q;
}
cl_int length_v2_Q16(int2 v_Q16)
{
    long2 v_1 = (long2)(v_Q16.x,v_Q16.y);
    long2 inner_Q32 = v_1 * v_1;

    cl_long innerSum_Q32 = inner_Q32.x + inner_Q32.y;
    cl_long len_Q16 = sqrt_i64(innerSum_Q32);
    return (cl_int)len_Q16;
}
cl_int distance_v2_Q16(int2 v1_Q16, int2 v2_Q16)
{
    return length_v2_Q16(v2_Q16 - v1_Q16);
}

cl_int distance_s2_Q16(cl_int x1_Q16, cl_int y1_Q16, cl_int x2_Q16, cl_int y2_Q16)
{
    return distance_v2_Q16((int2)(x1_Q16, y1_Q16), (int2)(x2_Q16, y2_Q16));
}

void normalize_v2_Q16(int2* v_Q16, cl_int* len_Q16)
{
    *len_Q16 = length_v2_Q16(*v_Q16);
    long2 v = (((long2)((*v_Q16).x, (*v_Q16).y)) << 16) / (*len_Q16);
    *v_Q16 = (int2)(v.x, v.y);
}


void normalize_s2_Q16(cl_int* x_Q16, cl_int* y_Q16, cl_int* len_Q16)
{
    int2 v = (int2)(*x_Q16, *y_Q16);
    normalize_v2_Q16(&v, len_Q16);
    *x_Q16 = v.x;
    *y_Q16 = v.y;
}


void linear_interp_1D_Q16(cl_long x1_Q16, cl_long x2_Q16, cl_int perc_Q16, cl_int* out_Q16)
{
    *out_Q16 = x1_Q16 + MUL_Q16((x2_Q16 - x1_Q16), perc_Q16);
}
void cubic_interp_1D_Q16(cl_long x1_Q16, cl_long x2_Q16, cl_int perc_Q16, cl_int* out_Q16)
{
    cl_long p = (cl_long)perc_Q16;
    cl_long perc_cubic_Q16 = MUL_Q16(MUL_Q16(p, p), (TO_Q16(3) - MUL_Q16(TO_Q16(2), p)));
    linear_interp_1D_Q16(x1_Q16, x2_Q16, (cl_int)perc_cubic_Q16, out_Q16);
}



cl_int noise_256(cl_int x, cl_int y, cl_int seed)
{
    int tmp = perlin_hash_numbers[(y + seed) % 256];
    return perlin_hash_numbers[(tmp + x) % 256];
}

cl_int noise_2d_Q16(cl_int x_Q16, cl_int y_Q16, cl_int seed)
{
    cl_int x_int = WHOLE_Q16(x_Q16);
    cl_int y_int = WHOLE_Q16(y_Q16);
    
    cl_int x_frac_Q16 = x_Q16 - WHOLE_ONLY_Q16(x_Q16);
    cl_int y_frac_Q16 = y_Q16 - WHOLE_ONLY_Q16(y_Q16);
    
    
    
    cl_int s = noise_256(x_int, y_int, seed);
    cl_int t = noise_256(x_int + 1, y_int, seed);
    cl_int u = noise_256(x_int, y_int + 1, seed);
    cl_int v = noise_256(x_int + 1, y_int + 1, seed);
    cl_int low_Q16;
    cl_int high_Q16;
    cubic_interp_1D_Q16(TO_Q16(s), TO_Q16(t), x_frac_Q16, &low_Q16);
    cubic_interp_1D_Q16(TO_Q16(u), TO_Q16(v), x_frac_Q16, &high_Q16);
    cl_int result_Q16;
    cubic_interp_1D_Q16(low_Q16, high_Q16, y_frac_Q16, &result_Q16);
    return result_Q16;
}

cl_int perlin_2d_Q16(cl_int x_Q16, cl_int y_Q16, cl_int freq_Q16, cl_int depth,  cl_int seed)
{
    cl_long xa_Q16 = MUL_PAD_Q16(x_Q16, freq_Q16);
    cl_long ya_Q16 = MUL_PAD_Q16(y_Q16, freq_Q16);
    cl_long amp_Q16 = TO_Q16(1);
    cl_long fin_Q16 = 0;
    cl_long div_Q16 = 0;

    int i;
    for (i = 0; i < depth; i++)
    {
        div_Q16 += MUL_PAD_Q16(TO_Q16(256) , amp_Q16);
        fin_Q16 += MUL_PAD_Q16(noise_2d_Q16(xa_Q16, ya_Q16, seed) , amp_Q16);
        amp_Q16 = DIV_PAD_Q16(amp_Q16, TO_Q16(2));
        xa_Q16 = MUL_PAD_Q16(xa_Q16, TO_Q16(2));
        ya_Q16 = MUL_PAD_Q16(ya_Q16, TO_Q16(2));
    }

    return DIV_PAD_Q16(fin_Q16, div_Q16);
}






void fixedPointTests()
{
    int Q = 16;
    float floatingNumber = -16.1897f;
    printf("FloatingNumber: %f\n", floatingNumber);
    cl_long fixed_Q16 = FloatToFixed(floatingNumber, Q);

    floatingNumber = FixedToFloat(fixed_Q16, Q);

    printf("FloatingNumber(Check): %f\n", floatingNumber);

    //for (float i = 0; i < 1.0f; i+= 0.01f)
    //{
    //    cl_int interp_Q16 = 0;

    //    //linear_interp_1D_Q16(TO_Q16(100), TO_Q16(200), FloatToFixed(i, 16), &interp_Q16);

    //    cubic_interp_1D_Q16(TO_Q16(100), TO_Q16(200), FloatToFixed(i, 16), &interp_Q16);
    //    printf("%d\n", WHOLE_Q16(interp_Q16));
    //}
}