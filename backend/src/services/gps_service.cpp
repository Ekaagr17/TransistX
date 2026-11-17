// gps_service.cpp - GPS service implementation
#include "transitx/services/gps_service.hpp"
#include "transitx/repositories/gps_repository.hpp"
#include "transitx/repositories/bus_repository.hpp"
#include "transitx/repositories/route_repository.hpp"
#include "transitx/repositories/trip_repository.hpp"
#include "transitx/websocket/live_handler.hpp"
#include "transitx/utils/validation.hpp"
#include "transitx/utils/logger.hpp"

namespace transitx {
namespace services {

GpsService& GpsService::get_instance() {
    static GpsService instance;
    return instance;
}

GpsProcessResult GpsService::process_update(
    int bus_id, int trip_id, double lat, double lng, double speed, double heading, double accuracy
) {
    GpsProcessResult res;

    // Validate coordinates
    if (!utils::Validation::is_valid_coordinates(lat, lng)) {
        res.error_message = "Invalid GPS coordinates: latitude must be [-90, 90] and longitude [-180, 180]";
        return res;
    }

    // Validate accuracy (reject GPS drift > 200m)
    if (accuracy > 200.0) {
        res.error_message = "GPS accuracy too poor (>200m), rejected update";
        return res;
    }

    // Validate speed
    if (!utils::Validation::is_valid_speed(speed)) {
        res.error_message = "Invalid speed value";
        return res;
    }

    models::GpsUpdate update;
    update.bus_id = bus_id;
    update.trip_id = trip_id;
    update.latitude = lat;
    update.longitude = lng;
    update.speed = speed;
    update.heading = heading;
    update.accuracy = accuracy;

    // Save to repository
    if (!repositories::GpsRepository::get_instance().insert_update(update)) {
        res.error_message = "Failed to store GPS update";
        return res;
    }

    // Prepare latest location record
    models::BusLatestLocation latest;
    latest.bus_id = bus_id;
    latest.latitude = lat;
    latest.longitude = lng;
    latest.speed = speed;
    latest.heading = heading;
    latest.accuracy = accuracy;
    latest.updated_at = update.recorded_at;

    // Enrich with bus & route info
    auto bus_opt = repositories::BusRepository::get_instance().find_by_id(bus_id);
    if (bus_opt.has_value()) {
        latest.bus_number = bus_opt->bus_number;
        latest.status = bus_opt->status;
    }

    if (trip_id > 0) {
        auto trip_opt = repositories::TripRepository::get_instance().find_by_id(trip_id);
        if (trip_opt.has_value()) {
            latest.route_id = trip_opt->route_id;
            latest.route_name = trip_opt->route_name;
            latest.route_code = trip_opt->route_code;
        }
    }

    // Upsert latest location
    repositories::GpsRepository::get_instance().upsert_latest_location(latest);

    // Broadcast to real-time WebSocket clients
    websocket::LiveWebSocketHandler::get_instance().broadcast_location_update(latest);

    res.success = true;
    res.update = update;
    return res;
}

nlohmann::json GpsService::get_gps_status() {
    auto locations = repositories::GpsRepository::get_instance().get_all_latest_locations();
    nlohmann::json buses_arr = nlohmann::json::array();
    for (const auto& loc : locations) {
        buses_arr.push_back(loc.to_json());
    }

    return nlohmann::json{
        {"status", "online"},
        {"active_buses_count", locations.size()},
        {"websocket_clients_connected", websocket::LiveWebSocketHandler::get_instance().get_connected_clients_count()},
        {"timestamp", utils::current_timestamp()},
        {"buses", buses_arr}
    };
}

std::vector<models::GpsUpdate> GpsService::get_trip_history(int trip_id, int limit) {
    return repositories::GpsRepository::get_instance().get_updates_by_trip(trip_id, limit);
}

} // namespace services
} // namespace transitx
