// auth_service.hpp - Authentication and authorization service
#pragma once
#ifndef TRANSITX_SERVICES_AUTH_SERVICE_HPP
#define TRANSITX_SERVICES_AUTH_SERVICE_HPP

#include <string>
#include <optional>
#include "transitx/models/user.hpp"
#include "transitx/repositories/user_repository.hpp"
#include "transitx/utils/config.hpp"
#include "jwt.hpp"
#include "bcrypt.hpp"

namespace transitx {
namespace services {

struct AuthResult {
    bool success = false;
    std::string token;
    models::User user;
    std::string error_message;
};

class AuthService {
public:
    static AuthService& get_instance();

    AuthResult register_user(const std::string& name, const std::string& email, const std::string& password, const std::string& role = "passenger");
    AuthResult login_user(const std::string& email, const std::string& password);
    std::string generate_token(const models::User& user);
    std::optional<nlohmann::json> verify_token(const std::string& token);
    std::optional<models::User> get_user_from_token(const std::string& token);
};

} // namespace services
} // namespace transitx

#endif // TRANSITX_SERVICES_AUTH_SERVICE_HPP
