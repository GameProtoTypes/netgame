#pragma once
#include "cl_type_glue.h"
#include "gpuvectortypes.h"

#ifndef INT_MAX
#define INT_MAX (2147483647)
#endif

#define GE_TO_CL_INT2(ge)((cl_int2)(ge.x,ge.y))
#define GE_TO_CL_INT3(ge)((cl_int3)(ge.x,ge.y,ge.z))
#define GE_TO_CL_INT4(ge)((cl_int4)(ge.x,ge.y,ge.z,ge.w))



#define TO_Q16(x) ((x)<<16)//convert int decimal to Q16
#define MUL_Q16(x,y) (((x)*(y)) >> 16)//x*y where X is Q16 and Y is Q16 returns Q16. 
#define DIV_Q16(x,y) (((x<<16)/(y)))//x/y where X is Q16 and Y is Q16 returns Q16.  warning: ensure X has enough bits
#define WHOLE_Q16(x) (x >> 16) //whole part of a Q16 as an int.
#define WHOLE_ONLY_Q16(x) ((x >> 16) << 16) //x with 0 fractional part

#define MUL_PAD_Q16(x,y) ((((cl_long)(x))*((cl_long)(y))) >> 16)
#define MUL_PAD_V2_Q16(x,y) ((((cl_long2)(x))*((cl_long2)(y))) >> 16)


#define DIV_PAD_Q16(x,y) ((((cl_long)(x))<<16)/((cl_long)y))

#define MUL_2D_COMP_Q16(a,b) ((cl_int2)( ((a.x)*(b.x)) >> 16 , ((a.y)*(b.y)) >> 16 ))
#define MUL_PAD_2D_COMP_Q16(a,b) ((cl_int2)( (((cl_long)(a.x))*((cl_long)(b.x))) >> 16 , (((cl_long)(a.y))*((cl_long)(b.y))) >> 16 ))


#define IS_ZERO_V2(a) ((a.x==0) && (a.y==0))

#define GE_INT3_TO_Q16(ge)(ge<<16)

#define GE_INT3_WHOLE_Q16(ge)(ge>>16)
#define GE_INT3_WHOLE_ONLY_Q16(ge)((ge>>16)<<16)






struct QMP32
{
    cl_int number;
    cl_char q; 
} typedef QMP32;

struct QMP64
{
    cl_long number;
    cl_char q;
} typedef QMP64;


struct QMP32_2
{
    QMP32 x;
    QMP32 y;
} typedef QMP32_2;



inline QMP32 Q16_TO_QMP32(cl_int number_Q16)
{
    QMP32 qmp;
    qmp.number = number_Q16;
    qmp.q = 16;
    return qmp;
}


inline void SHIFT_QMP32(QMP32* qmp, int shift)
{
    if (shift > 0)
    {
        qmp->number = qmp->number >> shift;
    }
    else if (shift < 0)
    {
        qmp->number = qmp->number << -shift;
    }
    qmp->q -= shift;
}
inline void SHIFT_QMP64(QMP64* qmp, int shift)
{
    if (shift > 0)
    {
        qmp->number = qmp->number >> shift;
    }
    else if (shift < 0)
    {
        qmp->number = qmp->number << -shift;
    }
    qmp->q -= shift;
}




inline void NORMALIZE_QMP32(QMP32* qmp)
{
    cl_int comparisonNumber = abs(qmp->number);

    int s = 31-clz(comparisonNumber);

    if(s>=qmp->q)
        SHIFT_QMP32(qmp, -(30 - s));


}
inline void NORMALIZE_QMP64(QMP64* qmp)
{
    cl_int comparisonNumber = abs(qmp->number);

    int s = 62 - clz(comparisonNumber);

    if (s >= qmp->q)
        SHIFT_QMP64(qmp, -(62 - s));
}

inline void MATCH_QMP32(QMP32* qmpA, QMP32* qmpB, int offset)
{
    int offsetA = offset;
    int offsetB = offset;
    if (qmpA->q > qmpB->q)
    {
        offsetA += (qmpA->q - qmpB->q);
    }
    else if (qmpA->q < qmpB->q)
    {
        offsetB += (qmpB->q - qmpA->q);
    }
    SHIFT_QMP32(qmpA,  offsetA);
    SHIFT_QMP32(qmpB,  offsetB);
}
inline void MATCH_QMP64(QMP64* qmpA, QMP64* qmpB, int offset)
{
    if (qmpA->q > qmpB->q)
    {
        SHIFT_QMP64(qmpA, (qmpA->q - qmpB->q) + offset);
    }
    else if (qmpA->q < qmpB->q)
    {
        SHIFT_QMP64(qmpB, (qmpB->q - qmpA->q) + offset);
    }
}










//positive numbers returns TO_Q16(1) 
//negative numbers returns TO_Q16(-1)
//0 returns TO_Q16(1) 
#define SIGN_MAG_Q15_16(x) ((((((x) >> 31) & 0x00000001)*-2)+1) << 16)

constant static int perlin_hash_numbers[] = { 208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
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
    return (round(floatingNumber * (1 << Q)));
}
cl_long FloatToFixed(float floatingNumber, int Q)
{
    return DoubleToFixed((double)floatingNumber, Q);
}

