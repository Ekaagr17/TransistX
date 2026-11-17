// route_matcher.hpp - Map matching for bus locations to route geometries
#pragma once
#ifndef TRANSITX_ALGORITHMS_ROUTE_MATCHER_HPP
#define TRANSITX_ALGORITHMS_ROUTE_MATCHER_HPP

#include <vector>
#include <string>
#include "transitx/algorithms/haversine.hpp"

namespace transitx {
namespace algorithms {

struct RoutePoint {
    int stop_id;
    std::string stop_name;
    double latitude;
    double longitude;
    int stop_order;
    int estimated_stop_duration = 30; // seconds
};

struct MatchResult {
    bool is_matched = false;
    int current_segment_index = -1;       // Segment between stop[i] and stop[i+1]
    int next_stop_index = -1;             // Index of next stop to reach
    int next_stop_id = -1;
    double distance_to_route_meters = 0.0; // Distance off-route
    double progress_along_segment = 0.0;   // 0.0 to 1.0 along current segment
    double matched_latitude = 0.0;
    double matched_longitude = 0.0;
};

class RouteMatcher {
public:
    /**
     * Match a given GPS point to the closest route segment and determine upcoming stops.
     * @param bus_lat Current bus latitude
     * @param bus_lon Current bus longitude
     * @param stops Ordered list of stops defining the route
     * @param max_off_route_meters Threshold distance before considering bus off-route (default 500m)
     */
    static MatchResult match_to_route(double bus_lat, double bus_lon,
                                      const std::vector<RoutePoint>& stops,
                                      double max_off_route_meters = 500.0);

    /**
     * Compute remaining distance along route from matched location to a target stop.
     */
    static double distance_to_target_stop(const MatchResult& match,
                                         const std::vector<RoutePoint>& stops,
                                         int target_stop_id);
};

} // namespace algorithms
} // namespace transitx

#endif // TRANSITX_ALGORITHMS_ROUTE_MATCHER_HPP
