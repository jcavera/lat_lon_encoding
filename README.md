# Effective encoding of longitude and latitude in 40 bits

In Internet-of-Things (IoT) applications, it is often desirable to transmit GPS data as efficiently as possible. In
researching this issue, I came across several different methods. Often, latitude and longitude are sent as double 
precision floats, giving an overall size of 64 bits per field; 128 bits in total. Using a single-precision float
reduces that to 32 bits per field, but at the expense of a small amount of positioning accuracy.

WSPR makes use of the Maidenhead system, overlaying a grid of ever-smaller blocks over the globe. Individual
blocks can be referenced by a series of four characters. The WSPR protocol encodes these characters in to just
15 bits, though at a significant loss in resolution. This is to be expected as the Maidenhead encoding scheme is
meant to be easy for humans communicating via radio, not for machines.

The maximum resolution for any encoding scheme is given by subdividing the globe in to squares of equal area, with 
the number of square each being given a unique index. The surface area of the earth is approximately 5.1x1014 square 
meters. So if one wishes to index each of those square, 44 bits would be needed for perfect encoding. Reducing the
resolution to 10 meter squares requires 43 bits; 100 meter squares require 36 bits. Clearly, we’re getting in to the 
ballpark, but there is another issue: the process of encoding. Part of my search is that the algorithm be efficient
and easy to implement on even the smallest of microcontrollers with a tiny memory footprint and no trig libraries.

I found two good sources for that. The first, "Encoding Normal Vectors using Optimized Spherical Coordinates" by 
Smith, Petrova, and Schaefer, describes a complex algorithm, best suited to gaming systems and graphics processors. 
Though wonderfully compact, the math is difficult for a tiny microcontroller to carry out in a reasonable time.

The second is the encoding scheme used by Microsoft for Bing Maps. The Bing Maps Tile System is similar to the
Maidenhead system in that the Earth is subdivided in to ever-smaller grids. Unlike the Maidenhead system, the
resolution is more easily variable and is based on a single constant in the code. According to the paper, with just 20
bits per field, ground resolution can be on the order of 150 meters. The paper does not indicate an error figure, but I
found it to be reasonable in testing.

The Bing Maps encoding scheme is somewhat processor intensive (considering small microcontrollers), as the
algorithm makes use of a number of trigonometric and inverse functions. The Bing Maps algorithm is as follows:

````
public static void LatLongToPixelXY (double latitude, double longitude, int levelOfDetail, out int pixelX, out int pixelY)  
{  
    latitude = Clip(latitude, MinLatitude, MaxLatitude);  
    longitude = Clip(longitude, MinLongitude, MaxLongitude);  

    double x = (longitude + 180) / 360;   
    double sinLatitude = Math.Sin(latitude * Math.PI / 180);  
    double y = 0.5 - Math.Log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * Math.PI);  
  
    uint mapSize = (256 << levelOfDetail);  
    pixelX = (int) Clip(x * mapSize + 0.5, 0, mapSize - 1);  
    pixelY = (int) Clip(y * mapSize + 0.5, 0, mapSize - 1);  
}

public static void PixelXYToLatLong (int pixelX, int pixelY, int levelOfDetail, out double latitude, out double longitude)  
{  
    double mapSize = (256 << levelOfDetail);  
    double x = (Clip(pixelX, 0, mapSize - 1) / mapSize) - 0.5;  
    double y = 0.5 - (Clip(pixelY, 0, mapSize - 1) / mapSize);  
  
    latitude = 90 - 360 * Math.Atan(Math.Exp(-y * 2 * Math.PI)) / Math.PI;  
    longitude = 360 * x;  
}
````

Though the encoding and decoding algorithms look clean and quick, there are a few "difficult" functions to deal with,
particularly in a constrained system.  I wanted to reduce this to grade-school math for ease of computation, so I went
with Chebyshev polynomial approximations of the difficult functions.  Two 32-element arrays of coefficients gave me
"good enough" accuracy.

