// validation.hpp - Input validation routines
#pragma once
#ifndef TRANSITX_UTILS_VALIDATION_HPP
#define TRANSITX_UTILS_VALIDATION_HPP

#include <string>
#include <regex>

namespace transitx {
namespace utils {

class Validation {
public:
    static bool is_valid_email(const std::string& email) {
        if (email.empty() || email.length() > 255) return false;
        const std::regex pattern(R"(^[a-zA-Z0-9.!#$%&'*+/=?^_`{|}~-]+@[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(?:\.[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$)");
        return std::regex_match(email, pattern);
    }

    static bool is_valid_coordinates(double lat, double lng) {
        return (lat >= -90.0 && lat <= 90.0) && (lng >= -180.0 && lng <= 180.0);
    }

    static bool is_valid_bus_number(const std::string& bus_number) {
        if (bus_number.empty() || bus_number.length() > 32) return false;
        // Alphanumeric, hyphens, spaces
        for (char c : bus_number) {
            if (!isalnum(c) && c != '-' && c != ' ') return false;
        }
        return true;
    }

    static bool is_valid_role(const std::string& role) {
        return role == "passenger" || role == "driver" || role == "admin";
    }

    static bool is_valid_speed(double speed) {
        return speed >= 0.0 && speed <= 150.0; // km/h
    }

    static bool is_valid_accuracy(double accuracy) {
        return accuracy >= 0.0 && accuracy <= 500.0; // meters
    }
};

} // namespace utils
} // namespace transitx

#endif // TRANSITX_UTILS_VALIDATION_HPP
