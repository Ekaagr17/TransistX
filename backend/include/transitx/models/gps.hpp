// gps.hpp - GPS update and latest location models
#pragma once
#ifndef TRANSITX_MODELS_GPS_HPP
#define TRANSITX_MODELS_GPS_HPP

#include <string>
#include <cstdint>
#include "json.hpp"

namespace transitx {
namespace models {

struct GpsUpdate {
    int64_t id = 0;
    int bus_id = 0;
    int trip_id = 0;
    double latitude = 0.0;
    double longitude = 0.0;
    double speed = 0.0;     // km/h
    double heading = 0.0;   // 0-360 degrees
    double accuracy = 0.0;  // meters
    std::string recorded_at;

    nlohmann::json to_json() const {
        return nlohmann::json{
            {"id", id},
            {"bus_id", bus_id},
            {"trip_id", trip_id},
            {"latitude", latitude},
            {"longitude", longitude},
            {"speed", speed},
            {"heading", heading},
            {"accuracy", accuracy},
            {"recorded_at", recorded_at}
        };
    }

    static GpsUpdate from_json(const nlohmann::json& j) {
        GpsUpdate g;
        if (j.contains("id")) g.id = j["id"].get<int64_t>();
        if (j.contains("bus_id")) g.bus_id = j["bus_id"].get<int>();
        if (j.contains("trip_id")) g.trip_id = j["trip_id"].get<int>();
        if (j.contains("latitude")) g.latitude = j["latitude"].get<double>();
        if (j.contains("longitude")) g.longitude = j["longitude"].get<double>();
        if (j.contains("speed")) g.speed = j["speed"].get<double>();
        if (j.contains("heading")) g.heading = j["heading"].get<double>();
        if (j.contains("accuracy")) g.accuracy = j["accuracy"].get<double>();
        if (j.contains("recorded_at")) g.recorded_at = j["recorded_at"].get<std::string>();
        return g;
    }
};

struct BusLatestLocation {
    int bus_id = 0;
    double latitude = 0.0;
    double longitude = 0.0;
    double speed = 0.0;
    double heading = 0.0;
    double accuracy = 0.0;
    std::string updated_at;

    // Joined metadata
    std::string bus_number;
    std::string route_name;
    std::string route_code;
    int route_id = 0;
    std::string status;

    nlohmann::json to_json() const {
        return nlohmann::json{
            {"bus_id", bus_id},
            {"latitude", latitude},
            {"longitude", longitude},
            {"speed", speed},
            {"heading", heading},
            {"accuracy", accuracy},
            {"updated_at", updated_at},
            {"bus_number", bus_number},
            {"route_name", route_name},
            {"route_code", route_code},
            {"route_id", route_id},
            {"status", status}
        };
    }

    static BusLatestLocation from_json(const nlohmann::json& j) {
        BusLatestLocation b;
        if (j.contains("bus_id")) b.bus_id = j["bus_id"].get<int>();
        if (j.contains("latitude")) b.latitude = j["latitude"].get<double>();
        if (j.contains("longitude")) b.longitude = j["longitude"].get<double>();
        if (j.contains("speed")) b.speed = j["speed"].get<double>();
        if (j.contains("heading")) b.heading = j["heading"].get<double>();
        if (j.contains("accuracy")) b.accuracy = j["accuracy"].get<double>();
        if (j.contains("updated_at")) b.updated_at = j["updated_at"].get<std::string>();
        if (j.contains("bus_number")) b.bus_number = j["bus_number"].get<std::string>();
        if (j.contains("route_name")) b.route_name = j["route_name"].get<std::string>();
        if (j.contains("route_code")) b.route_code = j["route_code"].get<std::string>();
        if (j.contains("route_id")) b.route_id = j["route_id"].get<int>();
        if (j.contains("status")) b.status = j["status"].get<std::string>();
        return b;
    }
};

} // namespace models
} // namespace transitx

#endif // TRANSITX_MODELS_GPS_HPP
