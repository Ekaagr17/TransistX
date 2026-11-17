// eta.hpp - ETA calculation engine for TransitX
#pragma once
#ifndef TRANSITX_ALGORITHMS_ETA_HPP
#define TRANSITX_ALGORITHMS_ETA_HPP

#include <string>
#include <vector>
#include <ctime>
#include "transitx/algorithms/route_matcher.hpp"
#include "json.hpp"

namespace transitx {
namespace algorithms {

struct EtaResult {
    std::string estimated_arrival;       // ISO 8601 or HH:MM format
    double remaining_distance_km = 0.0;
    double estimated_duration_minutes = 0.0;
    double confidence = 1.0;             // 0.0 to 1.0 based on freshness & off-route
    std::string last_gps_update;
    int target_stop_id = 0;
    std::string target_stop_name;

    nlohmann::json to_json() const {
        return nlohmann::json{
            {"estimated_arrival", estimated_arrival},
            {"remaining_distance_km", remaining_distance_km},
            {"estimated_duration_minutes", estimated_duration_minutes},
            {"confidence", confidence},
            {"last_gps_update", last_gps_update},
            {"target_stop_id", target_stop_id},
            {"target_stop_name", target_stop_name}
        };
    }
};

class EtaCalculator {
public:
    static constexpr double MIN_SPEED_KMH = 5.0;   // In heavy traffic or boarding
    static constexpr double MAX_SPEED_KMH = 80.0;  // Urban speed cap
    static constexpr double DEFAULT_SPEED_KMH = 25.0; // Typical urban bus speed
    static constexpr int DEFAULT_STOP_DWELL_SECONDS = 30; // Boarding/alighting time per stop

    /**
     * Calculate ETA to a specific stop along the route.
     * @param bus_lat Current bus latitude
     * @param bus_lon Current bus longitude
     * @param current_speed_kmh Bus speed from GPS
     * @param recorded_at Timestamp of the GPS fix
     * @param stops List of stops on this route in order
     * @param target_stop_id Destination stop ID
     */
    static EtaResult calculate_eta(
        double bus_lat, double bus_lon,
        double current_speed_kmh,
        const std::string& recorded_at,
        const std::vector<RoutePoint>& stops,
        int target_stop_id
    );
};

} // namespace algorithms
} // namespace transitx

#endif // TRANSITX_ALGORITHMS_ETA_HPP
