// logger.hpp - Thread-safe logging macros with timestamps
#pragma once
#ifndef TRANSITX_UTILS_LOGGER_HPP
#define TRANSITX_UTILS_LOGGER_HPP

#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace transitx {
namespace utils {

inline std::string current_timestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* tm_buf = std::localtime(&now);
    if (!tm_buf) return "";
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_buf);
    return std::string(buf);
}

} // namespace utils
} // namespace transitx

#define LOG_INFO(msg) \
    std::cout << "[" << transitx::utils::current_timestamp() << "] [INFO]  " << msg << std::endl

#define LOG_WARN(msg) \
    std::cout << "[" << transitx::utils::current_timestamp() << "] [WARN]  " << msg << std::endl

#define LOG_ERROR(msg) \
    std::cerr << "[" << transitx::utils::current_timestamp() << "] [ERROR] " << msg << std::endl

#define LOG_DEBUG(msg) \
    std::cout << "[" << transitx::utils::current_timestamp() << "] [DEBUG] " << msg << std::endl

#endif // TRANSITX_UTILS_LOGGER_HPP
