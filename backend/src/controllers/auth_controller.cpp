// auth_controller.cpp - Authentication endpoints implementation
#include "transitx/controllers/auth_controller.hpp"
#include "transitx/services/auth_service.hpp"
#include "transitx/middleware/auth_middleware.hpp"
#include "transitx/middleware/cors_middleware.hpp"
#include "json.hpp"

namespace transitx {
namespace controllers {

void AuthController::register_routes(crow::SimpleApp& app) {
    // POST /api/auth/register
    CROW_ROUTE(app, "/api/auth/register").methods(crow::HTTPMethod::Post)([](const crow::request& req) {
        try {
            nlohmann::json body = nlohmann::json::parse(req.body);
            std::string name = body.value("name", "");
            std::string email = body.value("email", "");
            std::string password = body.value("password", "");
            std::string role = body.value("role", "passenger");

            auto res = services::AuthService::get_instance().register_user(name, email, password, role);
            if (!res.success) {
                return crow::response(400, nlohmann::json{
                    {"error", "Registration Failed"},
                    {"message", res.error_message},
                    {"status", 400}
                });
            }

            return crow::response(201, nlohmann::json{
                {"message", "User registered successfully"},
                {"token", res.token},
                {"user", res.user.to_json()}
            });
        } catch (const std::exception& e) {
            return crow::response(400, nlohmann::json{{"error", "Bad Request"}, {"message", e.what()}, {"status", 400}});
        }
    });

    // POST /api/auth/login
    CROW_ROUTE(app, "/api/auth/login").methods(crow::HTTPMethod::Post)([](const crow::request& req) {
        try {
            nlohmann::json body = nlohmann::json::parse(req.body);
            std::string email = body.value("email", "");
            std::string password = body.value("password", "");

            auto res = services::AuthService::get_instance().login_user(email, password);
            if (!res.success) {
                return crow::response(401, nlohmann::json{
                    {"error", "Authentication Failed"},
                    {"message", res.error_message},
                    {"status", 401}
                });
            }

            return crow::response(200, nlohmann::json{
                {"message", "Login successful"},
                {"token", res.token},
                {"user", res.user.to_json()}
            });
        } catch (const std::exception& e) {
            return crow::response(400, nlohmann::json{{"error", "Bad Request"}, {"message", e.what()}, {"status", 400}});
        }
    });

    // GET /api/auth/me
    CROW_ROUTE(app, "/api/auth/me").methods(crow::HTTPMethod::Get)([](const crow::request& req) {
        auto user_opt = middleware::AuthMiddleware::authenticate(req);
        if (!user_opt.has_value()) {
            return middleware::AuthMiddleware::unauthorized_response("Valid token required to access profile");
        }

        return crow::response(200, nlohmann::json{
            {"user", user_opt->to_json()}
        });
    });
}

} // namespace controllers
} // namespace transitx
