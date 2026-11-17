/* haversine.c - Geodesic calculations implementation in pure C */
#include "haversine.h"
#include <math.h>

double deg_to_rad(double deg) {
    return deg * (PI_CONST / 180.0);
}

double rad_to_deg(double rad) {
    return rad * (180.0 / PI_CONST);
}

double haversine_distance_km(double lat1, double lon1, double lat2, double lon2) {
    double d_lat = deg_to_rad(lat2 - lat1);
    double d_lon = deg_to_rad(lon2 - lon1);

    double r_lat1 = deg_to_rad(lat1);
    double r_lat2 = deg_to_rad(lat2);

    double a = sin(d_lat / 2.0) * sin(d_lat / 2.0) +
               cos(r_lat1) * cos(r_lat2) *
               sin(d_lon / 2.0) * sin(d_lon / 2.0);

    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    return EARTH_RADIUS_KM * c;
}

double haversine_distance_meters(double lat1, double lon1, double lat2, double lon2) {
    return haversine_distance_km(lat1, lon1, lat2, lon2) * 1000.0;
}

double haversine_bearing(double lat1, double lon1, double lat2, double lon2) {
    double r_lat1 = deg_to_rad(lat1);
    double r_lat2 = deg_to_rad(lat2);
    double d_lon = deg_to_rad(lon2 - lon1);

    double y = sin(d_lon) * cos(r_lat2);
    double x = cos(r_lat1) * sin(r_lat2) - sin(r_lat1) * cos(r_lat2) * cos(d_lon);

    double bearing_rad = atan2(y, x);
    double bearing_deg = rad_to_deg(bearing_rad);

    return fmod(bearing_deg + 360.0, 360.0);
}
