// trip_service.cpp - Trip service implementation
#include "transitx/services/trip_service.hpp"
#include "transitx/repositories/trip_repository.hpp"
#include "transitx/repositories/bus_repository.hpp"

namespace transitx {
namespace services {

TripService& TripService::get_instance() {
    static TripService instance;
    return instance;
}

std::vector<models::Trip> TripService::get_active_trips() {
    return repositories::TripRepository::get_instance().find_active_trips();
}

std::optional<models::Trip> TripService::get_trip_by_id(int id) {
    return repositories::TripRepository::get_instance().find_by_id(id);
}

std::optional<models::Trip> TripService::get_active_trip_for_driver(int driver_id) {
    return repositories::TripRepository::get_instance().find_active_trip_by_driver(driver_id);
}

std::optional<models::Trip> TripService::start_trip(int bus_id, int route_id, int driver_id) {
    models::Trip trip;
    trip.bus_id = bus_id;
    trip.route_id = route_id;
    trip.driver_id = driver_id;
    trip.status = "in_progress";

    if (repositories::TripRepository::get_instance().create(trip)) {
        repositories::BusRepository::get_instance().update_status(bus_id, "active");
        return trip;
    }
    return std::nullopt;
}

bool TripService::pause_trip(int trip_id) {
    return repositories::TripRepository::get_instance().update_status(trip_id, "paused");
}

bool TripService::resume_trip(int trip_id) {
    return repositories::TripRepository::get_instance().update_status(trip_id, "in_progress");
}

bool TripService::end_trip(int trip_id) {
    auto trip_opt = repositories::TripRepository::get_instance().find_by_id(trip_id);
    if (trip_opt.has_value()) {
        repositories::BusRepository::get_instance().update_status(trip_opt->bus_id, "inactive");
    }
    return repositories::TripRepository::get_instance().end_trip(trip_id);
}

} // namespace services
} // namespace transitx
