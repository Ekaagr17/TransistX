// alert_service.hpp - Service alerts management service header
#pragma once
#ifndef TRANSITX_SERVICES_ALERT_SERVICE_HPP
#define TRANSITX_SERVICES_ALERT_SERVICE_HPP

#include <vector>
#include "transitx/models/alert.hpp"

namespace transitx {
namespace services {

class AlertService {
public:
    static AlertService& get_instance();

    std::vector<models::ServiceAlert> get_active_alerts();
    std::vector<models::ServiceAlert> get_alerts_by_route(int route_id);
    bool create_alert(models::ServiceAlert& alert);
    bool remove_alert(int id);
};

} // namespace services
} // namespace transitx

#endif // TRANSITX_SERVICES_ALERT_SERVICE_HPP
