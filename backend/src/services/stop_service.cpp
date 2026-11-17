// stop_service.cpp - Stop service implementation
#include "transitx/services/stop_service.hpp"
#include "transitx/repositories/stop_repository.hpp"

namespace transitx {
namespace services {

StopService& StopService::get_instance() {
    static StopService instance;
    return instance;
}

std::vector<models::Stop> StopService::get_all_stops() {
    return repositories::StopRepository::get_instance().find_all();
}

std::optional<models::Stop> StopService::get_stop_by_id(int id) {
    return repositories::StopRepository::get_instance().find_by_id(id);
}

std::vector<models::Stop> StopService::find_nearby_stops(double lat, double lng, double radius_meters) {
    if (radius_meters <= 0.0) radius_meters = 1000.0; // Default 1km
    if (radius_meters > 50000.0) radius_meters = 50000.0; // Max 50km
    return repositories::StopRepository::get_instance().find_nearby(lat, lng, radius_meters);
}

bool StopService::create_stop(models::Stop& stop) {
    return repositories::StopRepository::get_instance().create(stop);
}

} // namespace services
} // namespace transitx
