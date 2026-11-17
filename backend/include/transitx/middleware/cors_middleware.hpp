// cors_middleware.hpp - Cross-Origin Resource Sharing middleware
#pragma once
#ifndef TRANSITX_MIDDLEWARE_CORS_MIDDLEWARE_HPP
#define TRANSITX_MIDDLEWARE_CORS_MIDDLEWARE_HPP

#include <string>
#include "crow_all.h"
#include "transitx/utils/config.hpp"

namespace transitx {
namespace middleware {

class CorsMiddleware {
public:
    static void apply(crow::response& res) {
        std::string origin = utils::Config::cors_origin();
        res.set_header("Access-Control-Allow-Origin", origin.empty() ? "*" : origin);
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS, PATCH");
        res.set_header("Access-Control-Allow-Headers", "Authorization, Content-Type, Accept, X-Requested-With");
        res.set_header("Access-Control-Max-Age", "86400");
    }

    static crow::response handle_preflight() {
        crow::response res(204);
        apply(res);
        return res;
    }
};

} // namespace middleware
} // namespace transitx

#endif // TRANSITX_MIDDLEWARE_CORS_MIDDLEWARE_HPP
