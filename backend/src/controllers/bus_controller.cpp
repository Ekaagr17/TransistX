// bus_controller.cpp - Bus endpoints implementation
#include "transitx/controllers/bus_controller.hpp"
#include "transitx/services/bus_service.hpp"
#include "json.hpp"

namespace transitx {
namespace controllers {

void BusController::register_routes(crow::SimpleApp& app) {
    // GET /api/buses
    CROW_ROUTE(app, "/api/buses").methods(crow::HTTPMethod::Get)([](const crow::request& /*req*/) {
        auto buses = services::BusService::get_instance().get_all_buses();
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& b : buses) {
            arr.push_back(b.to_json());
        }
        return crow::response(200, arr);
    });

    // GET /api/buses/<int>
    CROW_ROUTE(app, "/api/buses/<int>").methods(crow::HTTPMethod::Get)([](const crow::request& /*req*/, int bus_id) {
        auto bus_opt = services::BusService::get_instance().get_bus_by_id(bus_id);
        if (!bus_opt.has_value()) {
            return crow::response(404, nlohmann::json{{"error", "Not Found"}, {"message", "Bus not found"}, {"status", 404}});
        }
        return crow::response(200, bus_opt->to_json());
    });

    // GET /api/buses/<int>/location
    CROW_ROUTE(app, "/api/buses/<int>/location").methods(crow::HTTPMethod::Get)([](const crow::request& /*req*/, int bus_id) {
        auto loc_opt = services::BusService::get_instance().get_bus_location(bus_id);
        if (!loc_opt.has_value()) {
            return crow::response(404, nlohmann::json{{"error", "Not Found"}, {"message", "No recent location for this bus"}, {"status", 404}});
        }
        return crow::response(200, loc_opt->to_json());
    });

    // GET /api/buses/<int>/eta/<int>
    CROW_ROUTE(app, "/api/buses/<int>/eta/<int>").methods(crow::HTTPMethod::Get)([](const crow::request& /*req*/, int bus_id, int stop_id) {
        auto eta_opt = services::BusService::get_instance().get_bus_eta(bus_id, stop_id);
        if (!eta_opt.has_value()) {
            return crow::response(404, nlohmann::json{
                {"error", "Not Found"},
                {"message", "Could not calculate ETA: bus location or route stops unavailable"},
                {"status", 404}
            });
        }
        return crow::response(200, eta_opt->to_json());
    });
}

} // namespace controllers
} // namespace transitx
