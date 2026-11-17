// route_repository.hpp - Route repository header
#pragma once
#ifndef TRANSITX_REPOSITORIES_ROUTE_REPOSITORY_HPP
#define TRANSITX_REPOSITORIES_ROUTE_REPOSITORY_HPP

#include <vector>
#include <string>
#include <optional>
#include "transitx/models/route.hpp"
#include "transitx/models/route_stop.hpp"
#include "transitx/database/connection.hpp"

namespace transitx {
namespace repositories {

class RouteRepository {
public:
    static RouteRepository& get_instance();

    std::vector<models::Route> find_all();
    std::optional<models::Route> find_by_id(int id);
    std::optional<models::Route> find_by_code(const std::string& code);
    std::vector<models::RouteStop> get_route_stops(int route_id);
    bool create(models::Route& route);
    bool update(const models::Route& route);
    bool remove(int id);
    bool add_stop_to_route(int route_id, int stop_id, int stop_order, int estimated_stop_duration = 30);
};

} // namespace repositories
} // namespace transitx

#endif // TRANSITX_REPOSITORIES_ROUTE_REPOSITORY_HPP
