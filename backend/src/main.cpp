// main.cpp - TransitX Backend Entry Point
#include <iostream>
#include <csignal>
#include "crow_all.h"
#include "transitx/utils/config.hpp"
#include "transitx/utils/logger.hpp"
#include "transitx/database/connection.hpp"
#include "transitx/websocket/live_handler.hpp"

// Controllers
#include "transitx/controllers/auth_controller.hpp"
#include "transitx/controllers/bus_controller.hpp"
#include "transitx/controllers/route_controller.hpp"
#include "transitx/controllers/stop_controller.hpp"
#include "transitx/controllers/gps_controller.hpp"
#include "transitx/controllers/driver_controller.hpp"
#include "transitx/controllers/favorite_controller.hpp"
#include "transitx/controllers/alert_controller.hpp"

using namespace transitx;

int main() {
    LOG_INFO("====================================================");
    LOG_INFO("         TransitX Real-Time Bus Tracking Backend    ");
    LOG_INFO("====================================================");

    // 1. Initialize PostgreSQL Connection Pool
    try {
        database::DatabasePool::get_instance().initialize(
            utils::Config::db_host(),
            utils::Config::db_port(),
            utils::Config::db_name(),
            utils::Config::db_user(),
            utils::Config::db_password(),
            utils::Config::db_pool_size()
        );
    } catch (const std::exception& e) {
        LOG_WARN("Database pool initialization warning: " << e.what());
    }

    // 2. Initialize Crow Application
    crow::SimpleApp app;
    int port = utils::Config::server_port();
    app.port(static_cast<uint16_t>(port));

    // 3. Health & Discovery Endpoints
    CROW_ROUTE(app, "/")
    .methods(crow::HTTPMethod::Get)
    ([](const crow::request& /*req*/) {
        nlohmann::json info = nlohmann::json::object();
        info["name"] = "TransitX Backend API";
        info["version"] = "1.0.0";
        info["status"] = "operational";
        info["location"] = "Dehradun, Uttarakhand, India";
        info["websocket_stream"] = "/ws/live-buses";
        return crow::response(200, info);
    });

    CROW_ROUTE(app, "/api/health")
    .methods(crow::HTTPMethod::Get)
    ([](const crow::request& /*req*/) {
        bool db_healthy = database::DatabasePool::get_instance().is_healthy();
        nlohmann::json health = {
            {"status", "healthy"},
            {"database", db_healthy ? "connected" : "fallback_mode"},
            {"timestamp", utils::current_timestamp()},
            {"websocket_clients", websocket::LiveWebSocketHandler::get_instance().get_connected_clients_count()}
        };
        return crow::response(200, health);
    });

    // 4. WebSocket Route for Real-time GPS Streaming
    CROW_WEBSOCKET_ROUTE(app, "/ws/live-buses")
    .onopen([&](crow::websocket::connection& conn) {
        websocket::LiveWebSocketHandler::get_instance().on_open(conn);
    })
    .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
        websocket::LiveWebSocketHandler::get_instance().on_message(conn, data, is_binary);
    })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
        websocket::LiveWebSocketHandler::get_instance().on_close(conn, reason);
    })
    .onerror([&](crow::websocket::connection& conn) {
        websocket::LiveWebSocketHandler::get_instance().on_error(conn);
    });

    // 5. Register All Controllers
    controllers::AuthController::register_routes(app);
    controllers::BusController::register_routes(app);
    controllers::RouteController::register_routes(app);
    controllers::StopController::register_routes(app);
    controllers::GpsController::register_routes(app);
    controllers::DriverController::register_routes(app);
    controllers::FavoriteController::register_routes(app);
    controllers::AlertController::register_routes(app);

    LOG_INFO("TransitX API routes successfully configured.");
    LOG_INFO("WebSocket live stream listening at ws://0.0.0.0:" << port << "/ws/live-buses");

    // 6. Start Server
    app.multithreaded().run();

    return 0;
}
