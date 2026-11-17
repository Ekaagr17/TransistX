/* journey_planner.h - Smart Journey Planning & Route Matching Engine in pure C */
#ifndef TRANSITX_JOURNEY_PLANNER_H
#define TRANSITX_JOURNEY_PLANNER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "data_store.h"
#include "eta_engine.h"

typedef struct {
    int start_stop_id;
    int end_stop_id;
    double passenger_lat;
    double passenger_lng;
    int has_passenger_loc;
} JourneyPlanRequest;

typedef struct {
    int bus_id;
    char bus_number[24];
    int route_id;
    char route_name[64];
    char route_code[16];

    /* Boarding stop arrival */
    char boarding_arrival_time[16];   /* "17:52" */
    double wait_time_minutes;

    /* Destination arrival */
    char destination_arrival_time[16]; /* "18:15" */
    double in_transit_minutes;
    double total_journey_minutes;

    /* Passenger walking to boarding stop */
    double walk_distance_meters;
    double walk_time_minutes;

    /* Stop-by-stop schedule */
    int num_intermediate_stops;
    StopEtaItem intermediate_stops[MAX_ROUTE_STOPS];

    /* Best pick flag & explanation */
    int is_best_pick;
    char recommendation_reason[128];
} JourneyCandidate;

typedef struct {
    int start_stop_id;
    char start_stop_name[64];
    int end_stop_id;
    char end_stop_name[64];
    int num_candidates;
    JourneyCandidate candidates[MAX_BUSES];
} JourneyPlanResult;

/* Plan journey and select best pick */
int plan_journey(const JourneyPlanRequest *req, JourneyPlanResult *out_result);

#ifdef __cplusplus
}
#endif

#endif /* TRANSITX_JOURNEY_PLANNER_H */
