// connection.hpp - PostgreSQL Connection and Connection Pool using libpqxx
#pragma once
#ifndef TRANSITX_DATABASE_CONNECTION_HPP
#define TRANSITX_DATABASE_CONNECTION_HPP

#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <pqxx/pqxx>
#include "crow_all.h"
#include "transitx/utils/config.hpp"
#include "transitx/utils/logger.hpp"

namespace transitx {
namespace database {

class ConnectionLease;

class DatabasePool {
private:
    std::string host_;
    int port_;
    std::string dbname_;
    std::string user_;
    std::string password_;
    int pool_size_;

    std::queue<std::shared_ptr<pqxx::connection>> available_connections_;
    std::vector<std::shared_ptr<pqxx::connection>> all_connections_;
    crow::Mutex pool_mutex_;
    bool is_initialized_ = false;

    DatabasePool() = default;

    std::string build_connection_string() const;
    std::shared_ptr<pqxx::connection> create_new_connection();

public:
    static DatabasePool& get_instance();

    void initialize(
        const std::string& host = utils::Config::db_host(),
        int port = utils::Config::db_port(),
        const std::string& dbname = utils::Config::db_name(),
        const std::string& user = utils::Config::db_user(),
        const std::string& password = utils::Config::db_password(),
        int pool_size = utils::Config::db_pool_size()
    );

    std::shared_ptr<pqxx::connection> acquire();
    void release(std::shared_ptr<pqxx::connection> conn);
    void close_all();

    bool is_healthy();

    // RAII connection helper
    ConnectionLease get_lease();
};

class ConnectionLease {
private:
    std::shared_ptr<pqxx::connection> conn_;
    DatabasePool& pool_;

public:
    ConnectionLease(std::shared_ptr<pqxx::connection> conn, DatabasePool& pool)
        : conn_(conn), pool_(pool) {}

    ~ConnectionLease() {
        if (conn_) {
            pool_.release(conn_);
        }
    }

    ConnectionLease(ConnectionLease&& other) noexcept
        : conn_(std::move(other.conn_)), pool_(other.pool_) {
        other.conn_ = nullptr;
    }

    ConnectionLease& operator=(ConnectionLease&& other) noexcept {
        if (this != &other) {
            if (conn_) pool_.release(conn_);
            conn_ = std::move(other.conn_);
            other.conn_ = nullptr;
        }
        return *this;
    }

    ConnectionLease(const ConnectionLease&) = delete;
    ConnectionLease& operator=(const ConnectionLease&) = delete;

    pqxx::connection& get() { return *conn_; }
    pqxx::connection& operator*() { return *conn_; }
    pqxx::connection* operator->() { return conn_.get(); }
};

} // namespace database
} // namespace transitx

#endif // TRANSITX_DATABASE_CONNECTION_HPP
