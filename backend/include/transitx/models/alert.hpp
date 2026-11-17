// alert.hpp - Service alert model
#pragma once
#ifndef TRANSITX_MODELS_ALERT_HPP
#define TRANSITX_MODELS_ALERT_HPP

#include <string>
#include "json.hpp"

namespace transitx {
namespace models {

struct ServiceAlert {
    int id = 0;
    int route_id = 0;
    std::string message;
    std::string severity = "info"; // 'info', 'warning', 'critical'
    std::string created_at;
    std::string expires_at;

    // Joined route information
    std::string route_name;
    std::string route_code;

    nlohmann::json to_json() const {
        return nlohmann::json{
            {"id", id},
            {"route_id", route_id},
            {"message", message},
            {"severity", severity},
            {"created_at", created_at},
            {"expires_at", expires_at},
            {"route_name", route_name},
            {"route_code", route_code}
        };
    }

    static ServiceAlert from_json(const nlohmann::json& j) {
        ServiceAlert a;
        if (j.contains("id")) a.id = j["id"].get<int>();
        if (j.contains("route_id")) a.route_id = j["route_id"].get<int>();
        if (j.contains("message")) a.message = j["message"].get<std::string>();
        if (j.contains("severity")) a.severity = j["severity"].get<std::string>();
        if (j.contains("created_at")) a.created_at = j["created_at"].get<std::string>();
        if (j.contains("expires_at")) a.expires_at = j["expires_at"].get<std::string>();
        if (j.contains("route_name")) a.route_name = j["route_name"].get<std::string>();
        if (j.contains("route_code")) a.route_code = j["route_code"].get<std::string>();
        return a;
    }
};

} // namespace models
} // namespace transitx

#endif // TRANSITX_MODELS_ALERT_HPP