QMP32 FloatToQMP32(float floatingNumber)
{
    int whole = floatingNumber;
    //determine q;
    int q = 30;
    int bits = whole;

    int signBitMask = (1 << (sizeof(int) * 8 - 1));
    int sign = bits & signBitMask;

    if (sign)
    {
        bits = ~bits+1;
    }
    for (int s = 30; s >= 0; s--)
    {
        if ((bits & ((1 << s))) != 0)
        {
            q = 30 - s;
            break;
        }
    }
    QMP32 res;
    res.number = FloatToFixed(floatingNumber, q);
    res.q = q;
    NORMALIZE_QMP32(&res);
    return res;
}

float FixedToFloat(cl_long fixedPoint, int Q)
{
    cl_ulong bits = 0;
    bits |= fixedPoint;
    cl_long number1 = 1;
    cl_ulong signBitMask = (number1 << (sizeof(cl_long) * 8 - 1));

    cl_ulong sign = bits & signBitMask;

    if (sign)
    {
        bits = ~bits;
        bits &= (~signBitMask);
    }

    double number = (double)((double)bits / (1 << Q));

    if (sign)
    {
        number *= -1;
    }

    return (float)number;
}
float FIXED2FLTQ16(cl_long fixedPoint)
{
    return FixedToFloat(fixedPoint, 16);
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


void PrintQ16(cl_long fixed_Q16)
{
    printf("%f\n", FixedToFloat(fixed_Q16, 16));
}

void PrintQMP32(QMP32 qmp)
{
    printf("%f (Q%d)\n", FixedToFloat(qmp.number, qmp.q), qmp.q);
}
void PrintQMP64(QMP64 qmp)
{
    printf("%f (Q%d)\n", FixedToFloat(qmp.number, qmp.q), qmp.q);
}



void Print_GE_INT3_Q16(ge_int3 fixed_Q16)
{
    printf("{%f,%f,%f}\n", FixedToFloat(fixed_Q16.x, 16), FixedToFloat(fixed_Q16.y, 16), FixedToFloat(fixed_Q16.z, 16));
}
void Print_GE_INT2_Q16(ge_int2 fixed_Q16)
{
    printf("{%f,%f}\n", FixedToFloat(fixed_Q16.x, 16), FixedToFloat(fixed_Q16.y, 16));
}


inline QMP32 QMP64_TO_QMP32(QMP64 qmp)
{
    cl_long number1 = 1;
    cl_long signBitMask = (number1 << (sizeof(cl_long) * 8 - 1));

    cl_long sign = qmp.number & signBitMask;
    cl_long comparisonNumber = qmp.number;
    if (sign)
    {
        comparisonNumber = ~qmp.number + 1;
    }



    QMP32 res;
    cl_long someLong = 1;
    for (int s = 62; s >= 0; s--)
    {
        if ((comparisonNumber & ((someLong << s))) != 0)
        {
            if (s > 30)
            {
                //bits need moved to the right
                int minmove = s - 30;
                
                if (minmove > qmp.q)
                {
                    printf("QMP64_TO_QMP32 overflow!\n");
                    minmove = qmp.q;
                }

                //printf("minmove: %d\n", minmove);
                SHIFT_QMP64(&qmp, minmove);
                break;
            }

        }
    }
    res.number = qmp.number;
    res.q = qmp.q;

    return res;
}
inline QMP64 QMP32_TO_QMP64(QMP32 qmp)
{
    QMP64 res;
    res.number = qmp.number;
    res.q = qmp.q;
    return res;
}

inline QMP32 ADD_QMP32(QMP32 qmpA, QMP32 qmpB)
{
    MATCH_QMP32(&qmpA, &qmpB,1);//match Q point and move right by one in case of adding overflow.

    qmpA.number = qmpA.number + qmpB.number;
    qmpA.q = qmpA.q;

    NORMALIZE_QMP32(&qmpA);
    return qmpA;
}
inline QMP32 MUL_QMP32(QMP32 qmpA, QMP32 qmpB)
{
    QMP64 r;

    r.number = (cl_long)qmpA.number * (cl_long)qmpB.number;
    r.q = qmpA.q + qmpB.q;

    QMP32 result = QMP64_TO_QMP32(r);

    NORMALIZE_QMP32(&result);
    return result;
}
inline QMP32 DIV_QMP32(QMP32 qmpA, QMP32 qmpB)
{

    QMP64 bigA = QMP32_TO_QMP64(qmpA);

    SHIFT_QMP64(&bigA, -qmpB.q);

    bigA.number = (cl_long)bigA.number / (cl_long)qmpB.number;
    bigA.q = qmpA.q;

    QMP32 result = QMP64_TO_QMP32(bigA);
    NORMALIZE_QMP32(&result);
    return result;
}

//Component Wise Division and Multiplication
ge_int2 DIV_v2_Q16(ge_int2 a_Q16, ge_int2 b_Q16)
{
    ge_int2 result;
    result.x = DIV_PAD_Q16(a_Q16.x, b_Q16.x);
    result.y = DIV_PAD_Q16(a_Q16.y, b_Q16.y);

    return result;
}
ge_int3 DIV_v3_Q16(ge_int3 a_Q16, ge_int3 b_Q16)
{
    ge_int3 result;
    result.x = DIV_PAD_Q16(a_Q16.x, b_Q16.x);
    result.y = DIV_PAD_Q16(a_Q16.y, b_Q16.y);
    result.z = DIV_PAD_Q16(a_Q16.z, b_Q16.z);

    return result;
}
ge_int2 MUL_v2_Q16(ge_int2 a_Q16, ge_int2 b_Q16)
{
    ge_int2 result;
    result.x = MUL_PAD_Q16(a_Q16.x, b_Q16.x);
    result.y = MUL_PAD_Q16(a_Q16.y, b_Q16.y);

    return result;
}
ge_int3 GE_INT3_MUL_Q16(ge_int3 a_Q16, ge_int3 b_Q16)
{
    ge_int3 result;

    result.x = MUL_PAD_Q16(a_Q16.x, b_Q16.x);
    result.y = MUL_PAD_Q16(a_Q16.y, b_Q16.y);
    result.z = MUL_PAD_Q16(a_Q16.z, b_Q16.z);

    return result;
}

ge_int3 GE_INT3_SCALAR_MUL_Q16(int scalar_Q16, ge_int3 a_Q16)
{
    ge_int3 result;

    result.x = MUL_PAD_Q16(scalar_Q16, a_Q16.x);
    result.y = MUL_PAD_Q16(scalar_Q16, a_Q16.y);
    result.z = MUL_PAD_Q16(scalar_Q16, a_Q16.z);

    return result;
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
cl_int cl_length_v2_Q16(int2 v_Q16)
{
    long2 v_1 = (long2)(v_Q16.x,v_Q16.y);
    long2 inner_Q32 = v_1 * v_1;

    cl_long innerSum_Q32 = inner_Q32.x + inner_Q32.y;
    cl_long len_Q16 = sqrt_i64(innerSum_Q32);
    return (cl_int)len_Q16;
}

cl_int cl_length_v3_Q16(int3 v_Q16)
{
    long3 v_1 = (long3)(v_Q16.x, v_Q16.y, v_Q16.z);
    long3 inner_Q32 = v_1 * v_1;

    cl_long innerSum_Q32 = inner_Q32.x + inner_Q32.y + inner_Q32.z;
    cl_long len_Q16 = sqrt_i64(innerSum_Q32);
    return (cl_int)len_Q16;
}
cl_int ge_length_v3_Q16(ge_int3 v_Q16)
{
    long3 v_1 = (long3)(v_Q16.x, v_Q16.y, v_Q16.z);
    long3 inner_Q32 = v_1 * v_1;

    cl_long innerSum_Q32 = inner_Q32.x + inner_Q32.y + inner_Q32.z;
    cl_long len_Q16 = sqrt_i64(innerSum_Q32);
    return (cl_int)len_Q16;
}

ge_int3 GE_INT3_NORMALIZE_Q16(ge_int3 v_Q16, int* mag)
{
    *mag = ge_length_v3_Q16(v_Q16);
    return (ge_int3) { DIV_PAD_Q16( v_Q16.x, *mag), DIV_PAD_Q16(v_Q16.y, *mag)
    , DIV_PAD_Q16(v_Q16.z, *mag)};
}


cl_int cl_distance_v2_Q16(int2 v1_Q16, int2 v2_Q16)
{
    return cl_length_v2_Q16(v2_Q16 - v1_Q16);
}

cl_int cl_distance_s2_Q16(cl_int x1_Q16, cl_int y1_Q16, cl_int x2_Q16, cl_int y2_Q16)
{
    return cl_distance_v2_Q16((int2)(x1_Q16, y1_Q16), (int2)(x2_Q16, y2_Q16));
}

void cl_normalize_v2_Q16(int2* v_Q16, cl_int* len_Q16)
{
    *len_Q16 = cl_length_v2_Q16(*v_Q16);
    long2 v = (((long2)((*v_Q16).x, (*v_Q16).y)) << 16) / (*len_Q16);
    *v_Q16 = (int2)(v.x, v.y);
}
void cl_normalize_v3_Q16(int3* v_Q16, cl_int* len_Q16)
{
    *len_Q16 = cl_length_v3_Q16(*v_Q16);
    long3 v = (((long3)((*v_Q16).x, (*v_Q16).y, (*v_Q16).z)) << 16) / (*len_Q16);
    *v_Q16 = (int3)(v.x, v.y, v.z);
}
void ge_normalize_v3_Q16(ge_int3* v_Q16, cl_int* len_Q16)
{
    *len_Q16 = ge_length_v3_Q16(*v_Q16);
    long3 v = (((long3)((*v_Q16).x, (*v_Q16).y, (*v_Q16).z)) << 16) / (*len_Q16);
    *v_Q16 = (ge_int3){ v.x, v.y, v.z };
}

void cl_normalize_s2_Q16(cl_int* x_Q16, cl_int* y_Q16, cl_int* len_Q16)
{
    int2 v = (int2)(*x_Q16, *y_Q16);
    cl_normalize_v2_Q16(&v, len_Q16);
    *x_Q16 = v.x;
    *y_Q16 = v.y;
}

void cl_dot_product_2D_Q16(cl_int2 a_Q16, cl_int2 b_Q16, cl_int* out_Q16)
{
    *out_Q16 = MUL_PAD_Q16(a_Q16.x, b_Q16.x) + MUL_PAD_Q16(a_Q16.y, b_Q16.y);
}

void ge_dot_product_3D_Q16(ge_int3 a_Q16, ge_int3 b_Q16, cl_int* out_Q16)
{
    *out_Q16 = MUL_PAD_Q16(a_Q16.x, b_Q16.x) + MUL_PAD_Q16(a_Q16.y, b_Q16.y) + MUL_PAD_Q16(a_Q16.z, b_Q16.z);
}

//project a onto b direction, also return the scalar.  b is also normalized.
void cl_project_2D_Q16(cl_int2* a_Q16, cl_int2* b_Q16, cl_int* out_scalar)
{
    cl_int len;
    cl_normalize_v2_Q16(b_Q16, &len);


    cl_dot_product_2D_Q16(*a_Q16, *b_Q16, out_scalar);

    (*a_Q16).x = MUL_PAD_Q16((* b_Q16).x, *out_scalar);
    (*a_Q16).y = MUL_PAD_Q16((* b_Q16).y, *out_scalar);
}



void cl_linear_interp_1D_Q16(cl_long x1_Q16, cl_long x2_Q16, cl_int perc_Q16, cl_int* out_Q16)
{
    *out_Q16 = x1_Q16 + MUL_Q16((x2_Q16 - x1_Q16), perc_Q16);
}
void cl_cubic_interp_1D_Q16(cl_long x1_Q16, cl_long x2_Q16, cl_int perc_Q16, cl_int* out_Q16)
{
    cl_long p = (cl_long)perc_Q16;
    cl_long perc_cubic_Q16 = MUL_Q16(MUL_Q16(p, p), (TO_Q16(3) - MUL_Q16(TO_Q16(2), p)));
    cl_linear_interp_1D_Q16(x1_Q16, x2_Q16, (cl_int)perc_cubic_Q16, out_Q16);
}



cl_int cl_noise_2d_256(cl_int x, cl_int y, cl_int seed)
{
    int tmp = perlin_hash_numbers[(y + seed) % 256];
    return perlin_hash_numbers[(tmp + x) % 256];
}

cl_int cl_noise_2d_Q16(cl_int x_Q16, cl_int y_Q16, cl_int seed)
{
    cl_int x_int = WHOLE_Q16(x_Q16);
    cl_int y_int = WHOLE_Q16(y_Q16);
    
    cl_int x_frac_Q16 = x_Q16 - WHOLE_ONLY_Q16(x_Q16);
    cl_int y_frac_Q16 = y_Q16 - WHOLE_ONLY_Q16(y_Q16);
    
    
    
    cl_int s = cl_noise_2d_256(x_int, y_int, seed);
    cl_int t = cl_noise_2d_256(x_int + 1, y_int, seed);
    cl_int u = cl_noise_2d_256(x_int, y_int + 1, seed);
    cl_int v = cl_noise_2d_256(x_int + 1, y_int + 1, seed);
    cl_int low_Q16;
    cl_int high_Q16;
    cl_cubic_interp_1D_Q16(TO_Q16(s), TO_Q16(t), x_frac_Q16, &low_Q16);
    cl_cubic_interp_1D_Q16(TO_Q16(u), TO_Q16(v), x_frac_Q16, &high_Q16);
    cl_int result_Q16;
    cl_cubic_interp_1D_Q16(low_Q16, high_Q16, y_frac_Q16, &result_Q16);
    return result_Q16;
}
cl_int cl_noise_3d_Q16(cl_int x_Q16, cl_int y_Q16, cl_int z_Q16, cl_int seed)
{
    //stub
    return 0;
}


cl_int cl_perlin_2d_Q16(cl_int x_Q16, cl_int y_Q16, cl_int freq_Q16, cl_int depth,  cl_int seed)
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
        fin_Q16 += MUL_PAD_Q16(cl_noise_2d_Q16(xa_Q16, ya_Q16, seed) , amp_Q16);
        amp_Q16 = DIV_PAD_Q16(amp_Q16, TO_Q16(2));
        xa_Q16 = MUL_PAD_Q16(xa_Q16, TO_Q16(2));
        ya_Q16 = MUL_PAD_Q16(ya_Q16, TO_Q16(2));
    }

    return DIV_PAD_Q16(fin_Q16, div_Q16);
}

