// bus_repository.hpp - Bus repository header
#pragma once
#ifndef TRANSITX_REPOSITORIES_BUS_REPOSITORY_HPP
#define TRANSITX_REPOSITORIES_BUS_REPOSITORY_HPP

#include <vector>
#include <string>
#include <optional>
#include "transitx/models/bus.hpp"
#include "transitx/database/connection.hpp"

namespace transitx {
namespace repositories {

class BusRepository {
public:
    static BusRepository& get_instance();

    std::vector<models::Bus> find_all();
    std::optional<models::Bus> find_by_id(int id);
    std::optional<models::Bus> find_by_number(const std::string& bus_number);
    bool create(models::Bus& bus);
    bool update(const models::Bus& bus);
    bool update_status(int id, const std::string& status);
    bool remove(int id);
};

} // namespace repositories
} // namespace transitx

#endif // TRANSITX_REPOSITORIES_BUS_REPOSITORY_HPP
