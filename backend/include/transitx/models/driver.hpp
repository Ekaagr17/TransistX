// driver.hpp - Driver model
#pragma once
#ifndef TRANSITX_MODELS_DRIVER_HPP
#define TRANSITX_MODELS_DRIVER_HPP

#include <string>
#include "json.hpp"

namespace transitx {
namespace models {

struct Driver {
    int id = 0;
    int user_id = 0;
    std::string license_number;
    std::string driver_name;
    std::string email;

    nlohmann::json to_json() const {
        return nlohmann::json{
            {"id", id},
            {"user_id", user_id},
            {"license_number", license_number},
            {"driver_name", driver_name},
            {"email", email}
        };
    }

    static Driver from_json(const nlohmann::json& j) {
        Driver d;
        if (j.contains("id")) d.id = j["id"].get<int>();
        if (j.contains("user_id")) d.user_id = j["user_id"].get<int>();
        if (j.contains("license_number")) d.license_number = j["license_number"].get<std::string>();
        if (j.contains("driver_name")) d.driver_name = j["driver_name"].get<std::string>();
        if (j.contains("email")) d.email = j["email"].get<std::string>();
        return d;
    }
};

} // namespace models
} // namespace transitx

#endif // TRANSITX_MODELS_DRIVER_HPP
