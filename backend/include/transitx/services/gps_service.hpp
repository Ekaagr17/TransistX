// gps_service.hpp - GPS processing and ingestion service header
#pragma once
#ifndef TRANSITX_SERVICES_GPS_SERVICE_HPP
#define TRANSITX_SERVICES_GPS_SERVICE_HPP

#include <vector>
#include <optional>
#include "transitx/models/gps.hpp"
#include "json.hpp"

namespace transitx {
namespace services {

struct GpsProcessResult {
    bool success = false;
    std::string error_message;
    models::GpsUpdate update;
};

class GpsService {
public:
    static GpsService& get_instance();

    GpsProcessResult process_update(int bus_id, int trip_id, double lat, double lng, double speed, double heading, double accuracy);
    nlohmann::json get_gps_status();
    std::vector<models::GpsUpdate> get_trip_history(int trip_id, int limit = 100);
};

} // namespace services
} // namespace transitx

#endif // TRANSITX_SERVICES_GPS_SERVICE_HPP
