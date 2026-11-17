// favorite_controller.hpp - User favorites endpoints
#pragma once
#ifndef TRANSITX_CONTROLLERS_FAVORITE_CONTROLLER_HPP
#define TRANSITX_CONTROLLERS_FAVORITE_CONTROLLER_HPP

#include "crow_all.h"

namespace transitx {
namespace controllers {

class FavoriteController {
public:
    static void register_routes(crow::SimpleApp& app);
};

} // namespace controllers
} // namespace transitx

#endif // TRANSITX_CONTROLLERS_FAVORITE_CONTROLLER_HPP
