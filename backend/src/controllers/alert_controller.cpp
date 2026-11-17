// alert_controller.cpp - Service alerts implementation
#include "transitx/controllers/alert_controller.hpp"
#include "transitx/services/alert_service.hpp"
#include "transitx/middleware/auth_middleware.hpp"
#include "transitx/middleware/role_middleware.hpp"
#include "json.hpp"

namespace transitx {
namespace controllers {

void AlertController::register_routes(crow::SimpleApp& app) {
    // GET /api/alerts
    CROW_ROUTE(app, "/api/alerts").methods(crow::HTTPMethod::Get)([](const crow::request& req) {
        const char* route_id_str = req.url_params.get("route_id");
        std::vector<models::ServiceAlert> alerts;

        if (route_id_str) {
            try {
                int route_id = std::stoi(route_id_str);
                alerts = services::AlertService::get_instance().get_alerts_by_route(route_id);
            } catch (...) {
                alerts = services::AlertService::get_instance().get_active_alerts();
            }
        } else {
            alerts = services::AlertService::get_instance().get_active_alerts();
        }

        nlohmann::json arr = nlohmann::json::array();
        for (const auto& a : alerts) {
            arr.push_back(a.to_json());
        }
        return crow::response(200, arr);
    });

    // POST /api/admin/alerts - Admin only
    CROW_ROUTE(app, "/api/admin/alerts").methods(crow::HTTPMethod::Post)([](const crow::request& req) {
        auto user_opt = middleware::AuthMiddleware::authenticate(req);
        if (!user_opt.has_value()) {
            return middleware::AuthMiddleware::unauthorized_response();
        }

        if (!middleware::RoleMiddleware::has_role(*user_opt, "admin")) {
            return middleware::RoleMiddleware::forbidden_response("Admin role required to broadcast alerts");
        }

        try {
            nlohmann::json body = nlohmann::json::parse(req.body);
            int route_id = body.value("route_id", 0);
            std::string message = body.value("message", "");
            std::string severity = body.value("severity", "info");

            if (message.empty()) {
                return crow::response(400, nlohmann::json{{"error", "Alert message cannot be empty"}});
            }

            models::ServiceAlert alert;
            alert.route_id = route_id;
            alert.message = message;
            alert.severity = severity;

            if (services::AlertService::get_instance().create_alert(alert)) {
                return crow::response(201, nlohmann::json{
                    {"message", "Service alert created and broadcast to all users"},
                    {"alert", alert.to_json()}
                });
            }

            return crow::response(400, nlohmann::json{{"error", "Failed to create alert"}});
        } catch (const std::exception& e) {
            return crow::response(400, nlohmann::json{{"error", e.what()}});
        }
    });
}

} // namespace controllers
} // namespace transitx
