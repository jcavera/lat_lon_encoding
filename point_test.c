#include  <stdio.h>
#include  <math.h>

#define PI ((float) 3.141592653589793238462643)

float Clip (float n, float minValue, float maxValue)  
{  
    return fmin(fmax(n, minValue), maxValue);  
}

int LonToPixelX (float longitude, int levelOfDetail)  
{  
    float x = (longitude + 180) / 360;   
    unsigned int mapSize = (256 << levelOfDetail);  
    return ((int) Clip(x * mapSize + 0.5, 0, mapSize - 1));  
}

int LatToPixelY (float latitude, int levelOfDetail)  
{  
    float sinLatitude = sin(latitude * PI / 180);  
    float y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * PI);  
    unsigned int mapSize = (256 << levelOfDetail);  
    return ((int) Clip(y * mapSize + 0.5, 0, mapSize - 1));  
}

float PixelXToLon(int pixelX, int levelOfDetail)  
{  
    float mapSize = (256 << levelOfDetail);  
    float x = (Clip(pixelX, 0, mapSize - 1) / mapSize) - 0.5;  
    return (360 * x);  
}

float PixelYToLat(int pixelY, int levelOfDetail)  
{  
    float mapSize = (256 << levelOfDetail);  
    float y = 0.5 - (Clip(pixelY, 0, mapSize - 1) / mapSize);  
    return (90 - 360 * atan(exp(-y * 2 * PI)) / PI);  
}


int main(void)
{
    int   lod   = 12;
    float lat_i =  -84.987987;
    float lon_i =  178.456456;
    
    int x = LonToPixelX(lon_i, lod);
    int y = LatToPixelY(lat_i, lod);
    
    float lat_f = PixelYToLat(y, lod);
    float lon_f = PixelXToLon(x, lod);
    
    printf("lat i = %f \n", lat_i);
    printf("lon i = %f \n", lon_i);
    printf("x     = %08x \n", x);
    printf("y     = %08x \n", y);
    printf("lat f = %f \n", lat_f);
    printf("lon f = %f \n", lon_f);
    printf("err x = %f \n", (lon_f - lon_i));
    printf("err y = %f \n", (lat_f - lat_i));
    
    return 0;
}
