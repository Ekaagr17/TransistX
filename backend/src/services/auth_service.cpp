// auth_service.cpp - Authentication service implementation
#include "transitx/services/auth_service.hpp"
#include "transitx/utils/validation.hpp"

namespace transitx {
namespace services {

AuthService& AuthService::get_instance() {
    static AuthService instance;
    return instance;
}

AuthResult AuthService::register_user(const std::string& name, const std::string& email, const std::string& password, const std::string& role) {
    AuthResult result;

    if (name.empty()) {
        result.error_message = "Name cannot be empty";
        return result;
    }
    if (!utils::Validation::is_valid_email(email)) {
        result.error_message = "Invalid email format";
        return result;
    }
    if (password.length() < 6) {
        result.error_message = "Password must be at least 6 characters long";
        return result;
    }
    if (!utils::Validation::is_valid_role(role)) {
        result.error_message = "Role must be 'passenger', 'driver', or 'admin'";
        return result;
    }

    auto existing = repositories::UserRepository::get_instance().find_by_email(email);
    if (existing.has_value()) {
        result.error_message = "A user with this email already exists";
        return result;
    }

    models::User u;
    u.name = name;
    u.email = email;
    u.password_hash = bcrypt::generate_hash(password, 10);
    u.role = role;

    if (!repositories::UserRepository::get_instance().create(u)) {
        result.error_message = "Failed to create user in database";
        return result;
    }

    result.success = true;
    result.user = u;
    result.token = generate_token(u);
    return result;
}

AuthResult AuthService::login_user(const std::string& email, const std::string& password) {
    AuthResult result;

    if (email.empty() || password.empty()) {
        result.error_message = "Email and password are required";
        return result;
    }

    auto user_opt = repositories::UserRepository::get_instance().find_by_email(email);
    if (!user_opt.has_value()) {
        result.error_message = "Invalid credentials";
        return result;
    }

    const auto& user = user_opt.value();
    if (!bcrypt::verify(password, user.password_hash)) {
        result.error_message = "Invalid credentials";
        return result;
    }

    result.success = true;
    result.user = user;
    result.token = generate_token(user);
    return result;
}

std::string AuthService::generate_token(const models::User& user) {
    nlohmann::json claims = {
        {"sub", user.id},
        {"name", user.name},
        {"email", user.email},
        {"role", user.role}
    };
    return jwt::create(claims, utils::Config::jwt_secret(), utils::Config::jwt_expiry_seconds());
}

std::optional<nlohmann::json> AuthService::verify_token(const std::string& token) {
    try {
        if (!jwt::verify(token, utils::Config::jwt_secret())) {
            return std::nullopt;
        }
        return jwt::get_claims(token, utils::Config::jwt_secret());
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<models::User> AuthService::get_user_from_token(const std::string& token) {
    auto claims = verify_token(token);
    if (!claims.has_value()) return std::nullopt;

    if (!claims->contains("sub")) return std::nullopt;
    int user_id = (*claims)["sub"].get<int>();

    return repositories::UserRepository::get_instance().find_by_id(user_id);
}

} // namespace services
} // namespace transitx
