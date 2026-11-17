// bus_service.cpp - Bus service implementation
#include "transitx/services/bus_service.hpp"
#include "transitx/services/eta_service.hpp"
#include "transitx/repositories/bus_repository.hpp"
#include "transitx/repositories/gps_repository.hpp"

namespace transitx {
namespace services {

BusService& BusService::get_instance() {
    static BusService instance;
    return instance;
}

std::vector<models::Bus> BusService::get_all_buses() {
    return repositories::BusRepository::get_instance().find_all();
}

std::optional<models::Bus> BusService::get_bus_by_id(int id) {
    return repositories::BusRepository::get_instance().find_by_id(id);
}

std::optional<models::BusLatestLocation> BusService::get_bus_location(int bus_id) {
    return repositories::GpsRepository::get_instance().get_latest_location(bus_id);
}

std::vector<models::BusLatestLocation> BusService::get_all_bus_locations() {
    return repositories::GpsRepository::get_instance().get_all_latest_locations();
}

std::optional<algorithms::EtaResult> BusService::get_bus_eta(int bus_id, int stop_id) {
    return EtaService::get_instance().calculate_eta_for_bus(bus_id, stop_id);
}

} // namespace services
} // namespace transitx
