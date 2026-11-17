// bus_repository.cpp - Bus repository implementation
#include "transitx/repositories/bus_repository.hpp"
#include <map>

namespace transitx {
namespace repositories {

BusRepository& BusRepository::get_instance() {
    static BusRepository instance;
    return instance;
}

static crow::Mutex bus_mutex;
static std::map<int, models::Bus> mock_buses = {
    {1, {1, "UK07PA7001", "UK-07-PA-7001", 40, "active", "2026-11-17 10:00:00"}},
    {2, {2, "UK07PA7002", "UK-07-PA-7002", 42, "active", "2026-11-17 10:00:00"}},
    {3, {3, "UK07PA7003", "UK-07-PA-7003", 44, "active", "2026-11-17 10:00:00"}},
    {4, {4, "UK07PA7004", "UK-07-PA-7004", 48, "active", "2026-11-17 10:00:00"}},
    {5, {5, "UK07PA7005", "UK-07-PA-7005", 50, "active", "2026-11-17 10:00:00"}},
    {6, {6, "UK07PA7006", "UK-07-PA-7006", 52, "inactive", "2026-11-17 10:00:00"}},
    {7, {7, "UK07PA7007", "UK-07-PA-7007", 54, "inactive", "2026-11-17 10:00:00"}},
    {8, {8, "UK07PA7008", "UK-07-PA-7008", 56, "active", "2026-11-17 10:00:00"}}
};
static int next_bus_id = 9;

std::vector<models::Bus> BusRepository::find_all() {
    std::vector<models::Bus> list;
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec("SELECT id, bus_number, registration_number, capacity, status, created_at FROM buses ORDER BY id ASC");
            for (size_t i = 0; i < res.size(); ++i) {
                models::Bus b;
                b.id = res[i]["id"].as<int>();
                b.bus_number = res[i]["bus_number"].as<std::string>();
                b.registration_number = res[i]["registration_number"].as<std::string>();
                b.capacity = res[i]["capacity"].as<int>();
                b.status = res[i]["status"].as<std::string>();
                b.created_at = res[i]["created_at"].as<std::string>();
                list.push_back(b);
            }
            if (!list.empty()) return list;
        }
    } catch (...) {}

    crow::LockGuard lock(bus_mutex);
    for (const auto& pair : mock_buses) {
        list.push_back(pair.second);
    }
    return list;
}

std::optional<models::Bus> BusRepository::find_by_id(int id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params("SELECT id, bus_number, registration_number, capacity, status, created_at FROM buses WHERE id = $1", id);
            if (!res.empty()) {
                models::Bus b;
                b.id = res[0]["id"].as<int>();
                b.bus_number = res[0]["bus_number"].as<std::string>();
                b.registration_number = res[0]["registration_number"].as<std::string>();
                b.capacity = res[0]["capacity"].as<int>();
                b.status = res[0]["status"].as<std::string>();
                b.created_at = res[0]["created_at"].as<std::string>();
                return b;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(bus_mutex);
    auto it = mock_buses.find(id);
    if (it != mock_buses.end()) return it->second;
    return std::nullopt;
}

std::optional<models::Bus> BusRepository::find_by_number(const std::string& bus_number) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params("SELECT id, bus_number, registration_number, capacity, status, created_at FROM buses WHERE bus_number = $1", bus_number);
            if (!res.empty()) {
                models::Bus b;
                b.id = res[0]["id"].as<int>();
                b.bus_number = res[0]["bus_number"].as<std::string>();
                b.registration_number = res[0]["registration_number"].as<std::string>();
                b.capacity = res[0]["capacity"].as<int>();
                b.status = res[0]["status"].as<std::string>();
                b.created_at = res[0]["created_at"].as<std::string>();
                return b;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(bus_mutex);
    for (const auto& pair : mock_buses) {
        if (pair.second.bus_number == bus_number) return pair.second;
    }
    return std::nullopt;
}

bool BusRepository::create(models::Bus& bus) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params(
                "INSERT INTO buses (bus_number, registration_number, capacity, status) VALUES ($1, $2, $3, $4) RETURNING id, created_at",
                bus.bus_number, bus.registration_number, bus.capacity, bus.status
            );
            if (!res.empty()) {
                bus.id = res[0]["id"].as<int>();
                bus.created_at = res[0]["created_at"].as<std::string>();
                txn.commit();
                return true;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(bus_mutex);
    bus.id = next_bus_id++;
    bus.created_at = utils::current_timestamp();
    mock_buses[bus.id] = bus;
    return true;
}

bool BusRepository::update(const models::Bus& bus) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params("UPDATE buses SET bus_number = $1, registration_number = $2, capacity = $3, status = $4 WHERE id = $5",
                bus.bus_number, bus.registration_number, bus.capacity, bus.status, bus.id);
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(bus_mutex);
    auto it = mock_buses.find(bus.id);
    if (it != mock_buses.end()) {
        it->second = bus;
        return true;
    }
    return false;
}

bool BusRepository::update_status(int id, const std::string& status) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params("UPDATE buses SET status = $1 WHERE id = $2", status, id);
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(bus_mutex);
    auto it = mock_buses.find(id);
    if (it != mock_buses.end()) {
        it->second.status = status;
        return true;
    }
    return false;
}

bool BusRepository::remove(int id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params("DELETE FROM buses WHERE id = $1", id);
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(bus_mutex);
    return mock_buses.erase(id) > 0;
}

} // namespace repositories
} // namespace transitx
