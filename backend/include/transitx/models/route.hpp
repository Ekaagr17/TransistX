// route.hpp - Route model
#pragma once
#ifndef TRANSITX_MODELS_ROUTE_HPP
#define TRANSITX_MODELS_ROUTE_HPP

#include <string>
#include "json.hpp"

namespace transitx {
namespace models {

struct Route {
    int id = 0;
    std::string route_name;
    std::string route_code;
    std::string start_point;
    std::string end_point;
    std::string status = "active"; // 'active', 'inactive'

    nlohmann::json to_json() const {
        return nlohmann::json{
            {"id", id},
            {"route_name", route_name},
            {"route_code", route_code},
            {"start_point", start_point},
            {"end_point", end_point},
            {"status", status}
        };
    }

    static Route from_json(const nlohmann::json& j) {
        Route r;
        if (j.contains("id")) r.id = j["id"].get<int>();
        if (j.contains("route_name")) r.route_name = j["route_name"].get<std::string>();
        if (j.contains("route_code")) r.route_code = j["route_code"].get<std::string>();
        if (j.contains("start_point")) r.start_point = j["start_point"].get<std::string>();
        if (j.contains("end_point")) r.end_point = j["end_point"].get<std::string>();
        if (j.contains("status")) r.status = j["status"].get<std::string>();
        return r;
    }
};

} // namespace models
} // namespace transitx

#endif // TRANSITX_MODELS_ROUTE_HPP
