// config.hpp - Environment and application configuration loader
#pragma once
#ifndef TRANSITX_UTILS_CONFIG_HPP
#define TRANSITX_UTILS_CONFIG_HPP

#include <string>
#include <cstdlib>

namespace transitx {
namespace utils {

class Config {
public:
    static std::string get_env(const std::string& key, const std::string& default_val = "") {
        const char* val = std::getenv(key.c_str());
        return val ? std::string(val) : default_val;
    }

    static int get_env_int(const std::string& key, int default_val = 0) {
        const char* val = std::getenv(key.c_str());
        if (!val) return default_val;
        try {
            return std::stoi(val);
        } catch (...) {
            return default_val;
        }
    }

    // Database configuration
    static std::string db_host() { return get_env("DB_HOST", "localhost"); }
    static int db_port() { return get_env_int("DB_PORT", 5432); }
    static std::string db_name() { return get_env("DB_NAME", "transitx_db"); }
    static std::string db_user() { return get_env("DB_USER", "transitx"); }
    static std::string db_password() { return get_env("DB_PASSWORD", "change_me_in_production"); }
    static int db_pool_size() { return get_env_int("DB_POOL_SIZE", 10); }

    // Server configuration
    static int server_port() { return get_env_int("BACKEND_PORT", 8080); }
    static std::string jwt_secret() { return get_env("JWT_SECRET", "transitx_super_secret_jwt_key_2026_production"); }
    static int jwt_expiry_seconds() { return get_env_int("JWT_EXPIRY_SECONDS", 86400 * 7); } // 7 days
    static std::string cors_origin() { return get_env("CORS_ORIGIN", "*"); }
};

} // namespace utils
} // namespace transitx

#endif // TRANSITX_UTILS_CONFIG_HPP
