// trip_repository.hpp - Trip repository header
#pragma once
#ifndef TRANSITX_REPOSITORIES_TRIP_REPOSITORY_HPP
#define TRANSITX_REPOSITORIES_TRIP_REPOSITORY_HPP

#include <vector>
#include <string>
#include <optional>
#include "transitx/models/trip.hpp"
#include "transitx/database/connection.hpp"

namespace transitx {
namespace repositories {

class TripRepository {
public:
    static TripRepository& get_instance();

    std::vector<models::Trip> find_all();
    std::optional<models::Trip> find_by_id(int id);
    std::vector<models::Trip> find_active_trips();
    std::optional<models::Trip> find_active_trip_by_bus(int bus_id);
    std::optional<models::Trip> find_active_trip_by_driver(int driver_id);
    bool create(models::Trip& trip);
    bool update_status(int id, const std::string& status);
    bool end_trip(int id);
    bool remove(int id);
};

} // namespace repositories
} // namespace transitx

#endif // TRANSITX_REPOSITORIES_TRIP_REPOSITORY_HPP
