// favorite_service.cpp - Favorite service implementation
#include "transitx/services/favorite_service.hpp"
#include "transitx/repositories/favorite_repository.hpp"

namespace transitx {
namespace services {

FavoriteService& FavoriteService::get_instance() {
    static FavoriteService instance;
    return instance;
}

std::vector<models::Favorite> FavoriteService::get_user_favorites(int user_id) {
    return repositories::FavoriteRepository::get_instance().get_by_user(user_id);
}

bool FavoriteService::add_favorite(int user_id, int route_id) {
    return repositories::FavoriteRepository::get_instance().add(user_id, route_id);
}

bool FavoriteService::remove_favorite(int user_id, int route_id) {
    return repositories::FavoriteRepository::get_instance().remove(user_id, route_id);
}

bool FavoriteService::is_favorite(int user_id, int route_id) {
    return repositories::FavoriteRepository::get_instance().is_favorite(user_id, route_id);
}

} // namespace services
} // namespace transitx