cl_int cl_perlin_3d_Q16(cl_int x_Q16, cl_int y_Q16, cl_int z_Q16, cl_int freq_Q16, cl_int depth, cl_int seed)
{
    cl_long xa_Q16 = MUL_PAD_Q16(x_Q16, freq_Q16);
    cl_long ya_Q16 = MUL_PAD_Q16(y_Q16, freq_Q16);
    cl_long za_Q16 = MUL_PAD_Q16(z_Q16, freq_Q16);
    cl_long amp_Q16 = TO_Q16(1);
    cl_long fin_Q16 = 0;
    cl_long div_Q16 = 0;

    int i;
    for (i = 0; i < depth; i++)
    {
        div_Q16 += MUL_PAD_Q16(TO_Q16(256), amp_Q16);
        fin_Q16 += MUL_PAD_Q16(cl_noise_3d_Q16(xa_Q16, ya_Q16, za_Q16, seed), amp_Q16);
        amp_Q16 = DIV_PAD_Q16(amp_Q16, TO_Q16(2));
        xa_Q16 = MUL_PAD_Q16(xa_Q16, TO_Q16(2));
        ya_Q16 = MUL_PAD_Q16(ya_Q16, TO_Q16(2));
        za_Q16 = MUL_PAD_Q16(za_Q16, TO_Q16(2));
    }

    return DIV_PAD_Q16(fin_Q16, div_Q16);
}


