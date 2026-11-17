// alert_repository.hpp - Alert repository header
#pragma once
#ifndef TRANSITX_REPOSITORIES_ALERT_REPOSITORY_HPP
#define TRANSITX_REPOSITORIES_ALERT_REPOSITORY_HPP

#include <vector>
#include <string>
#include <optional>
#include "transitx/models/alert.hpp"
#include "transitx/database/connection.hpp"

namespace transitx {
namespace repositories {

class AlertRepository {
public:
    static AlertRepository& get_instance();

    std::vector<models::ServiceAlert> get_active_alerts();
    std::vector<models::ServiceAlert> get_alerts_by_route(int route_id);
    bool create(models::ServiceAlert& alert);
    bool remove(int id);
};

} // namespace repositories
} // namespace transitx

#endif // TRANSITX_REPOSITORIES_ALERT_REPOSITORY_HPP
