// user_repository.cpp - User repository implementation
#include "transitx/repositories/user_repository.hpp"
#include <map>

namespace transitx {
namespace repositories {

UserRepository& UserRepository::get_instance() {
    static UserRepository instance;
    return instance;
}

static crow::Mutex user_mutex;
static std::map<int, models::User> mock_users = {
    {1, {1, "Amit Sharma", "pass1@demo.com", "$2b$12$LJ3m4ys3Lk0TSwHlvTGKAOWZYQz5y0jXbkTvai38A.GlwIRMiMvNi", "passenger", "2026-11-17 10:00:00"}},
    {2, {2, "Priya Rawat", "pass2@demo.com", "$2b$12$LJ3m4ys3Lk0TSwHlvTGKAOWZYQz5y0jXbkTvai38A.GlwIRMiMvNi", "passenger", "2026-11-17 10:00:00"}},
    {3, {3, "Rahul Negi", "pass3@demo.com", "$2b$12$LJ3m4ys3Lk0TSwHlvTGKAOWZYQz5y0jXbkTvai38A.GlwIRMiMvNi", "passenger", "2026-11-17 10:00:00"}},
    {4, {4, "Suresh Panwar", "driver1@demo.com", "$2b$12$LJ3m4ys3Lk0TSwHlvTGKAOWZYQz5y0jXbkTvai38A.GlwIRMiMvNi", "driver", "2026-11-17 10:00:00"}},
    {5, {5, "Manoj Bisht", "driver2@demo.com", "$2b$12$LJ3m4ys3Lk0TSwHlvTGKAOWZYQz5y0jXbkTvai38A.GlwIRMiMvNi", "driver", "2026-11-17 10:00:00"}},
    {6, {6, "Deepak Thapa", "driver3@demo.com", "$2b$12$LJ3m4ys3Lk0TSwHlvTGKAOWZYQz5y0jXbkTvai38A.GlwIRMiMvNi", "driver", "2026-11-17 10:00:00"}},
    {7, {7, "Rajesh Gusain", "driver4@demo.com", "$2b$12$LJ3m4ys3Lk0TSwHlvTGKAOWZYQz5y0jXbkTvai38A.GlwIRMiMvNi", "driver", "2026-11-17 10:00:00"}},
    {8, {8, "Admin User", "admin@transitx.com", "$2b$12$LJ3m4ys3Lk0TSwHlvTGKAOWZYQz5y0jXbkTvai38A.GlwIRMiMvNi", "admin", "2026-11-17 10:00:00"}}
};
static int next_user_id = 9;

bool UserRepository::create(models::User& user) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params(
                "INSERT INTO users (name, email, password_hash, role) VALUES ($1, $2, $3, $4) RETURNING id, created_at",
                user.name, user.email, user.password_hash, user.role
            );
            if (!res.empty()) {
                user.id = res[0]["id"].as<int>();
                user.created_at = res[0]["created_at"].as<std::string>();
                txn.commit();
                return true;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(user_mutex);
    user.id = next_user_id++;
    user.created_at = utils::current_timestamp();
    mock_users[user.id] = user;
    return true;
}

std::optional<models::User> UserRepository::find_by_id(int id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params("SELECT id, name, email, password_hash, role, created_at FROM users WHERE id = $1", id);
            if (!res.empty()) {
                models::User u;
                u.id = res[0]["id"].as<int>();
                u.name = res[0]["name"].as<std::string>();
                u.email = res[0]["email"].as<std::string>();
                u.password_hash = res[0]["password_hash"].as<std::string>();
                u.role = res[0]["role"].as<std::string>();
                u.created_at = res[0]["created_at"].as<std::string>();
                return u;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(user_mutex);
    auto it = mock_users.find(id);
    if (it != mock_users.end()) return it->second;
    return std::nullopt;
}

std::optional<models::User> UserRepository::find_by_email(const std::string& email) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params("SELECT id, name, email, password_hash, role, created_at FROM users WHERE email = $1", email);
            if (!res.empty()) {
                models::User u;
                u.id = res[0]["id"].as<int>();
                u.name = res[0]["name"].as<std::string>();
                u.email = res[0]["email"].as<std::string>();
                u.password_hash = res[0]["password_hash"].as<std::string>();
                u.role = res[0]["role"].as<std::string>();
                u.created_at = res[0]["created_at"].as<std::string>();
                return u;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(user_mutex);
    for (const auto& pair : mock_users) {
        if (pair.second.email == email) return pair.second;
    }
    return std::nullopt;
}

std::vector<models::User> UserRepository::find_all() {
    std::vector<models::User> list;
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec("SELECT id, name, email, password_hash, role, created_at FROM users ORDER BY id ASC");
            for (size_t i = 0; i < res.size(); ++i) {
                models::User u;
                u.id = res[i]["id"].as<int>();
                u.name = res[i]["name"].as<std::string>();
                u.email = res[i]["email"].as<std::string>();
                u.password_hash = res[i]["password_hash"].as<std::string>();
                u.role = res[i]["role"].as<std::string>();
                u.created_at = res[i]["created_at"].as<std::string>();
                list.push_back(u);
            }
            if (!list.empty()) return list;
        }
    } catch (...) {}

    crow::LockGuard lock(user_mutex);
    for (const auto& pair : mock_users) {
        list.push_back(pair.second);
    }
    return list;
}

bool UserRepository::update(const models::User& user) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params("UPDATE users SET name = $1, email = $2, role = $3 WHERE id = $4",
                user.name, user.email, user.role, user.id);
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(user_mutex);
    auto it = mock_users.find(user.id);
    if (it != mock_users.end()) {
        it->second.name = user.name;
        it->second.email = user.email;
        it->second.role = user.role;
        return true;
    }
    return false;
}

bool UserRepository::remove(int id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params("DELETE FROM users WHERE id = $1", id);
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(user_mutex);
    return mock_users.erase(id) > 0;
}

} // namespace repositories
} // namespace transitx
