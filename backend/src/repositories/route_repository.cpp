// route_repository.cpp - Route repository implementation
#include "transitx/repositories/route_repository.hpp"
#include "transitx/repositories/stop_repository.hpp"
#include <map>

namespace transitx {
namespace repositories {

RouteRepository& RouteRepository::get_instance() {
    static RouteRepository instance;
    return instance;
}

static crow::Mutex route_mutex;
static std::map<int, models::Route> mock_routes = {
    {1, {1, "Demo: Clock Tower - ISBT", "DR001", "Clock Tower", "ISBT Dehradun", "active"}},
    {2, {2, "Demo: Rajpur Road - Mussoorie", "DR002", "Rajpur Road", "Mussoorie", "active"}},
    {3, {3, "Demo: Clement Town - Ballupur", "DR003", "Clement Town", "Ballupur Chowk", "active"}},
    {4, {4, "Demo: Rispana - Premnagar", "DR004", "Rispana Bridge", "Premnagar", "active"}},
    {5, {5, "Demo: Survey Chowk - Pacific Mall", "DR005", "Survey Chowk", "Pacific Mall", "active"}}
};

// Route stops mapping: route_id -> list of RouteStop
static std::map<int, std::vector<models::RouteStop>> mock_route_stops = {
    {1, {
        {1, 1, 1, 1, 60, "Clock Tower", 30.3165, 78.0322},
        {2, 1, 2, 2, 30, "Paltan Bazaar", 30.3185, 78.0370},
        {3, 1, 3, 3, 30, "Gandhi Road", 30.3148, 78.0400},
        {4, 1, 4, 4, 30, "Nehru Colony", 30.3080, 78.0410},
        {5, 1, 5, 5, 30, "Dalanwala", 30.3030, 78.0435},
        {6, 1, 6, 6, 60, "ISBT Dehradun", 30.2724, 78.0462}
    }},
    {2, {
        {7, 2, 7, 1, 60, "Rajpur Road", 30.3380, 78.0540},
        {8, 2, 8, 2, 30, "Jakhan", 30.3480, 78.0580},
        {9, 2, 9, 3, 30, "Malsi", 30.3600, 78.0620},
        {10, 2, 10, 4, 30, "Kolhu Khet", 30.3750, 78.0680},
        {11, 2, 11, 5, 30, "Barlowganj", 30.4200, 78.0730},
        {12, 2, 12, 6, 60, "Mussoorie Library Chowk", 30.4560, 78.0750}
    }},
    {3, {
        {13, 3, 13, 1, 60, "Clement Town", 30.2880, 78.0180},
        {14, 3, 14, 2, 30, "FRI Gate", 30.3410, 78.0000},
        {15, 3, 15, 3, 30, "Race Course", 30.3250, 78.0260},
        {16, 3, 16, 4, 30, "Astley Hall", 30.3200, 78.0310},
        {17, 3, 17, 5, 30, "Karanpur", 30.3220, 78.0350},
        {18, 3, 18, 6, 60, "Ballupur Chowk", 30.3310, 78.0480}
    }},
    {4, {
        {19, 4, 19, 1, 60, "Rispana Bridge", 30.3140, 78.0280},
        {20, 4, 20, 2, 30, "Laxmi Road", 30.3100, 78.0220},
        {21, 4, 21, 3, 30, "Chander Nagar", 30.3060, 78.0150},
        {22, 4, 22, 4, 30, "Niranjanpur", 30.2980, 78.0100},
        {23, 4, 23, 5, 60, "Premnagar", 30.2900, 78.0050}
    }},
    {5, {
        {24, 5, 24, 1, 60, "Survey Chowk", 30.3195, 78.0290},
        {25, 5, 25, 2, 30, "EC Road", 30.3230, 78.0360},
        {26, 5, 26, 3, 30, "Saharanpur Chowk", 30.3270, 78.0430},
        {27, 5, 27, 4, 30, "Patel Nagar", 30.3330, 78.0500},
        {28, 5, 28, 5, 30, "Shimla Bypass", 30.3400, 78.0590},
        {29, 5, 29, 6, 60, "Pacific Mall", 30.3570, 78.0670}
    }}
};
static int next_route_id = 6;

std::vector<models::Route> RouteRepository::find_all() {
    std::vector<models::Route> list;
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec("SELECT id, route_name, route_code, start_point, end_point, status FROM routes ORDER BY id ASC");
            for (size_t i = 0; i < res.size(); ++i) {
                models::Route r;
                r.id = res[i]["id"].as<int>();
                r.route_name = res[i]["route_name"].as<std::string>();
                r.route_code = res[i]["route_code"].as<std::string>();
                r.start_point = res[i]["start_point"].as<std::string>();
                r.end_point = res[i]["end_point"].as<std::string>();
                r.status = res[i]["status"].as<std::string>();
                list.push_back(r);
            }
            if (!list.empty()) return list;
        }
    } catch (...) {}

    crow::LockGuard lock(route_mutex);
    for (const auto& pair : mock_routes) {
        list.push_back(pair.second);
    }
    return list;
}

