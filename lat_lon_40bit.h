#ifndef LAT_LON_40B_H
#define LAT_LON_40B_H

#define LL40_FTYPE  double               /* floating point data type - platform specific */
#define LL40_ITYPE  unsigned long long   /* 64b unsigned integer type - platform specific */

int        encode_lon_to_x  (LL40_FTYPE longitude);       // encode floating point longitude to a 20-bit integer
int        encode_lat_to_y  (LL40_FTYPE latitude);        // encode floating point latitude to a 20-bit integer
LL40_FTYPE decode_x_to_lon  (int pixelX);                 // decode a 20-bit integer representation of longitude to floating point
LL40_FTYPE decode_y_to_lat  (int pixelY);                 // decode a 20-bit integer representation of latitude to floating point

LL40_ITYPE combine_xy       (int x, int y);               // combine two 20-bit integers (x and y) to a 40-bit (xy)
int        split_x_from_xy  (LL40_ITYPE xy);              // split the 20-bit x component out of the combined 40-bit (xy) integer
int        split_y_from_xy  (LL40_ITYPE xy);              // split the 20-bit y component out of the combined 40-bit (xy) integer

LL40_ITYPE encode_to_xy     (LL40_FTYPE lat, FTYPE lon);  // encode floating point latitude and longitude to a 40-bit integer
LL40_FTYPE decode_xy_to_lat (LL40_ITYPE xy);              // fetch the floating point latitude from the combined 40-bit (xy) integer
LL40_FTYPE decode_xy_to_lon (LL40_ITYPE xy);              // fetch the floating point longitude from the combined 40-bit (xy) integer

#endif
