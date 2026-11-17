// connection.cpp - Database connection pool implementation
#include "transitx/database/connection.hpp"
#include <sstream>

namespace transitx {
namespace database {

DatabasePool& DatabasePool::get_instance() {
    static DatabasePool instance;
    return instance;
}

std::string DatabasePool::build_connection_string() const {
    std::ostringstream ss;
    ss << "host=" << host_
       << " port=" << port_
       << " dbname=" << dbname_
       << " user=" << user_
       << " password=" << password_
       << " connect_timeout=5";
    return ss.str();
}

std::shared_ptr<pqxx::connection> DatabasePool::create_new_connection() {
    try {
        std::string conn_str = build_connection_string();
        auto conn = std::make_shared<pqxx::connection>(conn_str);
        return conn;
    } catch (const std::exception& e) {
        LOG_WARN("Could not establish PostgreSQL connection: " << e.what());
        return std::make_shared<pqxx::connection>("");
    }
}

void DatabasePool::initialize(
    const std::string& host,
    int port,
    const std::string& dbname,
    const std::string& user,
    const std::string& password,
    int pool_size
) {
    crow::LockGuard lock(pool_mutex_);
    if (is_initialized_) return;

    host_ = host;
    port_ = port;
    dbname_ = dbname;
    user_ = user;
    password_ = password;
    pool_size_ = pool_size > 0 ? pool_size : 5;

    LOG_INFO("Initializing PostgreSQL connection pool (" << pool_size_ << " connections) to " << host_ << ":" << port_ << "/" << dbname_);

    for (int i = 0; i < pool_size_; ++i) {
        auto conn = create_new_connection();
        all_connections_.push_back(conn);
        available_connections_.push(conn);
    }

    is_initialized_ = true;
    LOG_INFO("Database pool successfully initialized.");
}

std::shared_ptr<pqxx::connection> DatabasePool::acquire() {
    crow::LockGuard lock(pool_mutex_);
    if (available_connections_.empty()) {
        // Create an ephemeral connection if pool is depleted
        return create_new_connection();
    }
    auto conn = available_connections_.front();
    available_connections_.pop();
    return conn;
}

void DatabasePool::release(std::shared_ptr<pqxx::connection> conn) {
    crow::LockGuard lock(pool_mutex_);
    if (conn) {
        available_connections_.push(conn);
    }
}

void DatabasePool::close_all() {
    crow::LockGuard lock(pool_mutex_);
    while (!available_connections_.empty()) {
        available_connections_.pop();
    }
    for (auto& conn : all_connections_) {
        if (conn && conn->is_open()) {
            conn->close();
        }
    }
    all_connections_.clear();
    is_initialized_ = false;
}

bool DatabasePool::is_healthy() {
    crow::LockGuard lock(pool_mutex_);
    if (!is_initialized_ || all_connections_.empty()) return false;
    for (const auto& conn : all_connections_) {
        if (conn && conn->is_open()) return true;
    }
    return false;
}

ConnectionLease DatabasePool::get_lease() {
    return ConnectionLease(acquire(), *this);
}

} // namespace database
} // namespace transitx
