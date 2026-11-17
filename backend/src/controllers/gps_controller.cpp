// gps_controller.cpp - GPS endpoints implementation
#include "transitx/controllers/gps_controller.hpp"
#include "transitx/services/gps_service.hpp"
#include "transitx/middleware/auth_middleware.hpp"
#include "transitx/middleware/role_middleware.hpp"
#include "json.hpp"

namespace transitx {
namespace controllers {

void GpsController::register_routes(crow::SimpleApp& app) {
    // POST /api/gps/update - Authenticated drivers only
    CROW_ROUTE(app, "/api/gps/update").methods(crow::HTTPMethod::Post)([](const crow::request& req) {
        auto user_opt = middleware::AuthMiddleware::authenticate(req);
        if (!user_opt.has_value()) {
            return middleware::AuthMiddleware::unauthorized_response("Authentication token required to submit GPS data");
        }

        if (!middleware::RoleMiddleware::has_any_role(*user_opt, {"driver", "admin"})) {
            return middleware::RoleMiddleware::forbidden_response("Only drivers or administrators can submit GPS updates");
        }

        try {
            nlohmann::json body = nlohmann::json::parse(req.body);
            int bus_id = body.value("bus_id", 0);
            int trip_id = body.value("trip_id", 0);
            double lat = body.value("latitude", 0.0);
            double lng = body.value("longitude", 0.0);
            double speed = body.value("speed", 0.0);
            double heading = body.value("heading", 0.0);
            double accuracy = body.value("accuracy", 10.0);

            if (bus_id <= 0) {
                return crow::response(400, nlohmann::json{{"error", "Bad Request"}, {"message", "bus_id is required"}, {"status", 400}});
            }

            auto res = services::GpsService::get_instance().process_update(bus_id, trip_id, lat, lng, speed, heading, accuracy);
            if (!res.success) {
                return crow::response(422, nlohmann::json{
                    {"error", "Unprocessable Entity"},
                    {"message", res.error_message},
                    {"status", 422}
                });
            }

            return crow::response(200, nlohmann::json{
                {"status", "success"},
                {"message", "GPS update received and broadcast"},
                {"update", res.update.to_json()}
            });
        } catch (const std::exception& e) {
            return crow::response(400, nlohmann::json{{"error", "Bad Request"}, {"message", e.what()}, {"status", 400}});
        }
    });

    // GET /api/gps/status
    CROW_ROUTE(app, "/api/gps/status").methods(crow::HTTPMethod::Get)([](const crow::request& /*req*/) {
        auto status = services::GpsService::get_instance().get_gps_status();
        return crow::response(200, status);
    });
}

} // namespace controllers
} // namespace transitx