std::optional<models::Route> RouteRepository::find_by_id(int id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params("SELECT id, route_name, route_code, start_point, end_point, status FROM routes WHERE id = $1", id);
            if (!res.empty()) {
                models::Route r;
                r.id = res[0]["id"].as<int>();
                r.route_name = res[0]["route_name"].as<std::string>();
                r.route_code = res[0]["route_code"].as<std::string>();
                r.start_point = res[0]["start_point"].as<std::string>();
                r.end_point = res[0]["end_point"].as<std::string>();
                r.status = res[0]["status"].as<std::string>();
                return r;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(route_mutex);
    auto it = mock_routes.find(id);
    if (it != mock_routes.end()) return it->second;
    return std::nullopt;
}

std::optional<models::Route> RouteRepository::find_by_code(const std::string& code) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params("SELECT id, route_name, route_code, start_point, end_point, status FROM routes WHERE route_code = $1", code);
            if (!res.empty()) {
                models::Route r;
                r.id = res[0]["id"].as<int>();
                r.route_name = res[0]["route_name"].as<std::string>();
                r.route_code = res[0]["route_code"].as<std::string>();
                r.start_point = res[0]["start_point"].as<std::string>();
                r.end_point = res[0]["end_point"].as<std::string>();
                r.status = res[0]["status"].as<std::string>();
                return r;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(route_mutex);
    for (const auto& pair : mock_routes) {
        if (pair.second.route_code == code) return pair.second;
    }
    return std::nullopt;
}

std::vector<models::RouteStop> RouteRepository::get_route_stops(int route_id) {
    std::vector<models::RouteStop> list;
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params(
                "SELECT rs.id, rs.route_id, rs.stop_id, rs.stop_order, rs.estimated_stop_duration, "
                "s.stop_name, s.latitude, s.longitude "
                "FROM route_stops rs "
                "JOIN stops s ON rs.stop_id = s.id "
                "WHERE rs.route_id = $1 "
                "ORDER BY rs.stop_order ASC", route_id
            );
            for (size_t i = 0; i < res.size(); ++i) {
                models::RouteStop rs;
                rs.id = res[i]["id"].as<int>();
                rs.route_id = res[i]["route_id"].as<int>();
                rs.stop_id = res[i]["stop_id"].as<int>();
                rs.stop_order = res[i]["stop_order"].as<int>();
                rs.estimated_stop_duration = res[i]["estimated_stop_duration"].as<int>();
                rs.stop_name = res[i]["stop_name"].as<std::string>();
                rs.latitude = res[i]["latitude"].as<double>();
                rs.longitude = res[i]["longitude"].as<double>();
                list.push_back(rs);
            }
            if (!list.empty()) return list;
        }
    } catch (...) {}

    crow::LockGuard lock(route_mutex);
    auto it = mock_route_stops.find(route_id);
    if (it != mock_route_stops.end()) return it->second;
    return list;
}

bool RouteRepository::create(models::Route& route) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params(
                "INSERT INTO routes (route_name, route_code, start_point, end_point, status) VALUES ($1, $2, $3, $4, $5) RETURNING id",
                route.route_name, route.route_code, route.start_point, route.end_point, route.status
            );
            if (!res.empty()) {
                route.id = res[0]["id"].as<int>();
                txn.commit();
                return true;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(route_mutex);
    route.id = next_route_id++;
    mock_routes[route.id] = route;
    return true;
}

bool RouteRepository::update(const models::Route& route) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params(
                "UPDATE routes SET route_name = $1, route_code = $2, start_point = $3, end_point = $4, status = $5 WHERE id = $6",
                route.route_name, route.route_code, route.start_point, route.end_point, route.status, route.id
            );
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(route_mutex);
    auto it = mock_routes.find(route.id);
    if (it != mock_routes.end()) {
        it->second = route;
        return true;
    }
    return false;
}

bool RouteRepository::remove(int id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params("DELETE FROM routes WHERE id = $1", id);
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(route_mutex);
    return mock_routes.erase(id) > 0;
}

bool RouteRepository::add_stop_to_route(int route_id, int stop_id, int stop_order, int estimated_stop_duration) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params(
                "INSERT INTO route_stops (route_id, stop_id, stop_order, estimated_stop_duration) "
                "VALUES ($1, $2, $3, $4) "
                "ON CONFLICT (route_id, stop_order) DO UPDATE SET stop_id = EXCLUDED.stop_id, estimated_stop_duration = EXCLUDED.estimated_stop_duration",
                route_id, stop_id, stop_order, estimated_stop_duration
            );
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(route_mutex);
    models::RouteStop rs;
    rs.id = static_cast<int>(mock_route_stops[route_id].size() + 1);
    rs.route_id = route_id;
    rs.stop_id = stop_id;
    rs.stop_order = stop_order;
    rs.estimated_stop_duration = estimated_stop_duration;
    mock_route_stops[route_id].push_back(rs);
    return true;
}

} // namespace repositories
} // namespace transitx
