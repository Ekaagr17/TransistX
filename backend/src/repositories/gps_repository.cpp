// gps_repository.cpp - GPS repository implementation
#include "transitx/repositories/gps_repository.hpp"
#include <map>

namespace transitx {
namespace repositories {

GpsRepository& GpsRepository::get_instance() {
    static GpsRepository instance;
    return instance;
}

static crow::Mutex gps_mutex;
// Seeded locations for active Dehradun buses
static std::map<int, models::BusLatestLocation> mock_latest = {
    {1, {1, 30.3165, 78.0322, 28.5, 145.0, 8.0, "2026-11-17 10:20:00", "UK07PA7001", "Demo: Clock Tower - ISBT", "DR001", 1, "active"}},
    {2, {2, 30.3420, 78.0560, 32.0, 20.0, 6.5, "2026-11-17 10:21:00", "UK07PA7002", "Demo: Rajpur Road - Mussoorie", "DR002", 2, "active"}},
    {3, {3, 30.3150, 78.0210, 24.0, 90.0, 9.0, "2026-11-17 10:19:30", "UK07PA7003", "Demo: Clement Town - Ballupur", "DR003", 3, "active"}},
    {4, {4, 30.3050, 78.0160, 30.0, 270.0, 7.2, "2026-11-17 10:22:15", "UK07PA7004", "Demo: Rispana - Premnagar", "DR004", 4, "active"}},
    {5, {5, 30.3300, 78.0460, 26.5, 45.0, 5.0, "2026-11-17 10:21:40", "UK07PA7005", "Demo: Survey Chowk - Pacific Mall", "DR005", 5, "active"}}
};

static std::vector<models::GpsUpdate> mock_gps_history;
static int64_t next_gps_id = 1;

bool GpsRepository::insert_update(models::GpsUpdate& update) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params(
                "INSERT INTO gps_updates (bus_id, trip_id, latitude, longitude, speed, heading, accuracy, recorded_at) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7, NOW()) RETURNING id, recorded_at",
                update.bus_id, update.trip_id, update.latitude, update.longitude, update.speed, update.heading, update.accuracy
            );
            if (!res.empty()) {
                update.id = res[0]["id"].as<int64_t>();
                update.recorded_at = res[0]["recorded_at"].as<std::string>();
                txn.commit();
            }
        }
    } catch (...) {}

    crow::LockGuard lock(gps_mutex);
    update.id = next_gps_id++;
    update.recorded_at = utils::current_timestamp();
    mock_gps_history.push_back(update);

    // Also update latest location
    models::BusLatestLocation latest;
    latest.bus_id = update.bus_id;
    latest.latitude = update.latitude;
    latest.longitude = update.longitude;
    latest.speed = update.speed;
    latest.heading = update.heading;
    latest.accuracy = update.accuracy;
    latest.updated_at = update.recorded_at;

    auto it = mock_latest.find(update.bus_id);
    if (it != mock_latest.end()) {
        latest.bus_number = it->second.bus_number;
        latest.route_name = it->second.route_name;
        latest.route_code = it->second.route_code;
        latest.route_id = it->second.route_id;
        latest.status = it->second.status;
    }
    mock_latest[update.bus_id] = latest;

    return true;
}

