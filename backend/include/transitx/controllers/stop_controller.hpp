// stop_controller.hpp - Stop endpoints
#pragma once
#ifndef TRANSITX_CONTROLLERS_STOP_CONTROLLER_HPP
#define TRANSITX_CONTROLLERS_STOP_CONTROLLER_HPP

#include "crow_all.h"

namespace transitx {
namespace controllers {

class StopController {
public:
    static void register_routes(crow::SimpleApp& app);
};

} // namespace controllers
} // namespace transitx

#endif // TRANSITX_CONTROLLERS_STOP_CONTROLLER_HPP
