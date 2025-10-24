import numpy as np

PI       = (  3.14159265359)
PI_2     = ( -6.28318530718)     ## -2 * pi
PI_180   = (  0.01745329251)     ## pi / 180
PI_360   = (114.591559026)       ## 360 / pi
PI_4     = (  0.07957747154)     ## 1/(pi * 4)
MAP_SIZE = (0x00100000)          ## 256 << levelOfDetail [12]

def clip_to_range (n, minval, maxval):
    if (n < minval): return minval
    if (n > maxval): return maxval
    return n

## haversine distance formula for computing the error in distance between the initial and final points
def distance(s_lat, s_lng, e_lat, e_lng):
   R = 6373.0  ## approximate radius of earth in km
   s_lat = s_lat * PI_180
   s_lng = s_lng * PI_180
   e_lat = e_lat * PI_180
   e_lng = e_lng * PI_180
   d = np.sin((e_lat - s_lat)/2)**2 + np.cos(s_lat)*np.cos(e_lat) * np.sin((e_lng - s_lng)/2)**2
   return (2 * R * np.arcsin(np.sqrt(d)))

NUM_COEFFS    = 32
coeffs_atan_e = [  1.5707963267948966,       0.8616499258747404,        3.9898639947466563e-17,    -0.14560225678982303,      \
                   7.37257477290143e-18,     0.03657164440576444,       3.382710778154774e-17,     -0.01034896289878665,      \
                  -2.42861286636753e-17,     0.0031413840346321952,     3.382710778154774e-17,     -0.0009992112885118679,    \
                  -2.190088388420719e-16,    0.0003283763078465933,    -1.192622389734055e-16,     -0.00011050582947498186,   \
                   1.6046192152785466e-16,   0.00003785850509181168,    4.347650711666873e-16,     -0.000013151919603233064,  \
                   2.3006770100142404e-16,   0.000004620161444477246,  -3.209238430557093e-17,     -0.0000016380185063416281, \
                  -2.8601253310167607e-16,   5.855386819088619e-7,     -9.324138683375338e-18,     -2.1167846570057432e-7,    \
                  -9.096456227153382e-17,    7.97167969440629e-8,      -2.4687283467494936e-16,    -3.768725189014872e-8  ]

coeffs_log_s  = [ -2.220446049250313e-16,    5.042450681824305,         4.440892098500626e-16,      1.054868187102282,        \
                   6.106226635438361e-16,    0.3462091255709088,        1.6653345369377348e-16,     0.1341509944074275,       \
                   5.551115123125783e-16,    0.05657639481144727,       1.6653345369377348e-16,     0.025099244487717554,     \
                  -1.4988010832439613e-15,   0.011515544050053228,     -3.3306690738754696e-16,     0.005411388552472862,     \
                  -2.7200464103316335e-15,   0.0025888930523743636,     1.1102230246251565e-16,     0.0012558465877667013,    \
                   0.000,                    0.0006158254353316461,     2.3314683517128287e-15,     0.00030436091270441556,   \
                  -1.609823385706477e-15,    0.000150829709385375,      1.2073675392798577e-15,     0.00007378672377310325,   \
                   1.5404344466674047e-15,   0.00003347413001082711,   -4.718447854656915e-16,      0.000009598620639480332 ]

def chebyshev_approx (coeffs, num_coeffs, x, x_min, x_max):
    x_rel_2  = -2.0 + (4.0 * (x - x_min) / (x_max - x_min))
    d        = 0.0
    dd       = 0.0
    temp     = 0.0
    i        = num_coeffs - 1
    while (i > 0):
        temp = d
        d    = (x_rel_2 * d) - dd + coeffs[i]
        dd   = temp
        i -= 1
    return ((0.5 * x_rel_2 * d) - dd + (0.5 * coeffs[0]))

def encode_lon_to_x (longitude):  
    x = (longitude + 180.0) / 360.0
    return int(clip_to_range(x * MAP_SIZE + 0.5, 0, MAP_SIZE - 1))

def encode_lat_to_y (latitude):
    a = latitude * PI_180
    b = chebyshev_approx(coeffs_log_s, NUM_COEFFS, a, -1.5, 1.5)
    y = 0.5 - (PI_4 * b)
    return int(clip_to_range(y * MAP_SIZE + 0.5, 0, MAP_SIZE - 1))

def decode_x_to_lon (pixelX): 
    x = (clip_to_range(pixelX, 0, MAP_SIZE - 1) / MAP_SIZE) - 0.5
    return (360 * x)

def decode_y_to_lat (pixelY):
    y = PI_2 * (0.5 - (clip_to_range(pixelY, 0, MAP_SIZE - 1) / MAP_SIZE))
    a = chebyshev_approx(coeffs_atan_e, NUM_COEFFS, y, -3.2, 3.2)
    return (90 - (PI_360 * a))

def find_max_distance_error ():
    dist_err  = 0.00
    for lon_i in np.arange(-179.9, 179.8, 0.1):
    print ("lon = " + "{:.2f}".format(lon_i) + "     ", end='\r')
    for lat_i in np.arange(-84.9, 84.8, 0.1):
        y     = encode_lat_to_y(lat_i)
        x     = encode_lon_to_x(lon_i)
        lat_f = decode_y_to_lat(y)
        lon_f = decode_x_to_lon(x)
        dist  = 1000 * distance(lat_i, lon_i, lat_f, lon_f)
        if (dist > dist_err):
            dist_err = dist
    print ("\nmax dist err = ", dist_err)  ## should be approx. 27m
    

lat_i =  -84.987987
lon_i =  178.456456
y     = encode_lat_to_y(lat_i)
x     = encode_lon_to_x(lon_i)
lat_f = decode_y_to_lat(y)
lon_f = decode_x_to_lon(x)
dist  = 1000 * distance(lat_i, lon_i, lat_f, lon_f)

print("lat i = ", "{:.7f}".format(lat_i))
print("lon i = ", "{:.7f}".format(lon_i))
print("x     = ", '{:08x}'.format(x))
print("y     = ", '{:08x}'.format(y))
print("lat f = ", "{:.7f}".format(lat_f), " err y = ", "{:.7f}".format(lat_f - lat_i))
print("lon f = ", "{:.7f}".format(lon_f), " err x = ", "{:.7f}".format(lon_f - lon_i))
print("dist  = ", "{:.2f}".format(dist), "m")

