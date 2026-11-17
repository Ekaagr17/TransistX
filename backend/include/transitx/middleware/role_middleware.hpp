// role_middleware.hpp - Role-based authorization middleware
#pragma once
#ifndef TRANSITX_MIDDLEWARE_ROLE_MIDDLEWARE_HPP
#define TRANSITX_MIDDLEWARE_ROLE_MIDDLEWARE_HPP

#include <string>
#include <vector>
#include "crow_all.h"
#include "transitx/models/user.hpp"
#include "json.hpp"

namespace transitx {
namespace middleware {

class RoleMiddleware {
public:
    /**
     * Checks if user has the required role.
     */
    static bool has_role(const models::User& user, const std::string& required_role) {
        if (user.role == "admin") return true; // Admin has superuser access
        return user.role == required_role;
    }

    /**
     * Checks if user has any of the allowed roles.
     */
    static bool has_any_role(const models::User& user, const std::vector<std::string>& allowed_roles) {
        if (user.role == "admin") return true;
        for (const auto& r : allowed_roles) {
            if (user.role == r) return true;
        }
        return false;
    }

    /**
     * Generate standard 403 Forbidden response
     */
    static crow::response forbidden_response(const std::string& msg = "Access denied: insufficient permissions") {
        nlohmann::json err = {
            {"error", "Forbidden"},
            {"message", msg},
            {"status", 403}
        };
        return crow::response(403, err.dump());
    }
};

} // namespace middleware
} // namespace transitx

#endif // TRANSITX_MIDDLEWARE_ROLE_MIDDLEWARE_HPP
