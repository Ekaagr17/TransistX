// gps_repository.hpp - GPS repository header
#pragma once
#ifndef TRANSITX_REPOSITORIES_GPS_REPOSITORY_HPP
#define TRANSITX_REPOSITORIES_GPS_REPOSITORY_HPP

#include <vector>
#include <string>
#include <optional>
#include "transitx/models/gps.hpp"
#include "transitx/database/connection.hpp"

namespace transitx {
namespace repositories {

class GpsRepository {
public:
    static GpsRepository& get_instance();

    bool insert_update(models::GpsUpdate& update);
    std::optional<models::BusLatestLocation> get_latest_location(int bus_id);
    std::vector<models::BusLatestLocation> get_all_latest_locations();
    bool upsert_latest_location(const models::BusLatestLocation& loc);
    std::vector<models::GpsUpdate> get_updates_by_trip(int trip_id, int limit = 100);
};

} // namespace repositories
} // namespace transitx

#endif // TRANSITX_REPOSITORIES_GPS_REPOSITORY_HPP