void cl_catmull_rom_uniform_2d_Q16(
    cl_int2 p0_Q16, 
    cl_int2 p1_Q16,
    cl_int2 p2_Q16, 
    cl_int2 p3_Q16,
    cl_int t_Q16, //0-1 
    cl_int2* out_point_Q16,
    cl_int2* out_tangent_vec_Q16)
{
    cl_int t0_Q16 = 0;
    cl_int t1_Q16 = cl_distance_v2_Q16(p0_Q16, p1_Q16);
    cl_int t2_Q16 = cl_distance_v2_Q16(p1_Q16, p2_Q16);
    cl_int t3_Q16 = cl_distance_v2_Q16(p2_Q16, p3_Q16);


    cl_int t1_m_t0_Q16 = t1_Q16 - t0_Q16;
    cl_int t2_m_t1_Q16 = t2_Q16 - t1_Q16;
    cl_int t3_m_t2_Q16 = t3_Q16 - t2_Q16;
    cl_int t3_m_t1_Q16 = t3_Q16 - t1_Q16;
    

    //scale t_Q32 to be in [t1,t2]
    cl_int tscaled = MUL_PAD_Q16(t_Q16,(t2_Q16 - t1_Q16)) + t1_Q16;

    cl_int2 A_1_Q16;
    cl_int a1p0_Q16 = DIV_PAD_Q16((t1_Q16 - tscaled), (t1_m_t0_Q16));
    cl_int a1p1_Q16 = DIV_PAD_Q16((tscaled - t0_Q16), (t1_m_t0_Q16));
    A_1_Q16.x = MUL_PAD_Q16((a1p0_Q16),p0_Q16.x) + MUL_PAD_Q16((a1p1_Q16),p1_Q16.x);
    A_1_Q16.y = MUL_PAD_Q16((a1p0_Q16),p0_Q16.y) + MUL_PAD_Q16((a1p1_Q16),p1_Q16.y);
    

    cl_int2 A_2_Q16;
    cl_int a2p1_Q16 = DIV_PAD_Q16((t2_Q16 - tscaled), (t2_m_t1_Q16));
    cl_long a2p2_Q16 = DIV_PAD_Q16((tscaled - t1_Q16), (t2_m_t1_Q16));
    A_2_Q16.x = MUL_PAD_Q16((a2p1_Q16),p1_Q16.x) + MUL_PAD_Q16((a2p2_Q16),p2_Q16.x);
    A_2_Q16.y = MUL_PAD_Q16((a2p1_Q16),p1_Q16.y) + MUL_PAD_Q16((a2p2_Q16),p2_Q16.y);


    cl_int2 A_3_Q16;
    cl_int a3p2_Q16 = DIV_PAD_Q16((t3_Q16 - tscaled), (t3_m_t2_Q16));
    cl_int a3p3_Q16 = DIV_PAD_Q16((tscaled - t2_Q16), (t3_m_t2_Q16));
    A_3_Q16.x = MUL_PAD_Q16((a3p2_Q16),p2_Q16.x) + MUL_PAD_Q16((a3p3_Q16),p3_Q16.x);
    A_3_Q16.y = MUL_PAD_Q16((a3p2_Q16),p2_Q16.y) + MUL_PAD_Q16((a3p3_Q16),p3_Q16.y);


    cl_int2 B_1_Q16;
    cl_int b1a1 = DIV_PAD_Q16((t2_Q16 - tscaled), (t2_Q16 - t0_Q16));
    cl_int b1a2 = DIV_PAD_Q16((tscaled - t0_Q16), (t2_Q16 - t0_Q16));
    B_1_Q16.x = MUL_PAD_Q16(b1a1 ,A_1_Q16.x) + MUL_PAD_Q16(b1a2 , A_2_Q16.x);
    B_1_Q16.y = MUL_PAD_Q16(b1a1 , A_1_Q16.y) + MUL_PAD_Q16(b1a2 , A_2_Q16.y);

    cl_int2 B_2_Q16;
    cl_int b2a2 = DIV_PAD_Q16((t3_Q16 - tscaled), (t3_m_t1_Q16));
    cl_int b2a3 = DIV_PAD_Q16((tscaled - t1_Q16), (t3_m_t1_Q16));
    B_2_Q16.x = MUL_PAD_Q16(b2a2 , A_2_Q16.x) + MUL_PAD_Q16(b2a3 , A_3_Q16.x);
    B_2_Q16.y = MUL_PAD_Q16(b2a2 , A_2_Q16.y) + MUL_PAD_Q16(b2a3 , A_3_Q16.y);


    cl_int2 C_Q16;
    cl_int cb1 = DIV_PAD_Q16((t2_Q16 - tscaled), (t2_m_t1_Q16));
    cl_int cb2 = DIV_PAD_Q16((tscaled - t1_Q16), (t2_m_t1_Q16));

    C_Q16.x = MUL_PAD_Q16(cb1 , B_1_Q16.x) + MUL_PAD_Q16(cb2 , B_2_Q16.x);
    C_Q16.y = MUL_PAD_Q16(cb1 , B_1_Q16.y) + MUL_PAD_Q16(cb2 , B_2_Q16.y);
    *out_point_Q16 = C_Q16;



    //DERIVATIVES http://denkovacs.com/2016/02/catmull-rom-spline-derivatives/

    cl_int t2_m_t0_Q16 = t2_Q16 - t0_Q16;

    cl_int2 A1_Prime_Q16;
    cl_int s = DIV_PAD_Q16(TO_Q16(1), (t1_m_t0_Q16));
    A1_Prime_Q16.x = MUL_PAD_Q16(s, (p1_Q16 - p0_Q16).x);
    A1_Prime_Q16.y = MUL_PAD_Q16(s, (p1_Q16 - p0_Q16).y);

    cl_int2 A2_Prime_Q16;
    s = DIV_PAD_Q16(TO_Q16(1), (t2_m_t1_Q16));
    A2_Prime_Q16.x = MUL_PAD_Q16(s, (p2_Q16 - p1_Q16).x);
    A2_Prime_Q16.y = MUL_PAD_Q16(s, (p2_Q16 - p1_Q16).y);

    cl_int2 A3_Prime_Q16;
    s = DIV_PAD_Q16(TO_Q16(1), (t3_m_t2_Q16));
    A3_Prime_Q16.x = MUL_PAD_Q16(s, (p3_Q16 - p2_Q16).x);
    A3_Prime_Q16.y = MUL_PAD_Q16(s, (p3_Q16 - p2_Q16).y);

    cl_int2 B1_Prime_Q16;
    cl_int s1 = DIV_PAD_Q16(TO_Q16(1), (t2_m_t0_Q16));
    cl_int s2 = DIV_PAD_Q16((t2_Q16 - tscaled), (t2_m_t0_Q16));
    cl_int s3 = DIV_PAD_Q16((tscaled - t3_Q16), (t2_m_t0_Q16));

    B1_Prime_Q16.x = MUL_PAD_Q16(s1, (A2_Prime_Q16 - A1_Prime_Q16).x) 
        + MUL_PAD_Q16(s2,(A1_Prime_Q16.x))
        + MUL_PAD_Q16(s3,A2_Prime_Q16.x);
    B1_Prime_Q16.y = MUL_PAD_Q16(s1, (A2_Prime_Q16 - A1_Prime_Q16).y)
        + MUL_PAD_Q16(s2, (A1_Prime_Q16.y))
        + MUL_PAD_Q16(s3, A2_Prime_Q16.y);


    cl_int2 B2_Prime_Q16;
    s1 = DIV_PAD_Q16((1), (t3_m_t1_Q16));
    s2 = DIV_PAD_Q16((t3_Q16 - tscaled), (t3_m_t1_Q16));
    s3 = DIV_PAD_Q16((tscaled - t1_Q16), (t3_m_t1_Q16));

    B2_Prime_Q16.x = MUL_PAD_Q16(s1, (A3_Prime_Q16 - A2_Prime_Q16).x)
        + MUL_PAD_Q16(s2, (A2_Prime_Q16.x))
        + MUL_PAD_Q16(s3, A3_Prime_Q16.x);
    B2_Prime_Q16.y = MUL_PAD_Q16(s1, (A3_Prime_Q16 - A2_Prime_Q16).y)
        + MUL_PAD_Q16(s2, (A2_Prime_Q16.y))
        + MUL_PAD_Q16(s3, A3_Prime_Q16.y);


    cl_int2 C_Prime_Q16;
    s1 = DIV_PAD_Q16((1), (t2_m_t1_Q16));
    s2 = DIV_PAD_Q16((t2_Q16 - tscaled), (t2_m_t1_Q16));
    s3 = DIV_PAD_Q16((tscaled - t1_Q16), (t2_m_t1_Q16));

    C_Prime_Q16.x = MUL_PAD_Q16(s1, (B2_Prime_Q16 - B1_Prime_Q16).x)
        + MUL_PAD_Q16(s2, (B1_Prime_Q16.x))
        + MUL_PAD_Q16(s3, B2_Prime_Q16.x);
    C_Prime_Q16.y = MUL_PAD_Q16(s1, (B2_Prime_Q16 - B1_Prime_Q16).y)
        + MUL_PAD_Q16(s2, (B1_Prime_Q16.y))
        + MUL_PAD_Q16(s3, B2_Prime_Q16.y);

    *out_tangent_vec_Q16 = C_Prime_Q16;
}



