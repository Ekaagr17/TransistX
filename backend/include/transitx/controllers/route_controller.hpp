// route_controller.hpp - Route endpoints
#pragma once
#ifndef TRANSITX_CONTROLLERS_ROUTE_CONTROLLER_HPP
#define TRANSITX_CONTROLLERS_ROUTE_CONTROLLER_HPP

#include "crow_all.h"

namespace transitx {
namespace controllers {

class RouteController {
public:
    static void register_routes(crow::SimpleApp& app);
};

} // namespace controllers
} // namespace transitx

#endif // TRANSITX_CONTROLLERS_ROUTE_CONTROLLER_HPP
