#define LL40_FTYPE  double               /* floating point data type */
#define LL40_ITYPE  unsigned long long   /* 64b unsigned integer type */

int        encode_lon_to_x  (LL40_FTYPE longitude);
int        encode_lat_to_y  (LL40_FTYPE latitude);
LL40_FTYPE decode_x_to_lon  (int pixelX);
LL40_FTYPE decode_y_to_lat  (int pixelY);

LL40_ITYPE combine_xy       (int x, int y);
int        split_x_from_xy  (LL40_ITYPE xy);
int        split_y_from_xy  (LL40_ITYPE xy);

LL40_ITYPE encode_to_xy     (LL40_FTYPE lat, FTYPE lon);
LL40_FTYPE decode_xy_to_lat (LL40_ITYPE xy);
LL40_FTYPE decode_xy_to_lon (LL40_ITYPE xy);
