// alert_service.cpp - Alert service implementation
#include "transitx/services/alert_service.hpp"
#include "transitx/repositories/alert_repository.hpp"
#include "transitx/websocket/live_handler.hpp"

namespace transitx {
namespace services {

AlertService& AlertService::get_instance() {
    static AlertService instance;
    return instance;
}

std::vector<models::ServiceAlert> AlertService::get_active_alerts() {
    return repositories::AlertRepository::get_instance().get_active_alerts();
}

std::vector<models::ServiceAlert> AlertService::get_alerts_by_route(int route_id) {
    return repositories::AlertRepository::get_instance().get_alerts_by_route(route_id);
}

bool AlertService::create_alert(models::ServiceAlert& alert) {
    if (alert.message.empty()) return false;
    if (repositories::AlertRepository::get_instance().create(alert)) {
        websocket::LiveWebSocketHandler::get_instance().broadcast_alert(alert.to_json());
        return true;
    }
    return false;
}

bool AlertService::remove_alert(int id) {
    return repositories::AlertRepository::get_instance().remove(id);
}

} // namespace services
} // namespace transitx
