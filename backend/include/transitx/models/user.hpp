// user.hpp - User model
#pragma once
#ifndef TRANSITX_MODELS_USER_HPP
#define TRANSITX_MODELS_USER_HPP

#include <string>
#include "json.hpp"

namespace transitx {
namespace models {

struct User {
    int id = 0;
    std::string name;
    std::string email;
    std::string password_hash;
    std::string role; // 'passenger', 'driver', 'admin'
    std::string created_at;

    nlohmann::json to_json(bool include_password = false) const {
        nlohmann::json j = {
            {"id", id},
            {"name", name},
            {"email", email},
            {"role", role},
            {"created_at", created_at}
        };
        if (include_password) {
            j["password_hash"] = password_hash;
        }
        return j;
    }

    static User from_json(const nlohmann::json& j) {
        User u;
        if (j.contains("id")) u.id = j["id"].get<int>();
        if (j.contains("name")) u.name = j["name"].get<std::string>();
        if (j.contains("email")) u.email = j["email"].get<std::string>();
        if (j.contains("password_hash")) u.password_hash = j["password_hash"].get<std::string>();
        if (j.contains("role")) u.role = j["role"].get<std::string>();
        if (j.contains("created_at")) u.created_at = j["created_at"].get<std::string>();
        return u;
    }
};

} // namespace models
} // namespace transitx

#endif // TRANSITX_MODELS_USER_HPP
