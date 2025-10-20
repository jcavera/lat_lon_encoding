# Effective encoding of longitude and latitude in 40 bits

In Internet-of-Things (IoT) applications, it is often desirable to transmit GPS data as efficiently as possible. In
researching this issue, I came across several different methods. Often, latitude and longitude are sent as double precision
floats, giving an overall size of 64 bits per field; 128 bits in total. Using a single-precision float reduces
that to 32 bits per field, but at the expense of some accuracy.

WSPR makes use of the Maidenhead system, overlaying a grid of ever-smaller blocks over the globe. Individual
blocks can be referenced by a series of four characters. The WSPR protocol encodes these characters in to a mere
15 bits, though at a significant loss in resolution. This is to be expected as the Maidenhead encoding scheme is
meant to be easy for humans communicating via radio, not for machines.

Simple math tells us that the maximum resolution for any encoding scheme is given by subdividing the globe in to
squares of equal area, with the number of square each being given a unique index. The surface area of the earth is
approximately 5.1x1014 square meters. It follows that if one wishes to index each of those square, then one would
need 44 bits for perfect encoding. Reducing resolution to 10 meter squares requires 43 bits; 100 meter squares
require 36 bits. Clearly, we’re getting in to the ballpark, but there is another issue: the process of encoding. Part of
my search is that the algorithm be efficient and easy to implement on even the smallest of microcontrollers.
My search directed me to two sources. The first, "Encoding Normal Vectors using Optimized Spherical Coordinates" by 
Smith, Petrova, and Schaefer, describes a complex algorithm, best suited to gaming systems and graphics processors. 
Though wonderfully compact, the math is difficult for a tiny microcontroller to carry out in a reasonable time.

The second is the encoding scheme used by Microsoft for Bing Maps. The Bing Maps Tile System is similar to the
Maidenhead system in that the Earth is subdivided in to ever-smaller grids. Unlike the Maidenhead system, the
resolution is more easily variable and is based on a single constant in the code. According to the paper, with just 20
bits per field, ground resolution can be on the order of 150 meters. The paper does not indicate an error figure, but I
found it to be reasonable in testing.

The Bing Maps encoding scheme is somewhat processor intensive (considering small microcontrollers), as the
algorithm makes use of a number of trigonometric and inverse functions. By use of trigonometric identities, and by
combining constants in to "magic" numbers, I was able to considerably reduce the complexity of the algorithm such
that it could be implemented on a small microcontroller with high efficiency and minimum effort.

The Bing Maps algorithm is as follows:

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
````

Note that there are only two functions (sine and base-ten logarithm) that are "difficult". Both can be found in
standard libraries. The outputs (X and Y) are unsigned long integers, but only the lowest 20 bits are used from each.
Decoding is only slightly more difficult:

````
public static void PixelXYToLatLong (int pixelX, int pixelY, int levelOfDetail, out double latitude, out double longitude)  
{  
    double mapSize = (256 << levelOfDetail);  
    double x = (Clip(pixelX, 0, mapSize - 1) / mapSize) - 0.5;  
    double y = 0.5 - (Clip(pixelY, 0, mapSize - 1) / mapSize);  
  
    latitude = 90 - 360 * Math.Atan(Math.Exp(-y * 2 * Math.PI)) / Math.PI;  
    longitude = 360 * x;  
}
````

As before, there are only two standard library functions needed (inverse sine and power). The inputs (X and Y) are
20 bit unsigned integers and the latitude and longitude are floats.

Though the encoding and decoding algorithms look clean and quick, all of this is useless without a real-world test of
the accuracy. To this end, I implemented the algorithms in LabVIEW and ran them against every latitude from 89.9
degrees south to 89.9 degrees north; and every longitude from 180 degrees east to 179.9 degrees west, both in increments
of 0.05 degrees. As you can see from the algorithms, there are singularities at the poles, stemming from the
(1 + sin(x))/(1 – sin(x)) term. Thus, 90 degrees north and 90 degrees south were not included in the test.

First, latitude error was analyzed. For this, I encoded and then decode latitude and subtracted the result from the
initial figure. That was then analyzed, plotted, and overlaid on to a world map as shown in figure 1. The analysis of
the latitude error is as follows:

    Maximum error       0.000972896 degrees
    Minimum error       0.000005534 degrees
    Mean                0.000310100 degrees
    Standard deviation  0.0002451
    Variance            0.0000001

Longitude was similarly handled and the results are shown in figure 2. The analysis of longitude error is as follows:

    Maximum error       0.000423980 degrees
    Minimum error       0.000000191 degrees
    Mean                0.000212900 degrees
    Standard deviation  0.0001220
    Variance            0.0000000149

Of course, this does not translate directly in to positional error. In order to compute that, I made use of the standard
Haversine algorithm to compute distance between two points on a sphere. For the two points, I put in the original
latitude/longitude and the decoded latitude/longitude. The results of that are shown in figure 3, with the analysis
below.

    Maximum error       118.00900 meters
    Minimum error         0.01055 meters
    Mean                 39.60550 meters
    Standard deviation   26.88637
    Variance            722.8768

Though the maximum distance error is unfortunately near the equator, I consider the algorithm to be very useful
considering that this maximum is on the order of 100 meters. As one gets away from the equator, the distance error
drops to a mean of about 40 meters, which is more than sufficient to allow two nodes of a network to "see" each
other. Considering that the data is reduced from 128 bits to 40 using an easily computed algorithm, I believe that
this scheme offers considerable utility in the IoT arena.
