// driver_controller.cpp - Driver dashboard implementation
#include "transitx/controllers/driver_controller.hpp"
#include "transitx/services/trip_service.hpp"
#include "transitx/services/bus_service.hpp"
#include "transitx/services/route_service.hpp"
#include "transitx/middleware/auth_middleware.hpp"
#include "transitx/middleware/role_middleware.hpp"
#include "json.hpp"

namespace transitx {
namespace controllers {

void DriverController::register_routes(crow::SimpleApp& app) {
    // GET /api/driver/dashboard
    CROW_ROUTE(app, "/api/driver/dashboard").methods(crow::HTTPMethod::Get)([](const crow::request& req) {
        auto user_opt = middleware::AuthMiddleware::authenticate(req);
        if (!user_opt.has_value()) {
            return middleware::AuthMiddleware::unauthorized_response("Authentication token required");
        }

        if (!middleware::RoleMiddleware::has_any_role(*user_opt, {"driver", "admin"})) {
            return middleware::RoleMiddleware::forbidden_response("Driver access only");
        }

        int driver_id = user_opt->id; // Using user_id as driver_id for demo
        auto active_trip = services::TripService::get_instance().get_active_trip_for_driver(driver_id);

        nlohmann::json resp = {
            {"driver", user_opt->to_json()},
            {"has_active_trip", active_trip.has_value()},
            {"active_trip", active_trip.has_value() ? active_trip->to_json() : nlohmann::json(nullptr)},
            {"available_buses", nlohmann::json::array()},
            {"available_routes", nlohmann::json::array()}
        };

        auto buses = services::BusService::get_instance().get_all_buses();
        for (const auto& b : buses) {
            resp["available_buses"].push_back(b.to_json());
        }

        auto routes = services::RouteService::get_instance().get_all_routes();
        for (const auto& r : routes) {
            resp["available_routes"].push_back(r.to_json());
        }

        return crow::response(200, resp);
    });

    // POST /api/driver/trips/start
    CROW_ROUTE(app, "/api/driver/trips/start").methods(crow::HTTPMethod::Post)([](const crow::request& req) {
        auto user_opt = middleware::AuthMiddleware::authenticate(req);
        if (!user_opt.has_value()) {
            return middleware::AuthMiddleware::unauthorized_response();
        }
        if (!middleware::RoleMiddleware::has_any_role(*user_opt, {"driver", "admin"})) {
            return middleware::RoleMiddleware::forbidden_response();
        }

        try {
            nlohmann::json body = nlohmann::json::parse(req.body);
            int bus_id = body.value("bus_id", 1);
            int route_id = body.value("route_id", 1);
            int driver_id = user_opt->id;

            auto trip_opt = services::TripService::get_instance().start_trip(bus_id, route_id, driver_id);
            if (!trip_opt.has_value()) {
                return crow::response(400, nlohmann::json{{"error", "Failed to start trip"}, {"status", 400}});
            }

            return crow::response(201, nlohmann::json{
                {"message", "Trip started successfully"},
                {"trip", trip_opt->to_json()}
            });
        } catch (const std::exception& e) {
            return crow::response(400, nlohmann::json{{"error", "Bad Request"}, {"message", e.what()}, {"status", 400}});
        }
    });

    // POST /api/driver/trips/pause
    CROW_ROUTE(app, "/api/driver/trips/pause").methods(crow::HTTPMethod::Post)([](const crow::request& req) {
        auto user_opt = middleware::AuthMiddleware::authenticate(req);
        if (!user_opt.has_value()) return middleware::AuthMiddleware::unauthorized_response();

        try {
            nlohmann::json body = nlohmann::json::parse(req.body);
            int trip_id = body.value("trip_id", 0);
            if (trip_id <= 0) return crow::response(400, nlohmann::json{{"error", "trip_id required"}});

            if (services::TripService::get_instance().pause_trip(trip_id)) {
                return crow::response(200, nlohmann::json{{"message", "Trip paused successfully"}, {"status", "paused"}});
            }
            return crow::response(400, nlohmann::json{{"error", "Failed to pause trip"}});
        } catch (const std::exception& e) {
            return crow::response(400, nlohmann::json{{"error", e.what()}});
        }
    });

    // POST /api/driver/trips/resume
    CROW_ROUTE(app, "/api/driver/trips/resume").methods(crow::HTTPMethod::Post)([](const crow::request& req) {
        auto user_opt = middleware::AuthMiddleware::authenticate(req);
        if (!user_opt.has_value()) return middleware::AuthMiddleware::unauthorized_response();

        try {
            nlohmann::json body = nlohmann::json::parse(req.body);
            int trip_id = body.value("trip_id", 0);
            if (trip_id <= 0) return crow::response(400, nlohmann::json{{"error", "trip_id required"}});

            if (services::TripService::get_instance().resume_trip(trip_id)) {
                return crow::response(200, nlohmann::json{{"message", "Trip resumed successfully"}, {"status", "in_progress"}});
            }
            return crow::response(400, nlohmann::json{{"error", "Failed to resume trip"}});
        } catch (const std::exception& e) {
            return crow::response(400, nlohmann::json{{"error", e.what()}});
        }
    });

    // POST /api/driver/trips/end
    CROW_ROUTE(app, "/api/driver/trips/end").methods(crow::HTTPMethod::Post)([](const crow::request& req) {
        auto user_opt = middleware::AuthMiddleware::authenticate(req);
        if (!user_opt.has_value()) return middleware::AuthMiddleware::unauthorized_response();

        try {
            nlohmann::json body = nlohmann::json::parse(req.body);
            int trip_id = body.value("trip_id", 0);
            if (trip_id <= 0) return crow::response(400, nlohmann::json{{"error", "trip_id required"}});

            if (services::TripService::get_instance().end_trip(trip_id)) {
                return crow::response(200, nlohmann::json{{"message", "Trip completed successfully"}, {"status", "completed"}});
            }
            return crow::response(400, nlohmann::json{{"error", "Failed to end trip"}});
        } catch (const std::exception& e) {
            return crow::response(400, nlohmann::json{{"error", e.what()}});
        }
    });
}

} // namespace controllers
} // namespace transitx
