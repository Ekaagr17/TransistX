// favorite.hpp - Favorite route model
#pragma once
#ifndef TRANSITX_MODELS_FAVORITE_HPP
#define TRANSITX_MODELS_FAVORITE_HPP

#include <string>
#include "json.hpp"

namespace transitx {
namespace models {

struct Favorite {
    int id = 0;
    int user_id = 0;
    int route_id = 0;
    std::string created_at;

    // Joined route information
    std::string route_name;
    std::string route_code;
    std::string start_point;
    std::string end_point;

    nlohmann::json to_json() const {
        return nlohmann::json{
            {"id", id},
            {"user_id", user_id},
            {"route_id", route_id},
            {"created_at", created_at},
            {"route_name", route_name},
            {"route_code", route_code},
            {"start_point", start_point},
            {"end_point", end_point}
        };
    }

    static Favorite from_json(const nlohmann::json& j) {
        Favorite f;
        if (j.contains("id")) f.id = j["id"].get<int>();
        if (j.contains("user_id")) f.user_id = j["user_id"].get<int>();
        if (j.contains("route_id")) f.route_id = j["route_id"].get<int>();
        if (j.contains("created_at")) f.created_at = j["created_at"].get<std::string>();
        if (j.contains("route_name")) f.route_name = j["route_name"].get<std::string>();
        if (j.contains("route_code")) f.route_code = j["route_code"].get<std::string>();
        if (j.contains("start_point")) f.start_point = j["start_point"].get<std::string>();
        if (j.contains("end_point")) f.end_point = j["end_point"].get<std::string>();
        return f;
    }
};

} // namespace models
} // namespace transitx

#endif // TRANSITX_MODELS_FAVORITE_HPP