std::optional<models::BusLatestLocation> GpsRepository::get_latest_location(int bus_id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params(
                "SELECT bll.bus_id, bll.latitude, bll.longitude, bll.speed, bll.heading, bll.accuracy, bll.updated_at, "
                "b.bus_number, b.status, r.route_name, r.route_code, r.id as route_id "
                "FROM bus_latest_locations bll "
                "JOIN buses b ON bll.bus_id = b.id "
                "LEFT JOIN trips t ON (t.bus_id = b.id AND t.status IN ('in_progress', 'paused')) "
                "LEFT JOIN routes r ON t.route_id = r.id "
                "WHERE bll.bus_id = $1", bus_id
            );
            if (!res.empty()) {
                models::BusLatestLocation loc;
                loc.bus_id = res[0]["bus_id"].as<int>();
                loc.latitude = res[0]["latitude"].as<double>();
                loc.longitude = res[0]["longitude"].as<double>();
                loc.speed = res[0]["speed"].as<double>();
                loc.heading = res[0]["heading"].as<double>();
                loc.accuracy = res[0]["accuracy"].as<double>();
                loc.updated_at = res[0]["updated_at"].as<std::string>();
                loc.bus_number = res[0]["bus_number"].as<std::string>();
                loc.status = res[0]["status"].as<std::string>();
                loc.route_name = res[0]["route_name"].as<std::string>();
                loc.route_code = res[0]["route_code"].as<std::string>();
                loc.route_id = res[0]["route_id"].as<int>();
                return loc;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(gps_mutex);
    auto it = mock_latest.find(bus_id);
    if (it != mock_latest.end()) return it->second;
    return std::nullopt;
}

std::vector<models::BusLatestLocation> GpsRepository::get_all_latest_locations() {
    std::vector<models::BusLatestLocation> list;
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec(
                "SELECT bll.bus_id, bll.latitude, bll.longitude, bll.speed, bll.heading, bll.accuracy, bll.updated_at, "
                "b.bus_number, b.status, r.route_name, r.route_code, r.id as route_id "
                "FROM bus_latest_locations bll "
                "JOIN buses b ON bll.bus_id = b.id "
                "LEFT JOIN trips t ON (t.bus_id = b.id AND t.status IN ('in_progress', 'paused')) "
                "LEFT JOIN routes r ON t.route_id = r.id "
                "ORDER BY bll.bus_id ASC"
            );
            for (size_t i = 0; i < res.size(); ++i) {
                models::BusLatestLocation loc;
                loc.bus_id = res[i]["bus_id"].as<int>();
                loc.latitude = res[i]["latitude"].as<double>();
                loc.longitude = res[i]["longitude"].as<double>();
                loc.speed = res[i]["speed"].as<double>();
                loc.heading = res[i]["heading"].as<double>();
                loc.accuracy = res[i]["accuracy"].as<double>();
                loc.updated_at = res[i]["updated_at"].as<std::string>();
                loc.bus_number = res[i]["bus_number"].as<std::string>();
                loc.status = res[i]["status"].as<std::string>();
                loc.route_name = res[i]["route_name"].as<std::string>();
                loc.route_code = res[i]["route_code"].as<std::string>();
                loc.route_id = res[i]["route_id"].as<int>();
                list.push_back(loc);
            }
            if (!list.empty()) return list;
        }
    } catch (...) {}

    crow::LockGuard lock(gps_mutex);
    for (const auto& pair : mock_latest) {
        list.push_back(pair.second);
    }
    return list;
}

bool GpsRepository::upsert_latest_location(const models::BusLatestLocation& loc) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params(
                "INSERT INTO bus_latest_locations (bus_id, latitude, longitude, speed, heading, accuracy, updated_at) "
                "VALUES ($1, $2, $3, $4, $5, $6, NOW()) "
                "ON CONFLICT (bus_id) DO UPDATE SET "
                "latitude = EXCLUDED.latitude, longitude = EXCLUDED.longitude, "
                "speed = EXCLUDED.speed, heading = EXCLUDED.heading, accuracy = EXCLUDED.accuracy, updated_at = NOW()",
                loc.bus_id, loc.latitude, loc.longitude, loc.speed, loc.heading, loc.accuracy
            );
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(gps_mutex);
    mock_latest[loc.bus_id] = loc;
    return true;
}

std::vector<models::GpsUpdate> GpsRepository::get_updates_by_trip(int trip_id, int limit) {
    std::vector<models::GpsUpdate> list;
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params(
                "SELECT id, bus_id, trip_id, latitude, longitude, speed, heading, accuracy, recorded_at "
                "FROM gps_updates WHERE trip_id = $1 ORDER BY recorded_at DESC LIMIT $2",
                trip_id, limit
            );
            for (size_t i = 0; i < res.size(); ++i) {
                models::GpsUpdate u;
                u.id = res[i]["id"].as<int64_t>();
                u.bus_id = res[i]["bus_id"].as<int>();
                u.trip_id = res[i]["trip_id"].as<int>();
                u.latitude = res[i]["latitude"].as<double>();
                u.longitude = res[i]["longitude"].as<double>();
                u.speed = res[i]["speed"].as<double>();
                u.heading = res[i]["heading"].as<double>();
                u.accuracy = res[i]["accuracy"].as<double>();
                u.recorded_at = res[i]["recorded_at"].as<std::string>();
                list.push_back(u);
            }
            if (!list.empty()) return list;
        }
    } catch (...) {}

    crow::LockGuard lock(gps_mutex);
    for (auto it = mock_gps_history.rbegin(); it != mock_gps_history.rend(); ++it) {
        if (it->trip_id == trip_id) {
            list.push_back(*it);
            if (static_cast<int>(list.size()) >= limit) break;
        }
    }
    return list;
}

} // namespace repositories
} // namespace transitx
