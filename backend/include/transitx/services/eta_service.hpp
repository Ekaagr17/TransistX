// eta_service.hpp - ETA calculation service header
#pragma once
#ifndef TRANSITX_SERVICES_ETA_SERVICE_HPP
#define TRANSITX_SERVICES_ETA_SERVICE_HPP

#include <optional>
#include "transitx/algorithms/eta.hpp"

namespace transitx {
namespace services {

class EtaService {
public:
    static EtaService& get_instance();

    std::optional<algorithms::EtaResult> calculate_eta_for_bus(int bus_id, int stop_id);
};

} // namespace services
} // namespace transitx

#endif // TRANSITX_SERVICES_ETA_SERVICE_HPP
