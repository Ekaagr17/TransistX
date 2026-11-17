// route_controller.cpp - Route endpoints implementation
#include "transitx/controllers/route_controller.hpp"
#include "transitx/services/route_service.hpp"
#include "json.hpp"

namespace transitx {
namespace controllers {

void RouteController::register_routes(crow::SimpleApp& app) {
    // GET /api/routes
    CROW_ROUTE(app, "/api/routes").methods(crow::HTTPMethod::Get)([](const crow::request& /*req*/) {
        auto routes = services::RouteService::get_instance().get_all_routes();
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& r : routes) {
            arr.push_back(r.to_json());
        }
        return crow::response(200, arr);
    });

    // GET /api/routes/<int>
    CROW_ROUTE(app, "/api/routes/<int>").methods(crow::HTTPMethod::Get)([](const crow::request& /*req*/, int route_id) {
        auto route_opt = services::RouteService::get_instance().get_route_by_id(route_id);
        if (!route_opt.has_value()) {
            return crow::response(404, nlohmann::json{{"error", "Not Found"}, {"message", "Route not found"}, {"status", 404}});
        }
        return crow::response(200, route_opt->to_json());
    });

    // GET /api/routes/<int>/stops
    CROW_ROUTE(app, "/api/routes/<int>/stops").methods(crow::HTTPMethod::Get)([](const crow::request& /*req*/, int route_id) {
        auto stops = services::RouteService::get_instance().get_route_stops(route_id);
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& s : stops) {
            arr.push_back(s.to_json());
        }
        return crow::response(200, arr);
    });
}

} // namespace controllers
} // namespace transitx
