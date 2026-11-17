// live_handler.hpp - Real-time WebSocket live tracking handler
#pragma once
#ifndef TRANSITX_WEBSOCKET_LIVE_HANDLER_HPP
#define TRANSITX_WEBSOCKET_LIVE_HANDLER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "crow_all.h"
#include "transitx/models/gps.hpp"
#include "transitx/utils/logger.hpp"
#include "json.hpp"

namespace transitx {
namespace websocket {

struct ClientSubscription {
    std::string client_id;
    crow::websocket::connection* connection_ptr = nullptr;
    std::unordered_set<int> subscribed_bus_ids;   // Empty means all buses
    std::unordered_set<int> subscribed_route_ids; // Empty means all routes
    bool subscribed_to_all = true;
};

class LiveWebSocketHandler {
private:
    std::unordered_map<std::string, ClientSubscription> clients_;
    crow::Mutex mutex_;

    LiveWebSocketHandler() = default;

public:
    static LiveWebSocketHandler& get_instance();

    void on_open(crow::websocket::connection& conn);
    void on_message(crow::websocket::connection& conn, const std::string& data, bool is_binary);
    void on_close(crow::websocket::connection& conn, const std::string& reason);
    void on_error(crow::websocket::connection& conn);

    /**
     * Broadcast latest GPS location of a bus to interested subscribers.
     */
    void broadcast_location_update(const models::BusLatestLocation& location);

    /**
     * Broadcast an alert or service notification to all clients.
     */
    void broadcast_alert(const nlohmann::json& alert_data);

    size_t get_connected_clients_count();
};

} // namespace websocket
} // namespace transitx

#endif // TRANSITX_WEBSOCKET_LIVE_HANDLER_HPP
