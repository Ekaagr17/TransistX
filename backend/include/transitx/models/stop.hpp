// stop.hpp - Bus stop model
#pragma once
#ifndef TRANSITX_MODELS_STOP_HPP
#define TRANSITX_MODELS_STOP_HPP

#include <string>
#include "json.hpp"

namespace transitx {
namespace models {

struct Stop {
    int id = 0;
    std::string stop_name;
    double latitude = 0.0;
    double longitude = 0.0;
    double distance_meters = 0.0; // Populated when querying nearby stops

    nlohmann::json to_json() const {
        nlohmann::json j = {
            {"id", id},
            {"stop_name", stop_name},
            {"latitude", latitude},
            {"longitude", longitude}
        };
        if (distance_meters > 0.0) {
            j["distance_meters"] = distance_meters;
        }
        return j;
    }

    static Stop from_json(const nlohmann::json& j) {
        Stop s;
        if (j.contains("id")) s.id = j["id"].get<int>();
        if (j.contains("stop_name")) s.stop_name = j["stop_name"].get<std::string>();
        if (j.contains("latitude")) s.latitude = j["latitude"].get<double>();
        if (j.contains("longitude")) s.longitude = j["longitude"].get<double>();
        if (j.contains("distance_meters")) s.distance_meters = j["distance_meters"].get<double>();
        return s;
    }
};

} // namespace models
} // namespace transitx

#endif // TRANSITX_MODELS_STOP_HPP
