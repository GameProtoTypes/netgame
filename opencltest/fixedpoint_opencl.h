#pragma once
#include "cl_type_glue.h"
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
cl_int length_Q16(int2 v_Q16)
{
    long2 v_1 = (long2)(v_Q16.x,v_Q16.y);
    long2 inner_Q32 = v_1 * v_1;

    cl_long innerSum_Q32 = inner_Q32.x + inner_Q32.y;
    cl_long len_Q16 = sqrt_i64(innerSum_Q32);
    return (cl_int)len_Q16;
}
cl_int distance_Q16(int2 v1_Q16, int2 v2_Q16)
{
    return length_Q16(v2_Q16 - v1_Q16);
}

cl_int distance2_Q16(cl_int x1_Q16, cl_int y1_Q16, cl_int x2_Q16, cl_int y2_Q16)
{
    return distance_Q16((int2)(x1_Q16, y1_Q16), (int2)(x2_Q16, y2_Q16));
}

void normalize_v_Q16(int2* v_Q16, cl_int* len_Q16)
{
    *len_Q16 = length_Q16(*v_Q16);
    long2 v = (((long2)((*v_Q16).x, (*v_Q16).y)) << 16) / (*len_Q16);
    *v_Q16 = (int2)(v.x, v.y);
}


void normalize_Q16(cl_int* x_Q16, cl_int* y_Q16, cl_int* len_Q16)
{
    int2 v = (int2)(*x_Q16, *y_Q16);
    normalize_v_Q16(&v, len_Q16);
    *x_Q16 = v.x;
    *y_Q16 = v.y;
}











