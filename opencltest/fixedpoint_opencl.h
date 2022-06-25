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

cl_int length_Q16(cl_int x1_Q16, cl_int y1_Q16, cl_int x2_Q16, cl_int y2_Q16)
{
    cl_long deltax_Q16 = x1_Q16 - x2_Q16;
    cl_long deltay_Q16 = y1_Q16 - y2_Q16;

    cl_long innerx_Q32 = (deltax_Q16 * deltax_Q16);
    cl_long innery_Q32 = (deltay_Q16 * deltay_Q16);

    cl_long inner_Q32 = innerx_Q32 + innery_Q32;

    cl_long dist_Q16 = sqrt_i64(inner_Q32);
    return (cl_int)dist_Q16;
}

void normalize_Q16(cl_int* x_Q16, cl_int* y_Q16, cl_int* len_Q16)
{
    *len_Q16 = length_Q16(*x_Q16, *y_Q16, 0, 0);
    *x_Q16 = (((cl_long)(*x_Q16)) << 16) / (* len_Q16);
    *y_Q16 = (((cl_long)(*y_Q16)) << 16) / (* len_Q16);
}
