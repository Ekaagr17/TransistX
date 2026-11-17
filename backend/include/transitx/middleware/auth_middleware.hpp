// auth_middleware.hpp - JWT authentication middleware
#pragma once
#ifndef TRANSITX_MIDDLEWARE_AUTH_MIDDLEWARE_HPP
#define TRANSITX_MIDDLEWARE_AUTH_MIDDLEWARE_HPP

#include <string>
#include <optional>
#include "crow_all.h"
#include "transitx/models/user.hpp"
#include "transitx/services/auth_service.hpp"
#include "json.hpp"

namespace transitx {
namespace middleware {

class AuthMiddleware {
public:
    /**
     * Extracts Bearer token from Authorization header and verifies it.
     * Returns User if valid, or std::nullopt if missing/invalid.
     */
    static std::optional<models::User> authenticate(const crow::request& req) {
        std::string auth_header = req.get_header_value("Authorization");
        if (auth_header.empty()) {
            return std::nullopt;
        }

        std::string token;
        if (auth_header.rfind("Bearer ", 0) == 0) {
            token = auth_header.substr(7);
        } else {
            token = auth_header;
        }

        while (!token.empty() && (token.front() == ' ' || token.front() == '\t')) token.erase(0, 1);
        while (!token.empty() && (token.back() == ' ' || token.back() == '\t' || token.back() == '\r')) token.pop_back();

        if (token.empty()) return std::nullopt;

        return services::AuthService::get_instance().get_user_from_token(token);
    }

    /**
     * Generate standard 401 Unauthorized response
     */
    static crow::response unauthorized_response(const std::string& msg = "Authentication required") {
        nlohmann::json err = {
            {"error", "Unauthorized"},
            {"message", msg},
            {"status", 401}
        };
        return crow::response(401, err.dump());
    }
};

} // namespace middleware
} // namespace transitx

#endif // TRANSITX_MIDDLEWARE_AUTH_MIDDLEWARE_HPP
