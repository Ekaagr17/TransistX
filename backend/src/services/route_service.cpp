// route_service.cpp - Route service implementation
#include "transitx/services/route_service.hpp"
#include "transitx/repositories/route_repository.hpp"

namespace transitx {
namespace services {

RouteService& RouteService::get_instance() {
    static RouteService instance;
    return instance;
}

std::vector<models::Route> RouteService::get_all_routes() {
    return repositories::RouteRepository::get_instance().find_all();
}

std::optional<models::Route> RouteService::get_route_by_id(int id) {
    return repositories::RouteRepository::get_instance().find_by_id(id);
}

std::optional<models::Route> RouteService::get_route_by_code(const std::string& code) {
    return repositories::RouteRepository::get_instance().find_by_code(code);
}

std::vector<models::RouteStop> RouteService::get_route_stops(int route_id) {
    return repositories::RouteRepository::get_instance().get_route_stops(route_id);
}

bool RouteService::create_route(models::Route& route) {
    return repositories::RouteRepository::get_instance().create(route);
}

bool RouteService::update_route(const models::Route& route) {
    return repositories::RouteRepository::get_instance().update(route);
}

} // namespace services
} // namespace transitx
