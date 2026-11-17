// trip_repository.cpp - Trip repository implementation
#include "transitx/repositories/trip_repository.hpp"
#include <map>

namespace transitx {
namespace repositories {

TripRepository& TripRepository::get_instance() {
    static TripRepository instance;
    return instance;
}

static crow::Mutex trip_mutex;
static std::map<int, models::Trip> mock_trips = {
    {1, {1, 1, 1, 1, "2026-11-16 07:00:00+05:30", "2026-11-16 07:55:00+05:30", "completed", "UK07PA7001", "Demo: Clock Tower - ISBT", "DR001", "Suresh Panwar"}},
    {2, {2, 2, 2, 2, "2026-11-16 08:30:00+05:30", "2026-11-16 10:15:00+05:30", "completed", "UK07PA7002", "Demo: Rajpur Road - Mussoorie", "DR002", "Manoj Bisht"}}
};
static int next_trip_id = 3;

std::vector<models::Trip> TripRepository::find_all() {
    std::vector<models::Trip> list;
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec(
                "SELECT t.id, t.bus_id, t.route_id, t.driver_id, t.started_at, t.ended_at, t.status, "
                "b.bus_number, r.route_name, r.route_code, u.name as driver_name "
                "FROM trips t "
                "LEFT JOIN buses b ON t.bus_id = b.id "
                "LEFT JOIN routes r ON t.route_id = r.id "
                "LEFT JOIN drivers d ON t.driver_id = d.id "
                "LEFT JOIN users u ON d.user_id = u.id "
                "ORDER BY t.id DESC"
            );
            for (size_t i = 0; i < res.size(); ++i) {
                models::Trip t;
                t.id = res[i]["id"].as<int>();
                t.bus_id = res[i]["bus_id"].as<int>();
                t.route_id = res[i]["route_id"].as<int>();
                t.driver_id = res[i]["driver_id"].as<int>();
                t.started_at = res[i]["started_at"].as<std::string>();
                t.ended_at = res[i]["ended_at"].as<std::string>();
                t.status = res[i]["status"].as<std::string>();
                t.bus_number = res[i]["bus_number"].as<std::string>();
                t.route_name = res[i]["route_name"].as<std::string>();
                t.route_code = res[i]["route_code"].as<std::string>();
                t.driver_name = res[i]["driver_name"].as<std::string>();
                list.push_back(t);
            }
            if (!list.empty()) return list;
        }
    } catch (...) {}

    crow::LockGuard lock(trip_mutex);
    for (const auto& pair : mock_trips) {
        list.push_back(pair.second);
    }
    return list;
}

std::optional<models::Trip> TripRepository::find_by_id(int id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params(
                "SELECT t.id, t.bus_id, t.route_id, t.driver_id, t.started_at, t.ended_at, t.status, "
                "b.bus_number, r.route_name, r.route_code, u.name as driver_name "
                "FROM trips t "
                "LEFT JOIN buses b ON t.bus_id = b.id "
                "LEFT JOIN routes r ON t.route_id = r.id "
                "LEFT JOIN drivers d ON t.driver_id = d.id "
                "LEFT JOIN users u ON d.user_id = u.id "
                "WHERE t.id = $1", id
            );
            if (!res.empty()) {
                models::Trip t;
                t.id = res[0]["id"].as<int>();
                t.bus_id = res[0]["bus_id"].as<int>();
                t.route_id = res[0]["route_id"].as<int>();
                t.driver_id = res[0]["driver_id"].as<int>();
                t.started_at = res[0]["started_at"].as<std::string>();
                t.ended_at = res[0]["ended_at"].as<std::string>();
                t.status = res[0]["status"].as<std::string>();
                t.bus_number = res[0]["bus_number"].as<std::string>();
                t.route_name = res[0]["route_name"].as<std::string>();
                t.route_code = res[0]["route_code"].as<std::string>();
                t.driver_name = res[0]["driver_name"].as<std::string>();
                return t;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(trip_mutex);
    auto it = mock_trips.find(id);
    if (it != mock_trips.end()) return it->second;
    return std::nullopt;
}

std::vector<models::Trip> TripRepository::find_active_trips() {
    std::vector<models::Trip> list;
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec(
                "SELECT t.id, t.bus_id, t.route_id, t.driver_id, t.started_at, t.ended_at, t.status, "
                "b.bus_number, r.route_name, r.route_code, u.name as driver_name "
                "FROM trips t "
                "LEFT JOIN buses b ON t.bus_id = b.id "
                "LEFT JOIN routes r ON t.route_id = r.id "
                "LEFT JOIN drivers d ON t.driver_id = d.id "
                "LEFT JOIN users u ON d.user_id = u.id "
                "WHERE t.status IN ('in_progress', 'paused') "
                "ORDER BY t.started_at DESC"
            );
            for (size_t i = 0; i < res.size(); ++i) {
                models::Trip t;
                t.id = res[i]["id"].as<int>();
                t.bus_id = res[i]["bus_id"].as<int>();
                t.route_id = res[i]["route_id"].as<int>();
                t.driver_id = res[i]["driver_id"].as<int>();
                t.started_at = res[i]["started_at"].as<std::string>();
                t.ended_at = res[i]["ended_at"].as<std::string>();
                t.status = res[i]["status"].as<std::string>();
                t.bus_number = res[i]["bus_number"].as<std::string>();
                t.route_name = res[i]["route_name"].as<std::string>();
                t.route_code = res[i]["route_code"].as<std::string>();
                t.driver_name = res[i]["driver_name"].as<std::string>();
                list.push_back(t);
            }
            if (!list.empty()) return list;
        }
    } catch (...) {}

    crow::LockGuard lock(trip_mutex);
    for (const auto& pair : mock_trips) {
        if (pair.second.status == "in_progress" || pair.second.status == "paused") {
            list.push_back(pair.second);
        }
    }
    return list;
}

