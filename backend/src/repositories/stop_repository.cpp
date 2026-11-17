// stop_repository.cpp - Stop repository implementation
#include "transitx/repositories/stop_repository.hpp"
#include "transitx/algorithms/haversine.hpp"
#include <map>
#include <algorithm>

namespace transitx {
namespace repositories {

StopRepository& StopRepository::get_instance() {
    static StopRepository instance;
    return instance;
}

static crow::Mutex stop_mutex;
static std::map<int, models::Stop> mock_stops = {
    {1, {1, "Clock Tower", 30.3165, 78.0322, 0.0}},
    {2, {2, "Paltan Bazaar", 30.3185, 78.0370, 0.0}},
    {3, {3, "Gandhi Road", 30.3148, 78.0400, 0.0}},
    {4, {4, "Nehru Colony", 30.3080, 78.0410, 0.0}},
    {5, {5, "Dalanwala", 30.3030, 78.0435, 0.0}},
    {6, {6, "ISBT Dehradun", 30.2724, 78.0462, 0.0}},
    {7, {7, "Rajpur Road", 30.3380, 78.0540, 0.0}},
    {8, {8, "Jakhan", 30.3480, 78.0580, 0.0}},
    {9, {9, "Malsi", 30.3600, 78.0620, 0.0}},
    {10, {10, "Kolhu Khet", 30.3750, 78.0680, 0.0}},
    {11, {11, "Barlowganj", 30.4200, 78.0730, 0.0}},
    {12, {12, "Mussoorie Library Chowk", 30.4560, 78.0750, 0.0}},
    {13, {13, "Clement Town", 30.2880, 78.0180, 0.0}},
    {14, {14, "FRI Gate", 30.3410, 78.0000, 0.0}},
    {15, {15, "Race Course", 30.3250, 78.0260, 0.0}},
    {16, {16, "Astley Hall", 30.3200, 78.0310, 0.0}},
    {17, {17, "Karanpur", 30.3220, 78.0350, 0.0}},
    {18, {18, "Ballupur Chowk", 30.3310, 78.0480, 0.0}},
    {19, {19, "Rispana Bridge", 30.3140, 78.0280, 0.0}},
    {20, {20, "Laxmi Road", 30.3100, 78.0220, 0.0}},
    {21, {21, "Chander Nagar", 30.3060, 78.0150, 0.0}},
    {22, {22, "Niranjanpur", 30.2980, 78.0100, 0.0}},
    {23, {23, "Premnagar", 30.2900, 78.0050, 0.0}},
    {24, {24, "Survey Chowk", 30.3195, 78.0290, 0.0}},
    {25, {25, "EC Road", 30.3230, 78.0360, 0.0}},
    {26, {26, "Saharanpur Chowk", 30.3270, 78.0430, 0.0}},
    {27, {27, "Patel Nagar", 30.3330, 78.0500, 0.0}},
    {28, {28, "Shimla Bypass", 30.3400, 78.0590, 0.0}},
    {29, {29, "Pacific Mall", 30.3570, 78.0670, 0.0}}
};
static int next_stop_id = 30;

std::vector<models::Stop> StopRepository::find_all() {
    std::vector<models::Stop> list;
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec("SELECT id, stop_name, latitude, longitude FROM stops ORDER BY id ASC");
            for (size_t i = 0; i < res.size(); ++i) {
                models::Stop s;
                s.id = res[i]["id"].as<int>();
                s.stop_name = res[i]["stop_name"].as<std::string>();
                s.latitude = res[i]["latitude"].as<double>();
                s.longitude = res[i]["longitude"].as<double>();
                list.push_back(s);
            }
            if (!list.empty()) return list;
        }
    } catch (...) {}

    crow::LockGuard lock(stop_mutex);
    for (const auto& pair : mock_stops) {
        list.push_back(pair.second);
    }
    return list;
}

std::optional<models::Stop> StopRepository::find_by_id(int id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params("SELECT id, stop_name, latitude, longitude FROM stops WHERE id = $1", id);
            if (!res.empty()) {
                models::Stop s;
                s.id = res[0]["id"].as<int>();
                s.stop_name = res[0]["stop_name"].as<std::string>();
                s.latitude = res[0]["latitude"].as<double>();
                s.longitude = res[0]["longitude"].as<double>();
                return s;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(stop_mutex);
    auto it = mock_stops.find(id);
    if (it != mock_stops.end()) return it->second;
    return std::nullopt;
}

std::vector<models::Stop> StopRepository::find_nearby(double lat, double lng, double radius_meters) {
    std::vector<models::Stop> list;
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            // PostGIS ST_DWithin and ST_Distance on GEOGRAPHY column
            pqxx::result res = txn.exec_params(
                "SELECT id, stop_name, latitude, longitude, "
                "ST_Distance(location, ST_SetSRID(ST_MakePoint($1, $2), 4326)::geography) AS dist_meters "
                "FROM stops "
                "WHERE ST_DWithin(location, ST_SetSRID(ST_MakePoint($1, $2), 4326)::geography, $3) "
                "ORDER BY dist_meters ASC",
                lng, lat, radius_meters
            );
            for (size_t i = 0; i < res.size(); ++i) {
                models::Stop s;
                s.id = res[i]["id"].as<int>();
                s.stop_name = res[i]["stop_name"].as<std::string>();
                s.latitude = res[i]["latitude"].as<double>();
                s.longitude = res[i]["longitude"].as<double>();
                s.distance_meters = res[i]["dist_meters"].as<double>();
                list.push_back(s);
            }
            if (!list.empty()) return list;
        }
    } catch (const std::exception& e) {
        LOG_WARN("PostGIS find_nearby exception: " << e.what() << ", falling back to Haversine.");
    }

    // Haversine fallback calculation
    crow::LockGuard lock(stop_mutex);
    for (const auto& pair : mock_stops) {
        double dist = algorithms::Haversine::calculate_distance_meters(lat, lng, pair.second.latitude, pair.second.longitude);
        if (dist <= radius_meters) {
            models::Stop s = pair.second;
            s.distance_meters = std::round(dist * 10.0) / 10.0;
            list.push_back(s);
        }
    }

    std::sort(list.begin(), list.end(), [](const models::Stop& a, const models::Stop& b) {
        return a.distance_meters < b.distance_meters;
    });

    return list;
}

bool StopRepository::create(models::Stop& stop) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params(
                "INSERT INTO stops (stop_name, latitude, longitude) VALUES ($1, $2, $3) RETURNING id",
                stop.stop_name, stop.latitude, stop.longitude
            );
            if (!res.empty()) {
                stop.id = res[0]["id"].as<int>();
                txn.commit();
                return true;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(stop_mutex);
    stop.id = next_stop_id++;
    mock_stops[stop.id] = stop;
    return true;
}

bool StopRepository::update(const models::Stop& stop) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params(
                "UPDATE stops SET stop_name = $1, latitude = $2, longitude = $3 WHERE id = $4",
                stop.stop_name, stop.latitude, stop.longitude, stop.id
            );
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(stop_mutex);
    auto it = mock_stops.find(stop.id);
    if (it != mock_stops.end()) {
        it->second = stop;
        return true;
    }
    return false;
}

bool StopRepository::remove(int id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params("DELETE FROM stops WHERE id = $1", id);
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(stop_mutex);
    return mock_stops.erase(id) > 0;
}

} // namespace repositories
} // namespace transitx
