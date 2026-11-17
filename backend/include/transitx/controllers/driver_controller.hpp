// driver_controller.hpp - Driver dashboard and trip control endpoints
#pragma once
#ifndef TRANSITX_CONTROLLERS_DRIVER_CONTROLLER_HPP
#define TRANSITX_CONTROLLERS_DRIVER_CONTROLLER_HPP

#include "crow_all.h"

namespace transitx {
namespace controllers {

class DriverController {
public:
    static void register_routes(crow::SimpleApp& app);
};

} // namespace controllers
} // namespace transitx

#endif // TRANSITX_CONTROLLERS_DRIVER_CONTROLLER_HPP
