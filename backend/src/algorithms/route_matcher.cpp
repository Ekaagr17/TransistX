// route_matcher.cpp - Route map matching implementation
#include "transitx/algorithms/route_matcher.hpp"
#include <limits>
#include <algorithm>

namespace transitx {
namespace algorithms {

MatchResult RouteMatcher::match_to_route(double bus_lat, double bus_lon,
                                         const std::vector<RoutePoint>& stops,
                                         double max_off_route_meters) {
    MatchResult result;
    if (stops.empty()) return result;

    if (stops.size() == 1) {
        double dist = Haversine::calculate_distance_meters(bus_lat, bus_lon, stops[0].latitude, stops[0].longitude);
        result.is_matched = (dist <= max_off_route_meters);
        result.current_segment_index = 0;
        result.next_stop_index = 0;
        result.next_stop_id = stops[0].stop_id;
        result.distance_to_route_meters = dist;
        result.matched_latitude = stops[0].latitude;
        result.matched_longitude = stops[0].longitude;
        return result;
    }

    double min_distance = std::numeric_limits<double>::max();
    int best_segment = 0;
    double best_progress = 0.0;
    double best_lat = stops[0].latitude;
    double best_lon = stops[0].longitude;

    for (size_t i = 0; i < stops.size() - 1; ++i) {
        double x1 = stops[i].longitude;
        double y1 = stops[i].latitude;
        double x2 = stops[i + 1].longitude;
        double y2 = stops[i + 1].latitude;

        // Project point (bus_lon, bus_lat) onto line segment (x1, y1) -> (x2, y2)
        double dx = x2 - x1;
        double dy = y2 - y1;
        double seg_len_sq = dx * dx + dy * dy;

        double t = 0.0;
        if (seg_len_sq > 1e-12) {
            t = ((bus_lon - x1) * dx + (bus_lat - y1) * dy) / seg_len_sq;
            t = std::max(0.0, std::min(1.0, t));
        }

        double proj_lon = x1 + t * dx;
        double proj_lat = y1 + t * dy;

        double dist = Haversine::calculate_distance_meters(bus_lat, bus_lon, proj_lat, proj_lon);
        if (dist < min_distance) {
            min_distance = dist;
            best_segment = static_cast<int>(i);
            best_progress = t;
            best_lat = proj_lat;
            best_lon = proj_lon;
        }
    }

    result.is_matched = (min_distance <= max_off_route_meters);
    result.current_segment_index = best_segment;
    result.next_stop_index = best_segment + 1;
    if (result.next_stop_index < static_cast<int>(stops.size())) {
        result.next_stop_id = stops[result.next_stop_index].stop_id;
    }
    result.distance_to_route_meters = min_distance;
    result.progress_along_segment = best_progress;
    result.matched_latitude = best_lat;
    result.matched_longitude = best_lon;

    return result;
}

double RouteMatcher::distance_to_target_stop(const MatchResult& match,
                                            const std::vector<RoutePoint>& stops,
                                            int target_stop_id) {
    if (stops.empty()) return 0.0;

    int target_idx = -1;
    for (size_t i = 0; i < stops.size(); ++i) {
        if (stops[i].stop_id == target_stop_id) {
            target_idx = static_cast<int>(i);
            break;
        }
    }

    if (target_idx == -1) return 0.0;

    // If target stop is behind current matched segment
    if (target_idx <= match.current_segment_index && match.current_segment_index != -1) {
        // Direct distance from matched position
        return Haversine::calculate_distance_km(match.matched_latitude, match.matched_longitude,
                                               stops[target_idx].latitude, stops[target_idx].longitude);
    }

    double total_distance_km = 0.0;

    // Remaining distance on current segment to next stop
    if (match.next_stop_index >= 0 && match.next_stop_index < static_cast<int>(stops.size())) {
        total_distance_km += Haversine::calculate_distance_km(
            match.matched_latitude, match.matched_longitude,
            stops[match.next_stop_index].latitude, stops[match.next_stop_index].longitude
        );

        // Distance along consecutive stops until target_idx
        for (int i = match.next_stop_index; i < target_idx; ++i) {
            total_distance_km += Haversine::calculate_distance_km(
                stops[i].latitude, stops[i].longitude,
                stops[i + 1].latitude, stops[i + 1].longitude
            );
        }
    } else {
        total_distance_km = Haversine::calculate_distance_km(
            match.matched_latitude, match.matched_longitude,
            stops[target_idx].latitude, stops[target_idx].longitude
        );
    }

    return total_distance_km;
}

} // namespace algorithms
} // namespace transitx
