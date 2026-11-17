/* haversine.h - Geodesic calculations in pure C */
#ifndef TRANSITX_HAVERSINE_H
#define TRANSITX_HAVERSINE_H

#ifdef __cplusplus
extern "C" {
#endif

#define EARTH_RADIUS_KM 6371.0
#define PI_CONST 3.14159265358979323846

/* Convert degrees to radians and vice versa */
double deg_to_rad(double deg);
double rad_to_deg(double rad);

/* Great-circle distance between two GPS coordinates */
double haversine_distance_km(double lat1, double lon1, double lat2, double lon2);
double haversine_distance_meters(double lat1, double lon1, double lat2, double lon2);

/* Calculate initial bearing in degrees (0 - 360) */
double haversine_bearing(double lat1, double lon1, double lat2, double lon2);

#ifdef __cplusplus
}
#endif

#endif /* TRANSITX_HAVERSINE_H */
