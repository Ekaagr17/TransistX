// favorite_repository.cpp - Favorite repository implementation
#include "transitx/repositories/favorite_repository.hpp"
#include <map>

namespace transitx {
namespace repositories {

FavoriteRepository& FavoriteRepository::get_instance() {
    static FavoriteRepository instance;
    return instance;
}

static crow::Mutex fav_mutex;
static std::vector<models::Favorite> mock_favorites = {
    {1, 1, 1, "2026-11-17 10:00:00", "Demo: Clock Tower - ISBT", "DR001", "Clock Tower", "ISBT Dehradun"},
    {2, 1, 2, "2026-11-17 10:00:00", "Demo: Rajpur Road - Mussoorie", "DR002", "Rajpur Road", "Mussoorie"},
    {3, 2, 3, "2026-11-17 10:00:00", "Demo: Clement Town - Ballupur", "DR003", "Clement Town", "Ballupur Chowk"}
};
static int next_fav_id = 4;

std::vector<models::Favorite> FavoriteRepository::get_by_user(int user_id) {
    std::vector<models::Favorite> list;
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params(
                "SELECT f.id, f.user_id, f.route_id, f.created_at, "
                "r.route_name, r.route_code, r.start_point, r.end_point "
                "FROM favorites f "
                "JOIN routes r ON f.route_id = r.id "
                "WHERE f.user_id = $1 ORDER BY f.created_at DESC", user_id
            );
            for (size_t i = 0; i < res.size(); ++i) {
                models::Favorite f;
                f.id = res[i]["id"].as<int>();
                f.user_id = res[i]["user_id"].as<int>();
                f.route_id = res[i]["route_id"].as<int>();
                f.created_at = res[i]["created_at"].as<std::string>();
                f.route_name = res[i]["route_name"].as<std::string>();
                f.route_code = res[i]["route_code"].as<std::string>();
                f.start_point = res[i]["start_point"].as<std::string>();
                f.end_point = res[i]["end_point"].as<std::string>();
                list.push_back(f);
            }
            if (!list.empty()) return list;
        }
    } catch (...) {}

    crow::LockGuard lock(fav_mutex);
    for (const auto& fav : mock_favorites) {
        if (fav.user_id == user_id) {
            list.push_back(fav);
        }
    }
    return list;
}

bool FavoriteRepository::add(int user_id, int route_id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params(
                "INSERT INTO favorites (user_id, route_id) VALUES ($1, $2) ON CONFLICT (user_id, route_id) DO NOTHING",
                user_id, route_id
            );
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(fav_mutex);
    for (const auto& fav : mock_favorites) {
        if (fav.user_id == user_id && fav.route_id == route_id) return true;
    }
    models::Favorite f;
    f.id = next_fav_id++;
    f.user_id = user_id;
    f.route_id = route_id;
    f.created_at = utils::current_timestamp();
    mock_favorites.push_back(f);
    return true;
}

bool FavoriteRepository::remove(int user_id, int route_id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params("DELETE FROM favorites WHERE user_id = $1 AND route_id = $2", user_id, route_id);
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(fav_mutex);
    for (auto it = mock_favorites.begin(); it != mock_favorites.end(); ++it) {
        if (it->user_id == user_id && it->route_id == route_id) {
            mock_favorites.erase(it);
            return true;
        }
    }
    return false;
}

bool FavoriteRepository::is_favorite(int user_id, int route_id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params("SELECT id FROM favorites WHERE user_id = $1 AND route_id = $2", user_id, route_id);
            return !res.empty();
        }
    } catch (...) {}

    crow::LockGuard lock(fav_mutex);
    for (const auto& fav : mock_favorites) {
        if (fav.user_id == user_id && fav.route_id == route_id) return true;
    }
    return false;
}

} // namespace repositories
} // namespace transitx
