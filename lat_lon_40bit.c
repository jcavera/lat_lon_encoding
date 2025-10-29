#include "lat_lon_40bit.h"

#define PI         ((LL40_FTYPE)   3.14159265359)
#define PI_2       ((LL40_FTYPE)  -6.28318530718)     /* -2 * pi */
#define PI_180     ((LL40_FTYPE)   0.01745329251)     /* pi / 180 */
#define PI_360     ((LL40_FTYPE) 114.591559026)       /* 360 / pi */
#define PI_4       ((LL40_FTYPE)   0.07957747154)     /* 1/(pi * 4) */
#define MAP_SIZE   (0x00100000)                       /* 256 << levelOfDetail [12] */

// Return a number clipped to the minimum and maximum value range.

LL40_FTYPE clip_to_range (LL40_FTYPE n, LL40_FTYPE minval, LL40_FTYPE maxval) {
    if (n < minval) return minval;
    if (n > maxval) return maxval;
    return n;
}

// Chebyshev poly approximation to (atan(exp(x))) in the range of -pi to pi:

#define NUM_COEFFS 32
LL40_FTYPE coeffs_atan_e[NUM_COEFFS] = {1.5707963267948966,       0.8616499258747404,        3.9898639947466563e-17,    -0.14560225678982303,
                                        7.37257477290143e-18,     0.03657164440576444,       3.382710778154774e-17,     -0.01034896289878665,
                                       -2.42861286636753e-17,     0.0031413840346321952,     3.382710778154774e-17,     -0.0009992112885118679,
                                       -2.190088388420719e-16,    0.0003283763078465933,    -1.192622389734055e-16,     -0.00011050582947498186,
                                        1.6046192152785466e-16,   0.00003785850509181168,    4.347650711666873e-16,     -0.000013151919603233064,
                                        2.3006770100142404e-16,   0.000004620161444477246,  -3.209238430557093e-17,     -0.0000016380185063416281,
                                       -2.8601253310167607e-16,   5.855386819088619e-7,     -9.324138683375338e-18,     -2.1167846570057432e-7,
                                       -9.096456227153382e-17,    7.97167969440629e-8,      -2.4687283467494936e-16,    -3.768725189014872e-8  };

// Chebyshev poly approximation to (log((1 + sin(x)) / (1 - sin(x)))) in the range of -pi/2 to pi/2:

LL40_FTYPE coeffs_log_s[NUM_COEFFS] = {-2.220446049250313e-16,    5.042450681824305,         4.440892098500626e-16,      1.054868187102282,
                                        6.106226635438361e-16,    0.3462091255709088,        1.6653345369377348e-16,     0.1341509944074275,
                                        5.551115123125783e-16,    0.05657639481144727,       1.6653345369377348e-16,     0.025099244487717554,
                                       -1.4988010832439613e-15,   0.011515544050053228,     -3.3306690738754696e-16,     0.005411388552472862,
                                       -2.7200464103316335e-15,   0.0025888930523743636,     1.1102230246251565e-16,     0.0012558465877667013,
                                        0.000,                    0.0006158254353316461,     2.3314683517128287e-15,     0.00030436091270441556,
                                       -1.609823385706477e-15,    0.000150829709385375,      1.2073675392798577e-15,     0.00007378672377310325,
                                        1.5404344466674047e-15,   0.00003347413001082711,   -4.718447854656915e-16,      0.000009598620639480332  };

LL40_FTYPE chebyshev_approx (const LL40_FTYPE* coeffs, int num_coeffs, LL40_FTYPE x, LL40_FTYPE x_min, LL40_FTYPE x_max) {
    LL40_FTYPE x_rel_2  = -2.0 + (4.0 * (x - x_min) / (x_max - x_min));
    LL40_FTYPE d        = 0.0;
    LL40_FTYPE dd       = 0.0;
    LL40_FTYPE temp     = 0.0;
    for (int i = num_coeffs - 1; i > 0; i--) {
        temp = d;
        d    = (x_rel_2 * d) - dd + coeffs[i];
        dd   = temp;
    }
    return ((0.5 * x_rel_2 * d) - dd + (0.5 * coeffs[0]));
}

int encode_lon_to_x (LL40_FTYPE longitude) {  
    LL40_FTYPE x = (longitude + 180.0) / 360.0;   
    return ((int) clip_to_range(x * MAP_SIZE + 0.5, 0.0, MAP_SIZE - 1));  
}

int encode_lat_to_y (LL40_FTYPE latitude) {  
    LL40_FTYPE a = latitude * PI_180;
    LL40_FTYPE b = chebyshev_approx(coeffs_log_s, NUM_COEFFS, a, -1.5, 1.5);
    LL40_FTYPE y = 0.5 - (PI_4 * b);
    return ((int) clip_to_range(y * MAP_SIZE + 0.5, 0.0, MAP_SIZE - 1));  
}

LL40_FTYPE decode_x_to_lon (int pixelX) {  
    LL40_FTYPE x = (clip_to_range(pixelX, 0.0, MAP_SIZE - 1) / MAP_SIZE) - 0.5;  
    return (360.0 * x);  
}

LL40_FTYPE decode_y_to_lat (int pixelY) {  
    LL40_FTYPE y = PI_2 * (0.5 - (clip_to_range(pixelY, 0.0, MAP_SIZE - 1) / MAP_SIZE));  
    LL40_FTYPE a = chebyshev_approx(coeffs_atan_e, NUM_COEFFS, y, -3.2, 3.2);
    return (90.0 - (PI_360 * a));
}

LL40_ITYPE combine_xy       (int x, int y)                   {  return (((LL40_ITYPE) x) + (((LL40_ITYPE) y) << 20));             }
int        split_x_from_xy  (LL40_ITYPE xy)                  {  return ((int) (xy & 0x000fffff));                                 }
int        split_y_from_xy  (LL40_ITYPE xy)                  {  return ((int) ((xy >> 20) & 0x000fffff));                         }

LL40_ITYPE encode_to_xy     (LL40_FTYPE lat, LL40_FTYPE lon) {  return (combine_xy(encode_lon_to_x(lon), encode_lat_to_y(lat)));  }
LL40_FTYPE decode_xy_to_lat (LL40_ITYPE xy)                  {  return (decode_y_to_lat(split_y_from_xy(xy)));                    }
LL40_FTYPE decode_xy_to_lon (LL40_ITYPE xy)                  {  return (decode_x_to_lon(split_x_from_xy(xy)));                    }

