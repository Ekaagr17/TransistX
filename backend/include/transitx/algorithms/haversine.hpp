// haversine.hpp - Haversine distance calculations between geographical coordinates
#pragma once
#ifndef TRANSITX_ALGORITHMS_HAVERSINE_HPP
#define TRANSITX_ALGORITHMS_HAVERSINE_HPP

#include <cmath>

namespace transitx {
namespace algorithms {

class Haversine {
public:
    // Earth's mean radius in kilometers
    static constexpr double EARTH_RADIUS_KM = 6371.0;
    static constexpr double PI = 3.14159265358979323846;

    // Convert degrees to radians
    static inline double to_radians(double degrees) {
        return degrees * (PI / 180.0);
    }

    // Convert radians to degrees
    static inline double to_degrees(double radians) {
        return radians * (180.0 / PI);
    }

    /**
     * Calculate Great-Circle distance between two coordinates in kilometers using Haversine formula.
     * @param lat1 Latitude of point 1 (in degrees)
     * @param lon1 Longitude of point 1 (in degrees)
     * @param lat2 Latitude of point 2 (in degrees)
     * @param lon2 Longitude of point 2 (in degrees)
     * @return Distance in kilometers
     */
    static double calculate_distance_km(double lat1, double lon1, double lat2, double lon2);

    /**
     * Calculate Great-Circle distance in meters
     */
    static double calculate_distance_meters(double lat1, double lon1, double lat2, double lon2);

    /**
     * Calculate initial bearing (forward azimuth) from point 1 to point 2 in degrees (0-360)
     */
    static double calculate_bearing(double lat1, double lon1, double lat2, double lon2);
};

} // namespace algorithms
} // namespace transitx

#endif // TRANSITX_ALGORITHMS_HAVERSINE_HPP
