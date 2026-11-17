// favorite_repository.hpp - Favorite repository header
#pragma once
#ifndef TRANSITX_REPOSITORIES_FAVORITE_REPOSITORY_HPP
#define TRANSITX_REPOSITORIES_FAVORITE_REPOSITORY_HPP

#include <vector>
#include <string>
#include "transitx/models/favorite.hpp"
#include "transitx/database/connection.hpp"

namespace transitx {
namespace repositories {

class FavoriteRepository {
public:
    static FavoriteRepository& get_instance();

    std::vector<models::Favorite> get_by_user(int user_id);
    bool add(int user_id, int route_id);
    bool remove(int user_id, int route_id);
    bool is_favorite(int user_id, int route_id);
};

} // namespace repositories
} // namespace transitx

#endif // TRANSITX_REPOSITORIES_FAVORITE_REPOSITORY_HPP
