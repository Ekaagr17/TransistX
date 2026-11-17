// bus_service.hpp - Bus management and status service
#pragma once
#ifndef TRANSITX_SERVICES_BUS_SERVICE_HPP
#define TRANSITX_SERVICES_BUS_SERVICE_HPP

#include <vector>
#include <optional>
#include "transitx/models/bus.hpp"
#include "transitx/models/gps.hpp"
#include "transitx/algorithms/eta.hpp"

namespace transitx {
namespace services {

class BusService {
public:
    static BusService& get_instance();

    std::vector<models::Bus> get_all_buses();
    std::optional<models::Bus> get_bus_by_id(int id);
    std::optional<models::BusLatestLocation> get_bus_location(int bus_id);
    std::vector<models::BusLatestLocation> get_all_bus_locations();
    std::optional<algorithms::EtaResult> get_bus_eta(int bus_id, int stop_id);
};

} // namespace services
} // namespace transitx

#endif // TRANSITX_SERVICES_BUS_SERVICE_HPP