ge_int3 GE_INT3_CROSS_PRODUCT_Q16(ge_int3 a, ge_int3 b) {
    long x = MUL_PAD_Q16(a.y, b.z) - MUL_PAD_Q16(a.z, b.y);
    long y = MUL_PAD_Q16(-a.x, b.z) + MUL_PAD_Q16(a.z, b.x);
    long z = MUL_PAD_Q16(a.x, b.y) - MUL_PAD_Q16(a.y, b.x);

    return (ge_int3) { x, y, z };
}
int GE_INT3_DOT_PRODUCT_Q16(ge_int3 a, ge_int3 b) {
    return MUL_PAD_Q16(a.x, b.x) +
        MUL_PAD_Q16(a.y, b.y) + 
        MUL_PAD_Q16(a.z, b.z);
}

ge_int3 GE_INT3_DIV_Q16(ge_int3 a, ge_int3 b) {
    ge_int3 res;
    res.x = DIV_PAD_Q16(a.x, b.x);
    res.y = DIV_PAD_Q16(a.y, b.y);
    res.z = DIV_PAD_Q16(a.z, b.z);
    return res;
}




//v rotated about axis 90 degrees - axis does not need to be normalized.
inline ge_int3 GE_VECTOR3_ROTATE_ABOUT_AXIS_POS90_Q16(ge_int3 v, ge_int3 axis)
{
    ge_int3 a = v;
    ge_int3 b = axis;

    int a_dot_b = GE_INT3_DOT_PRODUCT_Q16(a, b);
    int b_dot_b = GE_INT3_DOT_PRODUCT_Q16(b, b);

    int s = DIV_PAD_Q16(a_dot_b, b_dot_b);
    ge_int3 a_par_b;
    a_par_b.x = MUL_PAD_Q16(s, b.x);
    a_par_b.y = MUL_PAD_Q16(s, b.y);
    a_par_b.z = MUL_PAD_Q16(s, b.z);

    ge_int3 a_orth_b = INT3_SUB(a, a_par_b);
    int a_orth_b_mag = ge_length_v3_Q16(a_orth_b);

    ge_int3 w = GE_INT3_CROSS_PRODUCT_Q16(b, a_orth_b);
    int w_mag;
    ge_int3 w_norm = GE_INT3_NORMALIZE_Q16(w, &w_mag);

    ge_int3 v_rot;
    v_rot.x = MUL_PAD_Q16(a_orth_b_mag, w_norm.x) + a_par_b.x;
    v_rot.y = MUL_PAD_Q16(a_orth_b_mag, w_norm.y) + a_par_b.y;
    v_rot.z = MUL_PAD_Q16(a_orth_b_mag, w_norm.z) + a_par_b.z;

    return v_rot;
}



