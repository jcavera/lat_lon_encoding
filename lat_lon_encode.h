// Latitude/longitude encoding
#include <math.h>

double a     = 425983.999935;
double b     = 67797.4592677;
double torad = 0.01745329252;
double xconv = 2366.57777742;

unsigned long encode_lon_to_x (double longitude) {
    return ((unsigned long) ((longitude + 180) * xconv));
}

unsigned long encode_lat_to_y (double latitiude) {
    double S = sin(latitiude * torad);
    double Y = a - (b * log((1 + S) / (1 - S)));
    return ((unsigned long) Y);
}

double xcon  = 0.00042255108;
double twopi = 6.28318530718;
double bcon  = 1.47498152704E-5;
double todeg = 57.2957795131;

double decode_x_to_lon (unsigned long X) {
    return ((((double) X) * xcon) - 180.0);
}

double decode_y_to_lat (unsigned long Y) {
    double b = twopi - (Y * bcon);
    double a = pow(10, b);
    return (todeg * (asin((a - 1)/(a + 1))));
}
