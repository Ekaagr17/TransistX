// haversine.cpp - Haversine distance implementation
#include "transitx/algorithms/haversine.hpp"

namespace transitx {
namespace algorithms {

double Haversine::calculate_distance_km(double lat1, double lon1, double lat2, double lon2) {
    double d_lat = to_radians(lat2 - lat1);
    double d_lon = to_radians(lon2 - lon1);

    double r_lat1 = to_radians(lat1);
    double r_lat2 = to_radians(lat2);

    double a = std::sin(d_lat / 2.0) * std::sin(d_lat / 2.0) +
               std::cos(r_lat1) * std::cos(r_lat2) *
               std::sin(d_lon / 2.0) * std::sin(d_lon / 2.0);

    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return EARTH_RADIUS_KM * c;
}

double Haversine::calculate_distance_meters(double lat1, double lon1, double lat2, double lon2) {
    return calculate_distance_km(lat1, lon1, lat2, lon2) * 1000.0;
}

double Haversine::calculate_bearing(double lat1, double lon1, double lat2, double lon2) {
    double r_lat1 = to_radians(lat1);
    double r_lat2 = to_radians(lat2);
    double d_lon = to_radians(lon2 - lon1);

    double y = std::sin(d_lon) * std::cos(r_lat2);
    double x = std::cos(r_lat1) * std::sin(r_lat2) -
               std::sin(r_lat1) * std::cos(r_lat2) * std::cos(d_lon);

    double bearing_rad = std::atan2(y, x);
    double bearing_deg = to_degrees(bearing_rad);

    return std::fmod(bearing_deg + 360.0, 360.0);
}

} // namespace algorithms
} // namespace transitx
