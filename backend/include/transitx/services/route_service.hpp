// route_service.hpp - Route service header
#pragma once
#ifndef TRANSITX_SERVICES_ROUTE_SERVICE_HPP
#define TRANSITX_SERVICES_ROUTE_SERVICE_HPP

#include <vector>
#include <optional>
#include "transitx/models/route.hpp"
#include "transitx/models/route_stop.hpp"

namespace transitx {
namespace services {

class RouteService {
public:
    static RouteService& get_instance();

    std::vector<models::Route> get_all_routes();
    std::optional<models::Route> get_route_by_id(int id);
    std::optional<models::Route> get_route_by_code(const std::string& code);
    std::vector<models::RouteStop> get_route_stops(int route_id);
    bool create_route(models::Route& route);
    bool update_route(const models::Route& route);
};

} // namespace services
} // namespace transitx

#endif // TRANSITX_SERVICES_ROUTE_SERVICE_HPP
