// stop_service.hpp - Stop service header
#pragma once
#ifndef TRANSITX_SERVICES_STOP_SERVICE_HPP
#define TRANSITX_SERVICES_STOP_SERVICE_HPP

#include <vector>
#include <optional>
#include "transitx/models/stop.hpp"

namespace transitx {
namespace services {

class StopService {
public:
    static StopService& get_instance();

    std::vector<models::Stop> get_all_stops();
    std::optional<models::Stop> get_stop_by_id(int id);
    std::vector<models::Stop> find_nearby_stops(double lat, double lng, double radius_meters);
    bool create_stop(models::Stop& stop);
};

} // namespace services
} // namespace transitx

#endif // TRANSITX_SERVICES_STOP_SERVICE_HPP
