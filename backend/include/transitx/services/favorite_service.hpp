// favorite_service.hpp - User favorites service header
#pragma once
#ifndef TRANSITX_SERVICES_FAVORITE_SERVICE_HPP
#define TRANSITX_SERVICES_FAVORITE_SERVICE_HPP

#include <vector>
#include "transitx/models/favorite.hpp"

namespace transitx {
namespace services {

class FavoriteService {
public:
    static FavoriteService& get_instance();

    std::vector<models::Favorite> get_user_favorites(int user_id);
    bool add_favorite(int user_id, int route_id);
    bool remove_favorite(int user_id, int route_id);
    bool is_favorite(int user_id, int route_id);
};

} // namespace services
} // namespace transitx

#endif // TRANSITX_SERVICES_FAVORITE_SERVICE_HPP
