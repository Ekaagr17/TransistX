// trip_service.hpp - Trip management service header
#pragma once
#ifndef TRANSITX_SERVICES_TRIP_SERVICE_HPP
#define TRANSITX_SERVICES_TRIP_SERVICE_HPP

#include <vector>
#include <optional>
#include "transitx/models/trip.hpp"

namespace transitx {
namespace services {

class TripService {
public:
    static TripService& get_instance();

    std::vector<models::Trip> get_active_trips();
    std::optional<models::Trip> get_trip_by_id(int id);
    std::optional<models::Trip> get_active_trip_for_driver(int driver_id);
    std::optional<models::Trip> start_trip(int bus_id, int route_id, int driver_id);
    bool pause_trip(int trip_id);
    bool resume_trip(int trip_id);
    bool end_trip(int trip_id);
};

} // namespace services
} // namespace transitx

#endif // TRANSITX_SERVICES_TRIP_SERVICE_HPP
