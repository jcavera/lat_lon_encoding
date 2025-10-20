// Latitude/longitude encoding
#include <math.h>

float a     = 425983.999935;
float b     = 67797.4592677;
float torad = 0.01745329252;
float xconv = 2366.57777742;

unsigned long encode_lon_to_x (float longitude) {
    return ((unsigned long) ((longitude + 180) * xconv));
}

unsigned long encode_lat_to_y (float latitiude) {
    float S = sin(latitiude * torad);
    float Y = a - (b * log((1 + S) / (1 - S)));
    return ((unsigned long) Y);
}

float xcon  = 0.00042255108;
float twopi = 6.28318530718;
float bcon  = 1.47498152704E-5;
float todeg = 57.2957795131;

float decode_x_to_lon (unsigned long X) {
    return ((((float) X) * xcon) - 180.0);
}

float decode_y_to_lat (unsigned long Y) {
    float b = twopi - (Y * bcon);
    float a = pow(10, b);
    return (todeg * (asin((a - 1)/(a + 1))));
}
