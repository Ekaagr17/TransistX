// trip.hpp - Trip model
#pragma once
#ifndef TRANSITX_MODELS_TRIP_HPP
#define TRANSITX_MODELS_TRIP_HPP

#include <string>
#include "json.hpp"

namespace transitx {
namespace models {

struct Trip {
    int id = 0;
    int bus_id = 0;
    int route_id = 0;
    int driver_id = 0;
    std::string started_at;
    std::string ended_at;
    std::string status = "scheduled"; // 'scheduled', 'in_progress', 'paused', 'completed', 'cancelled'

    // Optional metadata
    std::string bus_number;
    std::string route_name;
    std::string route_code;
    std::string driver_name;

    nlohmann::json to_json() const {
        return nlohmann::json{
            {"id", id},
            {"bus_id", bus_id},
            {"route_id", route_id},
            {"driver_id", driver_id},
            {"started_at", started_at},
            {"ended_at", ended_at},
            {"status", status},
            {"bus_number", bus_number},
            {"route_name", route_name},
            {"route_code", route_code},
            {"driver_name", driver_name}
        };
    }

    static Trip from_json(const nlohmann::json& j) {
        Trip t;
        if (j.contains("id")) t.id = j["id"].get<int>();
        if (j.contains("bus_id")) t.bus_id = j["bus_id"].get<int>();
        if (j.contains("route_id")) t.route_id = j["route_id"].get<int>();
        if (j.contains("driver_id")) t.driver_id = j["driver_id"].get<int>();
        if (j.contains("started_at")) t.started_at = j["started_at"].get<std::string>();
        if (j.contains("ended_at")) t.ended_at = j["ended_at"].get<std::string>();
        if (j.contains("status")) t.status = j["status"].get<std::string>();
        if (j.contains("bus_number")) t.bus_number = j["bus_number"].get<std::string>();
        if (j.contains("route_name")) t.route_name = j["route_name"].get<std::string>();
        if (j.contains("route_code")) t.route_code = j["route_code"].get<std::string>();
        if (j.contains("driver_name")) t.driver_name = j["driver_name"].get<std::string>();
        return t;
    }
};

} // namespace models
} // namespace transitx

#endif // TRANSITX_MODELS_TRIP_HPP