void fixedPointTests()
{
    int Q = 16;
    float floatingNumber = -16.1897f;
    printf("FloatingNumber: %f\n", floatingNumber);
    cl_long fixed_Q16 = FloatToFixed(floatingNumber, Q);

    floatingNumber = FixedToFloat(fixed_Q16, Q);

    printf("FloatingNumber(Check): %f\n", floatingNumber);

    for (float i = 0; i < 1.0f; i+= 0.01f)
    {
        cl_int interp_Q16 = 0;


    }

    printf("(Q2F)  1: %f\n", FixedToFloat(TO_Q16(1), 16));
    printf("(Q2F) -1: %f\n", FixedToFloat(TO_Q16(-1), 16));
    printf("(Q2F) -10: %f\n", FixedToFloat(TO_Q16(-10), 16));
    printf("(D2Q) -1: %d\n", TO_Q16(-1));

    printf(" (-2.2) %#x\n", FloatToFixed(-2.2f, 16));
    printf("-1.4: %f\n", FixedToFloat(FloatToFixed(-1.414185f, 16),16));
    printf("1.7: %f\n", FixedToFloat(FloatToFixed(1.748077f, 16), 16));


    printf("-0.5: %f\n", FixedToFloat(DIV_PAD_Q16(TO_Q16(-1), TO_Q16(2)), 16));
    printf("-0.823529: %f\n", FixedToFloat(DIV_PAD_Q16(FloatToFixed(-1.4,16), FloatToFixed(1.7, 16)), 16));

    printf("sqrt(0) = 0: %d\n", sqrt_i64(0));

    int a = 10;
    int b = 0;
    printf("10/0: %d\n", a / b);

    cl_int2 p0 = (cl_int2)(TO_Q16(0), TO_Q16(0));
    cl_int2 p1 = (cl_int2)(TO_Q16(1), TO_Q16(1));
    cl_int2 p2 = (cl_int2)(TO_Q16(2), TO_Q16(-2));
    cl_int2 p3 = (cl_int2)(TO_Q16(3), TO_Q16(4));

    for (float i = 0; i < 1.0f; i += 0.01f)
    {
        cl_int t_Q16 = FloatToFixed(i,16);
        cl_int2 point_Q16;
        cl_int2 tangent_Q16;
        cl_catmull_rom_uniform_2d_Q16(p0, p1, p2, p3, t_Q16, &point_Q16, &tangent_Q16);

       // printf("%f,%f\n", FixedToFloat(tangent_Q16.x, 16), FixedToFloat(tangent_Q16.y, 16));
    }

    cl_int3 test = (cl_int3)(42);
    printf("%d,%d,%d\n", test);

    cl_int2 v = (cl_int2)(TO_Q16(1), TO_Q16(1));
    cl_int len;
    cl_normalize_v2_Q16(&v, &len);
    printf("x: %f, y: %f, len: %f\n", FixedToFloat(v.x,16), FixedToFloat(v.y, 16), FixedToFloat(len, 16));


    v = (cl_int2)(TO_Q16(1), TO_Q16(1));
    cl_int2 proj = (cl_int2)(TO_Q16(0), TO_Q16(0));
    cl_int scalar;
    cl_project_2D_Q16(&v, &proj, &scalar);
    printf("[1,1] projected onto [0,0]: x: %f, y: %f, scalar: %f\n", FixedToFloat(v.x, 16), FixedToFloat(v.y, 16), FixedToFloat(scalar, 16));


    printf("Sign of -10: %f\n", FixedToFloat(SIGN_MAG_Q15_16(TO_Q16(-10)), 16));
    printf("Sign of 10: %f\n", FixedToFloat(SIGN_MAG_Q15_16(TO_Q16(10)), 16));
    printf("Sign of 0: %f\n", FixedToFloat(SIGN_MAG_Q15_16(TO_Q16(0)), 16));

    printf("QMP32-------------------------------------\n");
    printf("QMP32 Size: %d\n", sizeof(QMP32));
    printf("QMP64 Size: %d\n", sizeof(QMP64));

    const float z = -0.000123456;
    QMP32 numberZ = FloatToQMP32(z);
    printf("%f=", z); PrintQMP32(numberZ);



    const float a2 = -1.2;
    const float b2 = -11.548;    
    QMP32 numberA = FloatToQMP32(a2);
    QMP32 numberB = FloatToQMP32(b2);


    QMP32 numberR;
    numberR = ADD_QMP32(numberA, numberB);
    printf("%f+%f=%f = ",a2,b2,(float)a2+b2);
    PrintQMP32(numberR);

    numberR = MUL_QMP32(numberA, numberB);
    printf("%f*%f=%f = ", a2, b2, (float)a2*b2);
    PrintQMP32(numberR);

    numberR = DIV_QMP32(numberA, numberB);
    printf("%f/%f=%f = ", a2, b2, (float)a2 / b2);
    PrintQMP32(numberR);

    //QMP32 r_qmp = FloatToQMP32(1.001f);
    //QMP32 b_qmp = FloatToQMP32(1.001f);
    //for (long i = 0; i < 1000000; i++)
    //{
    //    r_qmp = ADD_QMP32(r_qmp, b_qmp);
    //}
    //PrintQMP32(r_qmp);


    //int r_Q16 = TO_Q16(1);
    //int b_Q16 = TO_Q16(1);

    //for (long i = 0; i < 1000000; i++)
    //{

    //    b_Q16 = TO_Q16(i)>>1;
    //    r_Q16 = r_Q16 + b_Q16>>2;
    //}
    //PrintQ16(r_Q16);
    printf("GE_INT* FUNC TESTS:\n");

    ge_int3 A;
    A.z = TO_Q16(10);
    ge_int3 B;
    B.x = TO_Q16(10);
    B.y = TO_Q16(1);



    Print_GE_INT3_Q16(A);
    Print_GE_INT3_Q16(B);
   // Print_GE_INT3_Q16(GE_INT3_CROSS_PRODUCT_Q16(A, B));

    //Print_GE_INT3_Q16(GE_VECTOR3_ROTATE_ABOUT_AXIS_POS90_Q16(B, INT3_NEG(A)));

    //Print_GE_INT3_Q16(GE_INT3_NORMALIZE_Q16( B));
   // GE_INT3_NORMALIZE_Q16



}

int GE_INT_TO_NEAREST_OF_TWO_Q16(int x_Q16, int a_Q16, int b_Q16)
{
    int dA = x_Q16 - a_Q16;
    int dB = x_Q16 - b_Q16;

    if(abs(dA) > abs(dB))
        return b_Q16;
    else
        return a_Q16;
}


cl_uchar GE_INT_IN_DOMAIN_Q16(int x_Q16, int a_Q16, int b_Q16)
{
    if(x_Q16 > a_Q16 && x_Q16 < b_Q16)
        return 1;
    else
        return 0;
}


cl_uchar GE_INT3_IN_DOMAIN_Q16(ge_int3 x_Q16, ge_int3 a_Q16, ge_int3 b_Q16)
{
    if((GE_INT_IN_DOMAIN_Q16(x_Q16.x, a_Q16.x, b_Q16.x) == 1) &&
       (GE_INT_IN_DOMAIN_Q16(x_Q16.y, a_Q16.y, b_Q16.y) == 1) &&
       (GE_INT_IN_DOMAIN_Q16(x_Q16.z, a_Q16.z, b_Q16.z) == 1))
       return 1;
    else
       return 0;
}

