/* eta_engine.h - ETA prediction engine in pure C */
#ifndef TRANSITX_ETA_ENGINE_H
#define TRANSITX_ETA_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "data_store.h"

typedef struct {
    int stop_id;
    char stop_name[64];
    double distance_km;
    double duration_minutes;
    char arrival_time_str[16]; // "HH:MM"
    int is_next_stop;
} StopEtaItem;

typedef struct {
    int bus_id;
    char bus_number[24];
    int route_id;
    int target_stop_id;
    char target_stop_name[64];
    double remaining_distance_km;
    double estimated_duration_minutes;
    char estimated_arrival_str[16]; // "HH:MM"
    double confidence;
    int num_stop_etas;
    StopEtaItem stop_etas[MAX_ROUTE_STOPS];
} BusEtaResult;

/* Calculate ETA for a specific bus to a specific stop */
int eta_calculate_bus_to_stop(int bus_id, int target_stop_id, BusEtaResult *out_result);

/* Calculate ETA schedule for every stop along the route from current clock time */
int eta_calculate_route_schedule(int route_id, int reference_bus_id, BusEtaResult *out_result);

#ifdef __cplusplus
}
#endif

#endif /* TRANSITX_ETA_ENGINE_H */
