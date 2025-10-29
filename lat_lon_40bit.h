int   encode_lon_to_x  (FTYPE longitude);
int   encode_lat_to_y  (FTYPE latitude);
FTYPE decode_x_to_lon  (int pixelX);
FTYPE decode_y_to_lat  (int pixelY);

ITYPE combine_xy       (int x, int y);
int   split_x_from_xy  (ITYPE xy);
int   split_y_from_xy  (ITYPE xy);

ITYPE encode_to_xy     (FTYPE lat, FTYPE lon);
FTYPE decode_xy_to_lat (ITYPE xy);
FTYPE decode_xy_to_lon (ITYPE xy);
