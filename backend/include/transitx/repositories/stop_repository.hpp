// stop_repository.hpp - Stop repository header
#pragma once
#ifndef TRANSITX_REPOSITORIES_STOP_REPOSITORY_HPP
#define TRANSITX_REPOSITORIES_STOP_REPOSITORY_HPP

#include <vector>
#include <string>
#include <optional>
#include "transitx/models/stop.hpp"
#include "transitx/database/connection.hpp"

namespace transitx {
namespace repositories {

class StopRepository {
public:
    static StopRepository& get_instance();

    std::vector<models::Stop> find_all();
    std::optional<models::Stop> find_by_id(int id);
    /**
     * Find stops within a given radius (in meters) of (lat, lng) using PostGIS ST_DWithin.
     */
    std::vector<models::Stop> find_nearby(double lat, double lng, double radius_meters);
    bool create(models::Stop& stop);
    bool update(const models::Stop& stop);
    bool remove(int id);
};

} // namespace repositories
} // namespace transitx

#endif // TRANSITX_REPOSITORIES_STOP_REPOSITORY_HPP
