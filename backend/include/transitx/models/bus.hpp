// bus.hpp - Bus model
#pragma once
#ifndef TRANSITX_MODELS_BUS_HPP
#define TRANSITX_MODELS_BUS_HPP

#include <string>
#include "json.hpp"

namespace transitx {
namespace models {

struct Bus {
    int id = 0;
    std::string bus_number;
    std::string registration_number;
    int capacity = 50;
    std::string status = "inactive"; // 'active', 'inactive', 'maintenance'
    std::string created_at;

    nlohmann::json to_json() const {
        return nlohmann::json{
            {"id", id},
            {"bus_number", bus_number},
            {"registration_number", registration_number},
            {"capacity", capacity},
            {"status", status},
            {"created_at", created_at}
        };
    }

    static Bus from_json(const nlohmann::json& j) {
        Bus b;
        if (j.contains("id")) b.id = j["id"].get<int>();
        if (j.contains("bus_number")) b.bus_number = j["bus_number"].get<std::string>();
        if (j.contains("registration_number")) b.registration_number = j["registration_number"].get<std::string>();
        if (j.contains("capacity")) b.capacity = j["capacity"].get<int>();
        if (j.contains("status")) b.status = j["status"].get<std::string>();
        if (j.contains("created_at")) b.created_at = j["created_at"].get<std::string>();
        return b;
    }
};

} // namespace models
} // namespace transitx

#endif // TRANSITX_MODELS_BUS_HPP
