// user_repository.hpp - User repository header
#pragma once
#ifndef TRANSITX_REPOSITORIES_USER_REPOSITORY_HPP
#define TRANSITX_REPOSITORIES_USER_REPOSITORY_HPP

#include <vector>
#include <string>
#include <optional>
#include "transitx/models/user.hpp"
#include "transitx/database/connection.hpp"

namespace transitx {
namespace repositories {

class UserRepository {
public:
    static UserRepository& get_instance();

    bool create(models::User& user);
    std::optional<models::User> find_by_id(int id);
    std::optional<models::User> find_by_email(const std::string& email);
    std::vector<models::User> find_all();
    bool update(const models::User& user);
    bool remove(int id);
};

} // namespace repositories
} // namespace transitx

#endif // TRANSITX_REPOSITORIES_USER_REPOSITORY_HPP
