#include <stdio.h>   // only used for the demo printf statements
#include <math.h>    // only used for the haversine distance measuring algorithm

#define FTYPE      double                        /* floating point data type */
#define ITYPE      unsigned long long            /* 64b unsigned integer type */
#define PI         ((FTYPE)   3.14159265359)
#define PI_2       ((FTYPE)  -6.28318530718)     /* -2 * pi */
#define PI_180     ((FTYPE)   0.01745329251)     /* pi / 180 */
#define PI_360     ((FTYPE) 114.591559026)       /* 360 / pi */
#define PI_4       ((FTYPE)   0.07957747154)     /* 1/(pi * 4) */
#define MAP_SIZE   (0x00100000)                  /* 256 << levelOfDetail [12] */

// Return a number clipped to the minimum and maximum value range.

FTYPE clip_to_range (FTYPE n, FTYPE minval, FTYPE maxval) {
    if (n < minval) return minval;
    if (n > maxval) return maxval;
    return n;
}

// haversine distance formula for computing the error in distance between the initial and final points in km
FTYPE distance (FTYPE s_lat, FTYPE s_lng, FTYPE e_lat, FTYPE e_lng) {
    FTYPE R = 6373.0;        // approximate radius of earth in km
    s_lat = s_lat * PI_180;  // convert all to radians
    s_lng = s_lng * PI_180;
    e_lat = e_lat * PI_180;
    e_lng = e_lng * PI_180;
    FTYPE a = sin((e_lat - s_lat)/2);
    FTYPE b = sin((e_lng - s_lng)/2);
    FTYPE d = (a * a) + (cos(s_lat) * cos(e_lat) * b * b);
    return (2 * R * asin(sqrt(d)));
}

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

FTYPE chebyshev_approx (const FTYPE* coeffs, int num_coeffs, FTYPE x, FTYPE x_min, FTYPE x_max) {
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

int encode_lon_to_x (FTYPE longitude) {  
    FTYPE x = (longitude + 180.0) / 360.0;   
    return ((int) clip_to_range(x * MAP_SIZE + 0.5, 0, MAP_SIZE - 1));  
}

int encode_lat_to_y (FTYPE latitude) {  
    FTYPE a = latitude * PI_180;
    FTYPE b = chebyshev_approx(coeffs_log_s, NUM_COEFFS, a, -1.5, 1.5);
    FTYPE y = 0.5 - (PI_4 * b);
    return ((int) clip_to_range(y * MAP_SIZE + 0.5, 0, MAP_SIZE - 1));  
}

FTYPE decode_x_to_lon (int pixelX) {  
    FTYPE x = (clip_to_range(pixelX, 0, MAP_SIZE - 1) / MAP_SIZE) - 0.5;  
    return (360 * x);  
}

FTYPE decode_y_to_lat (int pixelY) {  
    FTYPE y = PI_2 * (0.5 - (clip_to_range(pixelY, 0, MAP_SIZE - 1) / MAP_SIZE));  
    FTYPE a = chebyshev_approx(coeffs_atan_e, NUM_COEFFS, y, -3.2, 3.2);
    return (90 - (PI_360 * a));
}

ITYPE combine_xy (int x, int y) {
    return (((ITYPE) x) + (((ITYPE) y) << 20));
}

void test_single_point (FTYPE lat_i, FTYPE lon_i) {
    int   y     = encode_lat_to_y(lat_i);
    int   x     = encode_lon_to_x(lon_i);
    FTYPE lat_f = decode_y_to_lat(y);
    FTYPE lon_f = decode_x_to_lon(x);
    ITYPE xy    = combine_xy(x, y);
    
    printf("lat i = %f \n", lat_i);
    printf("lon i = %f \n", lon_i);
    printf("x     = %08x \n", x);
    printf("y     = %08x \n", y);
    printf("xy    = %llx \n", xy);
    printf("lat f = %f [ err y = %f ]\n", lat_f, (lat_f - lat_i));
    printf("lon f = %f [ err x = %f ]\n", lon_f, (lon_f - lon_i));
}

FTYPE find_max_distance_error (void) {
    FTYPE dist_err  = 0.00;
    FTYPE lon_i     = -179.9;
    FTYPE err_lat, err_lon;
    FTYPE x, y, lat_f, lon_f, dist;
    while (lon_i <= 179.9) {
        FTYPE lat_i = -84.9;
        printf("\rlon = %f ", lon_i);
        fflush(stdout);
        while (lat_i <= 84.9) {
            y     = encode_lat_to_y(lat_i);
            x     = encode_lon_to_x(lon_i);
            lat_f = decode_y_to_lat(y);
            lon_f = decode_x_to_lon(x);
            dist  = 1000.0 * distance(lat_i, lon_i, lat_f, lon_f);
            if (dist > dist_err) {
                dist_err = dist;
                err_lat  = lat_i;
                err_lon  = lon_i;
            } lat_i += 0.01;
        } lon_i += 0.01;
    }
    printf("\nmax dist err = %f \n", dist_err);
    printf("err lat = %f \n", err_lat);
    printf("err lon = %f \n", err_lon);
}

int main (void) {
    test_single_point(-84.987987, 178.456456);
    return 0;
}
