// eta.cpp - ETA calculation engine implementation
#include "transitx/algorithms/eta.hpp"
#include "transitx/algorithms/haversine.hpp"
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace transitx {
namespace algorithms {

EtaResult EtaCalculator::calculate_eta(
    double bus_lat, double bus_lon,
    double current_speed_kmh,
    const std::string& recorded_at,
    const std::vector<RoutePoint>& stops,
    int target_stop_id
) {
    EtaResult result;
    result.last_gps_update = recorded_at;
    result.target_stop_id = target_stop_id;

    // Find stop name
    for (const auto& sp : stops) {
        if (sp.stop_id == target_stop_id) {
            result.target_stop_name = sp.stop_name;
            break;
        }
    }

    if (stops.empty()) {
        result.confidence = 0.0;
        return result;
    }

    // Match bus to route
    MatchResult match = RouteMatcher::match_to_route(bus_lat, bus_lon, stops);

    // Calculate remaining distance along route
    double dist_km = RouteMatcher::distance_to_target_stop(match, stops, target_stop_id);
    result.remaining_distance_km = std::round(dist_km * 100.0) / 100.0;

    // Determine realistic speed
    double effective_speed = current_speed_kmh;
    if (effective_speed < MIN_SPEED_KMH) {
        effective_speed = DEFAULT_SPEED_KMH;
    } else if (effective_speed > MAX_SPEED_KMH) {
        effective_speed = MAX_SPEED_KMH;
    }

    // Blend: 70% current speed (if valid) + 30% default urban speed
    if (current_speed_kmh >= MIN_SPEED_KMH && current_speed_kmh <= MAX_SPEED_KMH) {
        effective_speed = 0.7 * current_speed_kmh + 0.3 * DEFAULT_SPEED_KMH;
    }

    // Calculate transit time in hours -> minutes
    double travel_hours = dist_km / effective_speed;
    double travel_minutes = travel_hours * 60.0;

    // Add dwell times for intermediate stops
    int target_idx = -1;
    for (size_t i = 0; i < stops.size(); ++i) {
        if (stops[i].stop_id == target_stop_id) {
            target_idx = static_cast<int>(i);
            break;
        }
    }

    int intermediate_stops = 0;
    if (match.next_stop_index >= 0 && target_idx > match.next_stop_index) {
        intermediate_stops = target_idx - match.next_stop_index;
    }
    double dwell_minutes = (intermediate_stops * DEFAULT_STOP_DWELL_SECONDS) / 60.0;

    result.estimated_duration_minutes = std::round((travel_minutes + dwell_minutes) * 10.0) / 10.0;
    if (result.estimated_duration_minutes < 1.0) result.estimated_duration_minutes = 1.0;

    // Confidence metric: decreases if off route or zero GPS
    double conf = 1.0;
    if (match.distance_to_route_meters > 100.0) {
        conf -= std::min(0.5, (match.distance_to_route_meters - 100.0) / 400.0 * 0.5);
    }
    result.confidence = std::max(0.2, std::min(1.0, std::round(conf * 100.0) / 100.0));

    // Calculate estimated arrival time
    std::time_t now = std::time(nullptr);
    std::time_t arrival_time = now + static_cast<std::time_t>(result.estimated_duration_minutes * 60.0);
    std::tm* tm_buf = std::localtime(&arrival_time);
    char time_str[32] = "";
    if (tm_buf) {
        std::strftime(time_str, sizeof(time_str), "%H:%M", tm_buf);
    }
    result.estimated_arrival = std::string(time_str);

    return result;
}

} // namespace algorithms
} // namespace transitx
