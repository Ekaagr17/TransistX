// live_handler.cpp - Real-time WebSocket live tracking implementation
#include "transitx/websocket/live_handler.hpp"

namespace transitx {
namespace websocket {

LiveWebSocketHandler& LiveWebSocketHandler::get_instance() {
    static LiveWebSocketHandler instance;
    return instance;
}

void LiveWebSocketHandler::on_open(crow::websocket::connection& conn) {
    crow::LockGuard lock(mutex_);
    ClientSubscription sub;
    sub.client_id = conn.id;
    sub.connection_ptr = &conn;
    sub.subscribed_to_all = true; // Default: receive all bus updates
    clients_[conn.id] = sub;

    LOG_INFO("[WebSocket] Client connected: " << conn.id << " (Total: " << clients_.size() << ")");

    // Send connection acknowledgement
    nlohmann::json welcome = {
        {"type", "connection_ack"},
        {"client_id", conn.id},
        {"message", "Connected to TransitX Live Bus Tracking stream"}
    };
    conn.send_text(welcome.dump());
}

void LiveWebSocketHandler::on_message(crow::websocket::connection& conn, const std::string& data, bool /*is_binary*/) {
    try {
        nlohmann::json msg = nlohmann::json::parse(data);
        std::string action = msg.value("action", "");

        crow::LockGuard lock(mutex_);
        auto it = clients_.find(conn.id);
        if (it == clients_.end()) return;

        if (action == "subscribe_bus") {
            int bus_id = msg.value("bus_id", 0);
            if (bus_id > 0) {
                it->second.subscribed_bus_ids.insert(bus_id);
                it->second.subscribed_to_all = false;
                conn.send_text(nlohmann::json{
                    {"type", "subscribed"},
                    {"target", "bus"},
                    {"bus_id", bus_id}
                }.dump());
            }
        } else if (action == "subscribe_route") {
            int route_id = msg.value("route_id", 0);
            if (route_id > 0) {
                it->second.subscribed_route_ids.insert(route_id);
                it->second.subscribed_to_all = false;
                conn.send_text(nlohmann::json{
                    {"type", "subscribed"},
                    {"target", "route"},
                    {"route_id", route_id}
                }.dump());
            }
        } else if (action == "subscribe_all") {
            it->second.subscribed_to_all = true;
            it->second.subscribed_bus_ids.clear();
            it->second.subscribed_route_ids.clear();
            conn.send_text(nlohmann::json{
                {"type", "subscribed"},
                {"target", "all"}
            }.dump());
        } else if (action == "ping") {
            conn.send_text(nlohmann::json{{"type", "pong"}, {"timestamp", utils::current_timestamp()}}.dump());
        }
    } catch (const std::exception& e) {
        LOG_WARN("[WebSocket] Invalid message received: " << e.what());
    }
}

void LiveWebSocketHandler::on_close(crow::websocket::connection& conn, const std::string& reason) {
    crow::LockGuard lock(mutex_);
    clients_.erase(conn.id);
    LOG_INFO("[WebSocket] Client disconnected: " << conn.id << ", reason: " << reason << " (Remaining: " << clients_.size() << ")");
}

void LiveWebSocketHandler::on_error(crow::websocket::connection& conn) {
    crow::LockGuard lock(mutex_);
    clients_.erase(conn.id);
    LOG_WARN("[WebSocket] Client error: " << conn.id);
}

void LiveWebSocketHandler::broadcast_location_update(const models::BusLatestLocation& location) {
    crow::LockGuard lock(mutex_);
    if (clients_.empty()) return;

    nlohmann::json msg = {
        {"type", "location_update"},
        {"data", location.to_json()}
    };
    std::string payload = msg.dump();

    for (auto it = clients_.begin(); it != clients_.end();) {
        const auto& sub = it->second;
        bool should_send = false;

        if (sub.subscribed_to_all) {
            should_send = true;
        } else if (sub.subscribed_bus_ids.count(location.bus_id) > 0) {
            should_send = true;
        } else if (location.route_id > 0 && sub.subscribed_route_ids.count(location.route_id) > 0) {
            should_send = true;
        }

        if (should_send && sub.connection_ptr) {
            try {
                sub.connection_ptr->send_text(payload);
                ++it;
            } catch (...) {
                it = clients_.erase(it);
            }
        } else {
            ++it;
        }
    }
}

void LiveWebSocketHandler::broadcast_alert(const nlohmann::json& alert_data) {
    crow::LockGuard lock(mutex_);
    if (clients_.empty()) return;

    nlohmann::json msg = {
        {"type", "service_alert"},
        {"data", alert_data}
    };
    std::string payload = msg.dump();

    for (auto it = clients_.begin(); it != clients_.end();) {
        if (it->second.connection_ptr) {
            try {
                it->second.connection_ptr->send_text(payload);
                ++it;
            } catch (...) {
                it = clients_.erase(it);
            }
        } else {
            ++it;
        }
    }
}

size_t LiveWebSocketHandler::get_connected_clients_count() {
    crow::LockGuard lock(mutex_);
    return clients_.size();
}

} // namespace websocket
} // namespace transitx
