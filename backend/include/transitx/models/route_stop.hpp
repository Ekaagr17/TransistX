// route_stop.hpp - Link between Route and Stop with order
#pragma once
#ifndef TRANSITX_MODELS_ROUTE_STOP_HPP
#define TRANSITX_MODELS_ROUTE_STOP_HPP

#include <string>
#include "json.hpp"

namespace transitx {
namespace models {

struct RouteStop {
    int id = 0;
    int route_id = 0;
    int stop_id = 0;
    int stop_order = 0;
    int estimated_stop_duration = 30; // seconds

    // Populated from joins
    std::string stop_name;
    double latitude = 0.0;
    double longitude = 0.0;

    nlohmann::json to_json() const {
        return nlohmann::json{
            {"id", id},
            {"route_id", route_id},
            {"stop_id", stop_id},
            {"stop_order", stop_order},
            {"estimated_stop_duration", estimated_stop_duration},
            {"stop_name", stop_name},
            {"latitude", latitude},
            {"longitude", longitude}
        };
    }

    static RouteStop from_json(const nlohmann::json& j) {
        RouteStop rs;
        if (j.contains("id")) rs.id = j["id"].get<int>();
        if (j.contains("route_id")) rs.route_id = j["route_id"].get<int>();
        if (j.contains("stop_id")) rs.stop_id = j["stop_id"].get<int>();
        if (j.contains("stop_order")) rs.stop_order = j["stop_order"].get<int>();
        if (j.contains("estimated_stop_duration")) rs.estimated_stop_duration = j["estimated_stop_duration"].get<int>();
        if (j.contains("stop_name")) rs.stop_name = j["stop_name"].get<std::string>();
        if (j.contains("latitude")) rs.latitude = j["latitude"].get<double>();
        if (j.contains("longitude")) rs.longitude = j["longitude"].get<double>();
        return rs;
    }
};

} // namespace models
} // namespace transitx

#endif // TRANSITX_MODELS_ROUTE_STOP_HPP
