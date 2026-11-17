// alert_repository.cpp - Alert repository implementation
#include "transitx/repositories/alert_repository.hpp"
#include <map>

namespace transitx {
namespace repositories {

AlertRepository& AlertRepository::get_instance() {
    static AlertRepository instance;
    return instance;
}

static crow::Mutex alert_mutex;
static std::vector<models::ServiceAlert> mock_alerts = {
    {1, 2, "Heavy rain near Mussoorie, buses operating at reduced speeds.", "warning", "2026-11-17 08:00:00", "2026-11-18 08:00:00", "Demo: Rajpur Road - Mussoorie", "DR002"},
    {2, 1, "Normal schedules restored along Clock Tower to ISBT corridor.", "info", "2026-11-17 09:00:00", "2026-11-18 09:00:00", "Demo: Clock Tower - ISBT", "DR001"}
};
static int next_alert_id = 3;

std::vector<models::ServiceAlert> AlertRepository::get_active_alerts() {
    std::vector<models::ServiceAlert> list;
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec(
                "SELECT a.id, a.route_id, a.message, a.severity, a.created_at, a.expires_at, "
                "r.route_name, r.route_code "
                "FROM service_alerts a "
                "LEFT JOIN routes r ON a.route_id = r.id "
                "WHERE a.expires_at IS NULL OR a.expires_at > NOW() "
                "ORDER BY a.created_at DESC"
            );
            for (size_t i = 0; i < res.size(); ++i) {
                models::ServiceAlert a;
                a.id = res[i]["id"].as<int>();
                a.route_id = res[i]["route_id"].as<int>();
                a.message = res[i]["message"].as<std::string>();
                a.severity = res[i]["severity"].as<std::string>();
                a.created_at = res[i]["created_at"].as<std::string>();
                a.expires_at = res[i]["expires_at"].as<std::string>();
                a.route_name = res[i]["route_name"].as<std::string>();
                a.route_code = res[i]["route_code"].as<std::string>();
                list.push_back(a);
            }
            if (!list.empty()) return list;
        }
    } catch (...) {}

    crow::LockGuard lock(alert_mutex);
    return mock_alerts;
}

std::vector<models::ServiceAlert> AlertRepository::get_alerts_by_route(int route_id) {
    std::vector<models::ServiceAlert> list;
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params(
                "SELECT a.id, a.route_id, a.message, a.severity, a.created_at, a.expires_at, "
                "r.route_name, r.route_code "
                "FROM service_alerts a "
                "LEFT JOIN routes r ON a.route_id = r.id "
                "WHERE (a.route_id = $1 OR a.route_id IS NULL) "
                "AND (a.expires_at IS NULL OR a.expires_at > NOW()) "
                "ORDER BY a.created_at DESC", route_id
            );
            for (size_t i = 0; i < res.size(); ++i) {
                models::ServiceAlert a;
                a.id = res[i]["id"].as<int>();
                a.route_id = res[i]["route_id"].as<int>();
                a.message = res[i]["message"].as<std::string>();
                a.severity = res[i]["severity"].as<std::string>();
                a.created_at = res[i]["created_at"].as<std::string>();
                a.expires_at = res[i]["expires_at"].as<std::string>();
                a.route_name = res[i]["route_name"].as<std::string>();
                a.route_code = res[i]["route_code"].as<std::string>();
                list.push_back(a);
            }
            if (!list.empty()) return list;
        }
    } catch (...) {}

    crow::LockGuard lock(alert_mutex);
    for (const auto& a : mock_alerts) {
        if (a.route_id == route_id || a.route_id == 0) list.push_back(a);
    }
    return list;
}

bool AlertRepository::create(models::ServiceAlert& alert) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            pqxx::result res = txn.exec_params(
                "INSERT INTO service_alerts (route_id, message, severity, created_at, expires_at) "
                "VALUES ($1, $2, $3, NOW(), NOW() + INTERVAL '24 hours') RETURNING id, created_at, expires_at",
                alert.route_id, alert.message, alert.severity
            );
            if (!res.empty()) {
                alert.id = res[0]["id"].as<int>();
                alert.created_at = res[0]["created_at"].as<std::string>();
                alert.expires_at = res[0]["expires_at"].as<std::string>();
                txn.commit();
                return true;
            }
        }
    } catch (...) {}

    crow::LockGuard lock(alert_mutex);
    alert.id = next_alert_id++;
    alert.created_at = utils::current_timestamp();
    mock_alerts.push_back(alert);
    return true;
}

bool AlertRepository::remove(int id) {
    try {
        auto lease = database::DatabasePool::get_instance().get_lease();
        if (lease->is_open()) {
            pqxx::work txn(*lease);
            txn.exec_params("DELETE FROM service_alerts WHERE id = $1", id);
            txn.commit();
            return true;
        }
    } catch (...) {}

    crow::LockGuard lock(alert_mutex);
    for (auto it = mock_alerts.begin(); it != mock_alerts.end(); ++it) {
        if (it->id == id) {
            mock_alerts.erase(it);
            return true;
        }
    }
    return false;
}

} // namespace repositories
} // namespace transitx
