#include  <stdio.h>
#include  <math.h>

#define PI         ((float) 3.14159265359)
#define PI_2       ((float) 6.28318530718)    /* 2 * pi */
#define PI_180     ((float) 0.01745329251)    /* pi / 180 */
#define PI_360     ((float) 114.591559026)    /* 360 / pi */
#define PI_4       ((float) 0.07957747154)    /* 1/(pi * 4) */
#define mapSize    (0x00100000)               /* 256 << levelOfDetail [12] */

float Clip (float n, float minval, float maxval)  
{
    if (n < minval) return minval;
    if (n > maxval) return maxval;
    return n;
}

int LonToPixelX (float longitude)  
{  
    float x = (longitude + 180.0) / 360.0;   
    return ((int) Clip(x * mapSize + 0.5, 0, mapSize - 1));  
}

int LatToPixelY (float latitude)  
{  
    float s = sin(latitude * PI_180);  
    float y = 0.5 - (PI_4 * log((1 + s) / (1 - s)));  
    return ((int) Clip(y * mapSize + 0.5, 0, mapSize - 1));  
}

float PixelXToLon(int pixelX)  
{  
    float x = (Clip(pixelX, 0, mapSize - 1) / mapSize) - 0.5;  
    return (360 * x);  
}

float PixelYToLat(int pixelY)  
{  
    float y = 0.5 - (Clip(pixelY, 0, mapSize - 1) / mapSize);  
    return (90 - PI_360 * atan(exp(-y * PI_2)));
}


int main(void)
{
    float lat_i =  -84.987987;
    float lon_i =  178.456456;
    
    int x = LonToPixelX(lon_i);
    int y = LatToPixelY(lat_i);
    float lat_f = PixelYToLat(y);
    float lon_f = PixelXToLon(x);
    
    printf("lat i = %f \n", lat_i);
    printf("lon i = %f \n", lon_i);
    printf("x     = %08x \n", x);
    printf("y     = %08x \n", y);
    printf("lat f = %f [ err y = %f ]\n", lat_f, (lat_f - lat_i));
    printf("lon f = %f [ err x = %f ]\n", lon_f, (lon_f - lon_i));
    return 0;
}
