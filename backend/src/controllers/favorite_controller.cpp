// favorite_controller.cpp - User favorites implementation
#include "transitx/controllers/favorite_controller.hpp"
#include "transitx/services/favorite_service.hpp"
#include "transitx/middleware/auth_middleware.hpp"
#include "json.hpp"

namespace transitx {
namespace controllers {

void FavoriteController::register_routes(crow::SimpleApp& app) {
    // GET /api/favorites
    CROW_ROUTE(app, "/api/favorites").methods(crow::HTTPMethod::Get)([](const crow::request& req) {
        auto user_opt = middleware::AuthMiddleware::authenticate(req);
        if (!user_opt.has_value()) {
            return middleware::AuthMiddleware::unauthorized_response("Sign in to view saved favorites");
        }

        auto favs = services::FavoriteService::get_instance().get_user_favorites(user_opt->id);
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& f : favs) {
            arr.push_back(f.to_json());
        }
        return crow::response(200, arr);
    });

    // POST /api/favorites
    CROW_ROUTE(app, "/api/favorites").methods(crow::HTTPMethod::Post)([](const crow::request& req) {
        auto user_opt = middleware::AuthMiddleware::authenticate(req);
        if (!user_opt.has_value()) {
            return middleware::AuthMiddleware::unauthorized_response();
        }

        try {
            nlohmann::json body = nlohmann::json::parse(req.body);
            int route_id = body.value("route_id", 0);
            if (route_id <= 0) {
                return crow::response(400, nlohmann::json{{"error", "route_id is required"}});
            }

            if (services::FavoriteService::get_instance().add_favorite(user_opt->id, route_id)) {
                return crow::response(201, nlohmann::json{{"message", "Route added to favorites"}, {"route_id", route_id}});
            }
            return crow::response(400, nlohmann::json{{"error", "Failed to add favorite"}});
        } catch (const std::exception& e) {
            return crow::response(400, nlohmann::json{{"error", e.what()}});
        }
    });

    // DELETE /api/favorites/<int>
    CROW_ROUTE(app, "/api/favorites/<int>").methods(crow::HTTPMethod::Delete)([](const crow::request& req, int route_id) {
        auto user_opt = middleware::AuthMiddleware::authenticate(req);
        if (!user_opt.has_value()) {
            return middleware::AuthMiddleware::unauthorized_response();
        }

        if (services::FavoriteService::get_instance().remove_favorite(user_opt->id, route_id)) {
            return crow::response(200, nlohmann::json{{"message", "Route removed from favorites"}, {"route_id", route_id}});
        }
        return crow::response(404, nlohmann::json{{"error", "Favorite not found or already removed"}});
    });
}

} // namespace controllers
} // namespace transitx
