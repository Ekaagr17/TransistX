// stop_controller.cpp - Stop endpoints implementation
#include "transitx/controllers/stop_controller.hpp"
#include "transitx/services/stop_service.hpp"
#include "json.hpp"

namespace transitx {
namespace controllers {

void StopController::register_routes(crow::SimpleApp& app) {
    // GET /api/stops/nearby?lat=&lng=&radius=
    CROW_ROUTE(app, "/api/stops/nearby").methods(crow::HTTPMethod::Get)([](const crow::request& req) {
        const char* lat_str = req.url_params.get("lat");
        const char* lng_str = req.url_params.get("lng");
        const char* radius_str = req.url_params.get("radius");

        if (!lat_str || !lng_str) {
            return crow::response(400, nlohmann::json{
                {"error", "Bad Request"},
                {"message", "lat and lng query parameters are required"},
                {"status", 400}
            });
        }

        try {
            double lat = std::stod(lat_str);
            double lng = std::stod(lng_str);
            double radius = radius_str ? std::stod(radius_str) : 1000.0; // default 1000m

            auto nearby = services::StopService::get_instance().find_nearby_stops(lat, lng, radius);
            nlohmann::json arr = nlohmann::json::array();
            for (const auto& s : nearby) {
                arr.push_back(s.to_json());
            }
            return crow::response(200, arr);
        } catch (const std::exception& e) {
            return crow::response(400, nlohmann::json{{"error", "Bad Request"}, {"message", e.what()}, {"status", 400}});
        }
    });

    // GET /api/stops
    CROW_ROUTE(app, "/api/stops").methods(crow::HTTPMethod::Get)([](const crow::request& /*req*/) {
        auto stops = services::StopService::get_instance().get_all_stops();
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& s : stops) {
            arr.push_back(s.to_json());
        }
        return crow::response(200, arr);
    });

    // GET /api/stops/<int>
    CROW_ROUTE(app, "/api/stops/<int>").methods(crow::HTTPMethod::Get)([](const crow::request& /*req*/, int stop_id) {
        auto stop_opt = services::StopService::get_instance().get_stop_by_id(stop_id);
        if (!stop_opt.has_value()) {
            return crow::response(404, nlohmann::json{{"error", "Not Found"}, {"message", "Stop not found"}, {"status", 404}});
        }
        return crow::response(200, stop_opt->to_json());
    });
}

} // namespace controllers
} // namespace transitx
