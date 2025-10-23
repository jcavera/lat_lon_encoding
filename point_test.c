#include  <stdio.h>
#include  <math.h>

#define FTYPE      double

#define PI         ((FTYPE)   3.14159265359)
#define PI_2       ((FTYPE)  -6.28318530718)  /* -2 * pi */
#define PI_180     ((FTYPE)   0.01745329251)  /* pi / 180 */
#define PI_360     ((FTYPE) 114.591559026)    /* 360 / pi */
#define PI_4       ((FTYPE)   0.07957747154)  /* 1/(pi * 4) */
#define mapSize    (0x00100000)               /* 256 << levelOfDetail [12] */

FTYPE Clip (FTYPE n, FTYPE minval, FTYPE maxval) {
    if (n < minval) return minval;
    if (n > maxval) return maxval;
    return n;
}

// Chebyshev poly via https://stuffmatic.com/chebyshev/
// Chebyshev poly approximation to (atan(exp(x))) in the range of -pi to pi:

#define NUM_COEFFS 32
FTYPE coeffs_atan_e[NUM_COEFFS] = {     1.5707963267948966,       0.8616499258747404,        3.9898639947466563e-17,    -0.14560225678982303,
                                        7.37257477290143e-18,     0.03657164440576444,       3.382710778154774e-17,     -0.01034896289878665,
                                       -2.42861286636753e-17,     0.0031413840346321952,     3.382710778154774e-17,     -0.0009992112885118679,
                                       -2.190088388420719e-16,    0.0003283763078465933,    -1.192622389734055e-16,     -0.00011050582947498186,
                                        1.6046192152785466e-16,   0.00003785850509181168,    4.347650711666873e-16,     -0.000013151919603233064,
                                        2.3006770100142404e-16,   0.000004620161444477246,  -3.209238430557093e-17,     -0.0000016380185063416281,
                                       -2.8601253310167607e-16,   5.855386819088619e-7,     -9.324138683375338e-18,     -2.1167846570057432e-7,
                                       -9.096456227153382e-17,    7.97167969440629e-8,      -2.4687283467494936e-16,    -3.768725189014872e-8  };

// Chebyshev poly approximation to (log((1 + sin(x)) / (1 - sin(x)))) in the range of -pi/2 to pi/2:

FTYPE coeffs_log_s[NUM_COEFFS] = {     -2.220446049250313e-16,    5.042450681824305,         4.440892098500626e-16,      1.054868187102282,
                                        6.106226635438361e-16,    0.3462091255709088,        1.6653345369377348e-16,     0.1341509944074275,
                                        5.551115123125783e-16,    0.05657639481144727,       1.6653345369377348e-16,     0.025099244487717554,
                                       -1.4988010832439613e-15,   0.011515544050053228,     -3.3306690738754696e-16,     0.005411388552472862,
                                       -2.7200464103316335e-15,   0.0025888930523743636,     1.1102230246251565e-16,     0.0012558465877667013,
                                        0.000,                    0.0006158254353316461,     2.3314683517128287e-15,     0.00030436091270441556,
                                       -1.609823385706477e-15,    0.000150829709385375,      1.2073675392798577e-15,     0.00007378672377310325,
                                        1.5404344466674047e-15,   0.00003347413001082711,   -4.718447854656915e-16,      0.000009598620639480332  };

FTYPE chebyshev (const FTYPE* coeffs, int num_coeffs, FTYPE x, FTYPE x_min, FTYPE x_max) {
    FTYPE x_rel_2  = -2.0 + (4.0 * (x - x_min) / (x_max - x_min));
    FTYPE d        = 0.0;
    FTYPE dd       = 0.0;
    FTYPE temp     = 0.0;
    for (int i = num_coeffs - 1; i > 0; i--) {
        temp = d;
        d    = (x_rel_2 * d) - dd + coeffs[i];
        dd   = temp;
    }
    return ((0.5 * x_rel_2 * d) - dd + (0.5 * coeffs[0]));
}


int LonToPixelX (FTYPE longitude) {  
    FTYPE x = (longitude + 180.0) / 360.0;   
    return ((int) Clip(x * mapSize + 0.5, 0, mapSize - 1));  
}

int LatToPixelY (FTYPE latitude) {  
//  FTYPE s = sin(latitude * PI_180);  
//  FTYPE y = 0.5 - (PI_4 * log((1 + s) / (1 - s)));
    FTYPE a = latitude * PI_180;
    FTYPE b = chebyshev(coeffs_log_s, NUM_COEFFS, a, -1.5, 1.5);
    FTYPE y = 0.5 - (PI_4 * b);
    return ((int) Clip(y * mapSize + 0.5, 0, mapSize - 1));  
}

FTYPE PixelXToLon (int pixelX) {  
    FTYPE x = (Clip(pixelX, 0, mapSize - 1) / mapSize) - 0.5;  
    return (360 * x);  
}

FTYPE PixelYToLat (int pixelY) {  
    FTYPE y = PI_2 * (0.5 - (Clip(pixelY, 0, mapSize - 1) / mapSize));  
//  return (90 - (PI_360 * atan(exp(y))));
    FTYPE a = chebyshev(coeffs_atan_e, NUM_COEFFS, y, -3.2, 3.2);
    return (90 - (PI_360 * a));
}

int main(void)
{
    FTYPE lat_i =  -84.987987;
    FTYPE lon_i =  178.456456;
    
    int x = LonToPixelX(lon_i);
    int y = LatToPixelY(lat_i);
    FTYPE lat_f = PixelYToLat(y);
    FTYPE lon_f = PixelXToLon(x);
    
    printf("lat i = %f \n", lat_i);
    printf("lon i = %f \n", lon_i);
    printf("x     = %08x \n", x);
    printf("y     = %08x \n", y);
    printf("lat f = %f [ err y = %f ]\n", lat_f, (lat_f - lat_i));
    printf("lon f = %f [ err x = %f ]\n", lon_f, (lon_f - lon_i));
    return 0;
}
