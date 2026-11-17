// rate_limit_middleware.hpp - In-memory rate limiting middleware
#pragma once
#ifndef TRANSITX_MIDDLEWARE_RATE_LIMIT_MIDDLEWARE_HPP
#define TRANSITX_MIDDLEWARE_RATE_LIMIT_MIDDLEWARE_HPP

#include <string>
#include <unordered_map>
#include <ctime>
#include "crow_all.h"
#include "json.hpp"

namespace transitx {
namespace middleware {

struct RateLimitBucket {
    int count = 0;
    std::time_t window_start = 0;
};

class RateLimitMiddleware {
private:
    static std::unordered_map<std::string, RateLimitBucket> buckets_;
    static crow::Mutex mutex_;

public:
    /**
     * Checks if IP exceeded rate limit.
     * Default: 120 requests per 60-second window.
     */
    static bool allow_request(const std::string& client_ip, int max_requests = 120, int window_seconds = 60) {
        crow::LockGuard lock(mutex_);
        std::time_t now = std::time(nullptr);

        auto& bucket = buckets_[client_ip];
        if (now - bucket.window_start >= window_seconds) {
            bucket.window_start = now;
            bucket.count = 1;
            return true;
        }

        bucket.count++;
        return bucket.count <= max_requests;
    }

    static crow::response rate_limited_response() {
        nlohmann::json err = {
            {"error", "Too Many Requests"},
            {"message", "Rate limit exceeded. Please slow down."},
            {"status", 429}
        };
        crow::response res(429, err.dump());
        res.set_header("Retry-After", "30");
        return res;
    }
};

inline std::unordered_map<std::string, RateLimitBucket> RateLimitMiddleware::buckets_;
inline crow::Mutex RateLimitMiddleware::mutex_;

} // namespace middleware
} // namespace transitx

#endif // TRANSITX_MIDDLEWARE_RATE_LIMIT_MIDDLEWARE_HPP