std::optional<models::Trip> TripRepository::find_active_trip_by_bus(int bus_id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params(
                "SELECT t.id, t.bus_id, t.route_id, t.driver_id, t.started_at, t.ended_at, t.status, "
                "b.bus_number, r.route_name, r.route_code, u.name as driver_name "
                "FROM trips t "
                "LEFT JOIN buses b ON t.bus_id = b.id "
                "LEFT JOIN routes r ON t.route_id = r.id "
                "LEFT JOIN drivers d ON t.driver_id = d.id "
                "LEFT JOIN users u ON d.user_id = u.id "
                "WHERE t.bus_id = $1 AND t.status IN ('in_progress', 'paused')", bus_id
            );
            if (!res.empty()) {
                models::Trip t;
                t.id = res[0]["id"].as<int>();
                t.bus_id = res[0]["bus_id"].as<int>();
                t.route_id = res[0]["route_id"].as<int>();
                t.driver_id = res[0]["driver_id"].as<int>();
                t.started_at = res[0]["started_at"].as<std::string>();
                t.ended_at = res[0]["ended_at"].as<std::string>();
                t.status = res[0]["status"].as<std::string>();
                t.bus_number = res[0]["bus_number"].as<std::string>();
                t.route_name = res[0]["route_name"].as<std::string>();
                t.route_code = res[0]["route_code"].as<std::string>();
                t.driver_name = res[0]["driver_name"].as<std::string>();
                return t;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(trip_mutex);
    for (const auto& pair : mock_trips) {
        if (pair.second.bus_id == bus_id && (pair.second.status == "in_progress" || pair.second.status == "paused")) {
            return pair.second;
        }
    }
    return std::nullopt;
}

std::optional<models::Trip> TripRepository::find_active_trip_by_driver(int driver_id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params(
                "SELECT t.id, t.bus_id, t.route_id, t.driver_id, t.started_at, t.ended_at, t.status, "
                "b.bus_number, r.route_name, r.route_code, u.name as driver_name "
                "FROM trips t "
                "LEFT JOIN buses b ON t.bus_id = b.id "
                "LEFT JOIN routes r ON t.route_id = r.id "
                "LEFT JOIN drivers d ON t.driver_id = d.id "
                "LEFT JOIN users u ON d.user_id = u.id "
                "WHERE t.driver_id = $1 AND t.status IN ('in_progress', 'paused')", driver_id
            );
            if (!res.empty()) {
                models::Trip t;
                t.id = res[0]["id"].as<int>();
                t.bus_id = res[0]["bus_id"].as<int>();
                t.route_id = res[0]["route_id"].as<int>();
                t.driver_id = res[0]["driver_id"].as<int>();
                t.started_at = res[0]["started_at"].as<std::string>();
                t.ended_at = res[0]["ended_at"].as<std::string>();
                t.status = res[0]["status"].as<std::string>();
                t.bus_number = res[0]["bus_number"].as<std::string>();
                t.route_name = res[0]["route_name"].as<std::string>();
                t.route_code = res[0]["route_code"].as<std::string>();
                t.driver_name = res[0]["driver_name"].as<std::string>();
                return t;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(trip_mutex);
    for (const auto& pair : mock_trips) {
        if (pair.second.driver_id == driver_id && (pair.second.status == "in_progress" || pair.second.status == "paused")) {
            return pair.second;
        }
    }
    return std::nullopt;
}

bool TripRepository::create(models::Trip& trip) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params(
                "INSERT INTO trips (bus_id, route_id, driver_id, started_at, status) "
                "VALUES ($1, $2, $3, NOW(), $4) RETURNING id, started_at",
                trip.bus_id, trip.route_id, trip.driver_id, trip.status
            );
            if (!res.empty()) {
                trip.id = res[0]["id"].as<int>();
                trip.started_at = res[0]["started_at"].as<std::string>();
                txn.commit();
                return true;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(trip_mutex);
    trip.id = next_trip_id++;
    trip.started_at = utils::current_timestamp();
    mock_trips[trip.id] = trip;
    return true;
}

bool TripRepository::update_status(int id, const std::string& status) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params("UPDATE trips SET status = $1 WHERE id = $2", status, id);
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(trip_mutex);
    auto it = mock_trips.find(id);
    if (it != mock_trips.end()) {
        it->second.status = status;
        return true;
    }
    return false;
}

bool TripRepository::end_trip(int id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params("UPDATE trips SET status = 'completed', ended_at = NOW() WHERE id = $1", id);
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(trip_mutex);
    auto it = mock_trips.find(id);
    if (it != mock_trips.end()) {
        it->second.status = "completed";
        it->second.ended_at = utils::current_timestamp();
        return true;
    }
    return false;
}

bool TripRepository::remove(int id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params("DELETE FROM trips WHERE id = $1", id);
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(trip_mutex);
    return mock_trips.erase(id) > 0;
}

} // namespace repositories
} // namespace transitx
