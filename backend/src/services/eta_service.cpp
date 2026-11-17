// eta_service.cpp - ETA service implementation
#include "transitx/services/eta_service.hpp"
#include "transitx/repositories/gps_repository.hpp"
#include "transitx/repositories/trip_repository.hpp"
#include "transitx/repositories/route_repository.hpp"
#include "transitx/repositories/stop_repository.hpp"
#include "transitx/algorithms/eta.hpp"

namespace transitx {
namespace services {

EtaService& EtaService::get_instance() {
    static EtaService instance;
    return instance;
}

std::optional<algorithms::EtaResult> EtaService::calculate_eta_for_bus(int bus_id, int stop_id) {
    // 1. Get bus latest location
    auto loc_opt = repositories::GpsRepository::get_instance().get_latest_location(bus_id);
    if (!loc_opt.has_value()) {
        return std::nullopt;
    }
    const auto& loc = loc_opt.value();

    // 2. Determine route id from active trip or location
    int route_id = loc.route_id;
    if (route_id == 0) {
        auto trip_opt = repositories::TripRepository::get_instance().find_active_trip_by_bus(bus_id);
        if (trip_opt.has_value()) {
            route_id = trip_opt->route_id;
        } else {
            route_id = 1; // Default to route 1 for demo
        }
    }

    // 3. Get ordered stops for this route
    auto route_stops = repositories::RouteRepository::get_instance().get_route_stops(route_id);
    if (route_stops.empty()) {
        return std::nullopt;
    }

    // Convert route_stops to RoutePoints for algorithm
    std::vector<algorithms::RoutePoint> points;
    points.reserve(route_stops.size());
    for (const auto& rs : route_stops) {
        algorithms::RoutePoint rp;
        rp.stop_id = rs.stop_id;
        rp.stop_name = rs.stop_name;
        rp.latitude = rs.latitude;
        rp.longitude = rs.longitude;
        rp.stop_order = rs.stop_order;
        rp.estimated_stop_duration = rs.estimated_stop_duration;
        points.push_back(rp);
    }

    // 4. Calculate ETA
    algorithms::EtaResult eta = algorithms::EtaCalculator::calculate_eta(
        loc.latitude, loc.longitude,
        loc.speed, loc.updated_at,
        points, stop_id
    );

    return eta;
}

} // namespace services
} // namespace transitx
