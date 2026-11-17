/* journey_planner.c - Smart Journey Planning & Best Pick Algorithm in pure C */
#include "journey_planner.h"
#include "haversine.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define WALKING_SPEED_KMH 4.5 /* Typical urban walking speed */

int plan_journey(const JourneyPlanRequest *req, JourneyPlanResult *out_result) {
    if (!req || !out_result) return 0;
    memset(out_result, 0, sizeof(JourneyPlanResult));

    StopRecord *start_s = data_store_find_stop_by_id(req->start_stop_id);
    StopRecord *end_s = data_store_find_stop_by_id(req->end_stop_id);
    if (!start_s || !end_s) return 0;

    out_result->start_stop_id = req->start_stop_id;
    strncpy(out_result->start_stop_name, start_s->name, sizeof(out_result->start_stop_name) - 1);
    out_result->end_stop_id = req->end_stop_id;
    strncpy(out_result->end_stop_name, end_s->name, sizeof(out_result->end_stop_name) - 1);

    /* Calculate walking distance from passenger to boarding stop if coordinates provided */
    double walk_dist_m = 0.0;
    double walk_minutes = 0.0;
    if (req->has_passenger_loc) {
        walk_dist_m = haversine_distance_meters(req->passenger_lat, req->passenger_lng, start_s->latitude, start_s->longitude);
        walk_minutes = (walk_dist_m / 1000.0 / WALKING_SPEED_KMH) * 60.0;
    }

    RouteRecord routes[MAX_ROUTES];
    int num_routes = data_store_get_all_routes(routes, MAX_ROUTES);

    BusRecord buses[MAX_BUSES];
    int num_buses = data_store_get_all_buses(buses, MAX_BUSES);

    time_t now = time(NULL);

    /* 1. Find all routes serving both stops in correct order */
    for (int r = 0; r < num_routes; r++) {
        RouteRecord *route = &routes[r];
        int start_order = -1, end_order = -1;

        for (int s = 0; s < route->num_stops; s++) {
            if (route->stops[s].stop_id == req->start_stop_id) start_order = route->stops[s].stop_order;
            if (route->stops[s].stop_id == req->end_stop_id) end_order = route->stops[s].stop_order;
        }

        /* Valid if both stops are present and start is before end */
        if (start_order != -1 && end_order != -1 && start_order < end_order) {
            /* Find buses on this route */
            for (int b = 0; b < num_buses; b++) {
                BusRecord *bus = &buses[b];
                if (bus->route_id == route->id && strcmp(bus->status, "active") == 0) {
                    if (out_result->num_candidates >= MAX_BUSES) break;

                    JourneyCandidate *cand = &out_result->candidates[out_result->num_candidates++];
                    cand->bus_id = bus->id;
                    strncpy(cand->bus_number, bus->bus_number, sizeof(cand->bus_number) - 1);
                    cand->route_id = route->id;
                    strncpy(cand->route_name, route->name, sizeof(cand->route_name) - 1);
                    strncpy(cand->route_code, route->code, sizeof(cand->route_code) - 1);

                    cand->walk_distance_meters = round(walk_dist_m * 10.0) / 10.0;
                    cand->walk_time_minutes = round(walk_minutes * 10.0) / 10.0;

                    /* Calculate ETA from bus to start stop */
                    BusEtaResult start_eta;
                    eta_calculate_bus_to_stop(bus->id, req->start_stop_id, &start_eta);
                    cand->wait_time_minutes = start_eta.estimated_duration_minutes;
                    strncpy(cand->boarding_arrival_time, start_eta.estimated_arrival_str, sizeof(cand->boarding_arrival_time) - 1);

                    /* Calculate transit time between start_stop and end_stop */
                    double transit_km = 0.0;
                    double transit_min = 0.0;
                    int start_collecting = 0;
                    cand->num_intermediate_stops = 0;

                    for (int s = 0; s < route->num_stops; s++) {
                        int s_id = route->stops[s].stop_id;
                        if (s_id == req->start_stop_id) {
                            start_collecting = 1;
                        }

                        if (start_collecting && cand->num_intermediate_stops < MAX_ROUTE_STOPS) {
                            StopRecord *stop_rec = data_store_find_stop_by_id(s_id);
                            if (stop_rec) {
                                StopEtaItem *item = &cand->intermediate_stops[cand->num_intermediate_stops++];
                                item->stop_id = s_id;
                                strncpy(item->stop_name, stop_rec->name, sizeof(item->stop_name) - 1);
                                item->distance_km = round(transit_km * 100.0) / 100.0;
                                item->duration_minutes = round((cand->wait_time_minutes + transit_min) * 10.0) / 10.0;

                                time_t arr_t = now + (time_t)(item->duration_minutes * 60.0);
                                struct tm *tinfo = localtime(&arr_t);
                                if (tinfo) strftime(item->arrival_time_str, sizeof(item->arrival_time_str), "%H:%M", tinfo);
                            }
                        }

                        if (s_id == req->end_stop_id) break;

                        if (start_collecting && s + 1 < route->num_stops) {
                            StopRecord *c_s = data_store_find_stop_by_id(route->stops[s].stop_id);
                            StopRecord *n_s = data_store_find_stop_by_id(route->stops[s + 1].stop_id);
                            if (c_s && n_s) {
                                double seg_d = haversine_distance_km(c_s->latitude, c_s->longitude, n_s->latitude, n_s->longitude);
                                transit_km += seg_d;
                                transit_min += (seg_d / 28.0) * 60.0 + (route->stops[s].dwell_seconds / 60.0);
                            }
                        }
                    }

                    cand->in_transit_minutes = round(transit_min * 10.0) / 10.0;
                    cand->total_journey_minutes = round((cand->walk_time_minutes + cand->wait_time_minutes + cand->in_transit_minutes) * 10.0) / 10.0;

                    time_t dest_arr_t = now + (time_t)((cand->wait_time_minutes + cand->in_transit_minutes) * 60.0);
                    struct tm *d_info = localtime(&dest_arr_t);
                    if (d_info) strftime(cand->destination_arrival_time, sizeof(cand->destination_arrival_time), "%H:%M", d_info);
                }
            }
        }
    }

    /* 2. Determine "BEST PICK" candidate */
    int best_idx = -1;
    double min_total_time = 1e9;

    for (int i = 0; i < out_result->num_candidates; i++) {
        JourneyCandidate *cand = &out_result->candidates[i];
        cand->is_best_pick = 0;

        /* Prefer bus that passenger can comfortably walk to in time and has earliest destination arrival */
        double penalty = 0.0;
        if (cand->walk_time_minutes > cand->wait_time_minutes) {
            /* Passenger will miss this bus unless they run */
            penalty += 100.0;
        }

        double score = cand->total_journey_minutes + penalty;
        if (score < min_total_time) {
            min_total_time = score;
            best_idx = i;
        }
    }

    if (best_idx >= 0 && best_idx < out_result->num_candidates) {
        out_result->candidates[best_idx].is_best_pick = 1;
        snprintf(out_result->candidates[best_idx].recommendation_reason,
                 sizeof(out_result->candidates[best_idx].recommendation_reason),
                 "BEST PICK: Arrives at %s in %.0f mins; fastest arrival at %s",
                 out_result->candidates[best_idx].boarding_arrival_time,
                 out_result->candidates[best_idx].wait_time_minutes,
                 out_result->candidates[best_idx].destination_arrival_time);
    }

    return out_result->num_candidates > 0;
}
