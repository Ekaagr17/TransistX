/* eta_engine.c - ETA prediction engine implementation in pure C */
#include "eta_engine.h"
#include "haversine.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define DEFAULT_BUS_SPEED_KMH 28.0
#define MIN_SPEED_KMH 12.0
#define MAX_SPEED_KMH 65.0

int eta_calculate_bus_to_stop(int bus_id, int target_stop_id, BusEtaResult *out_result) {
    if (!out_result) return 0;
    memset(out_result, 0, sizeof(BusEtaResult));

    BusRecord *bus = data_store_find_bus_by_id(bus_id);
    if (!bus) return 0;

    RouteRecord *route = data_store_find_route_by_id(bus->route_id);
    if (!route || route->num_stops < 2) return 0;

    StopRecord *target_stop = data_store_find_stop_by_id(target_stop_id);
    if (!target_stop) return 0;

    out_result->bus_id = bus->id;
    strncpy(out_result->bus_number, bus->bus_number, sizeof(out_result->bus_number) - 1);
    out_result->route_id = bus->route_id;
    out_result->target_stop_id = target_stop_id;
    strncpy(out_result->target_stop_name, target_stop->name, sizeof(out_result->target_stop_name) - 1);

    /* Find index of target stop in route */
    int target_idx = -1;
    for (int i = 0; i < route->num_stops; i++) {
        if (route->stops[i].stop_id == target_stop_id) {
            target_idx = i;
            break;
        }
    }

    /* Effective speed */
    double speed = bus->speed;
    if (speed < MIN_SPEED_KMH) speed = DEFAULT_BUS_SPEED_KMH;
    if (speed > MAX_SPEED_KMH) speed = MAX_SPEED_KMH;

    /* Current wall clock time */
    time_t now = time(NULL);

    /* Calculate distance along consecutive stops */
    double total_distance_km = 0.0;
    double total_minutes = 0.0;

    /* Distance from bus current GPS to next stop */
    int next_stop_id = bus->next_stop_id;
    StopRecord *next_stop = data_store_find_stop_by_id(next_stop_id);
    if (next_stop) {
        double d_to_next = haversine_distance_km(bus->latitude, bus->longitude, next_stop->latitude, next_stop->longitude);
        total_distance_km += d_to_next;
        total_minutes += (d_to_next / speed) * 60.0;
    }

    /* Find next stop index in route */
    int next_stop_idx = 0;
    for (int i = 0; i < route->num_stops; i++) {
        if (route->stops[i].stop_id == next_stop_id) {
            next_stop_idx = i;
            break;
        }
    }

    /* Accumulate stops */
    out_result->num_stop_etas = 0;
    int curr_idx = next_stop_idx;
    int steps = 0;

    while (steps < route->num_stops) {
        int stop_id = route->stops[curr_idx].stop_id;
        StopRecord *s = data_store_find_stop_by_id(stop_id);
        if (s && out_result->num_stop_etas < MAX_ROUTE_STOPS) {
            StopEtaItem *item = &out_result->stop_etas[out_result->num_stop_etas++];
            item->stop_id = stop_id;
            strncpy(item->stop_name, s->name, sizeof(item->stop_name) - 1);
            item->distance_km = round(total_distance_km * 100.0) / 100.0;
            item->duration_minutes = round(total_minutes * 10.0) / 10.0;
            item->is_next_stop = (stop_id == next_stop_id);

            time_t arrival_time = now + (time_t)(total_minutes * 60.0);
            struct tm *tinfo = localtime(&arrival_time);
            if (tinfo) {
                strftime(item->arrival_time_str, sizeof(item->arrival_time_str), "%H:%M", tinfo);
            }

            if (stop_id == target_stop_id) {
                out_result->remaining_distance_km = item->distance_km;
                out_result->estimated_duration_minutes = item->duration_minutes;
                strncpy(out_result->estimated_arrival_str, item->arrival_time_str, sizeof(out_result->estimated_arrival_str) - 1);
            }
        }

        if (stop_id == target_stop_id) break;

        int next_idx = curr_idx + 1;
        if (next_idx >= route->num_stops) next_idx = 0; // Loop or end

        StopRecord *curr_s = data_store_find_stop_by_id(route->stops[curr_idx].stop_id);
        StopRecord *nxt_s = data_store_find_stop_by_id(route->stops[next_idx].stop_id);
        if (curr_s && nxt_s) {
            double seg_dist = haversine_distance_km(curr_s->latitude, curr_s->longitude, nxt_s->latitude, nxt_s->longitude);
            total_distance_km += seg_dist;
            total_minutes += (seg_dist / speed) * 60.0 + (route->stops[curr_idx].dwell_seconds / 60.0);
        }
        curr_idx = next_idx;
        steps++;
    }

    /* Fallback if target stop was behind */
    if (out_result->remaining_distance_km == 0.0 && target_stop) {
        double direct = haversine_distance_km(bus->latitude, bus->longitude, target_stop->latitude, target_stop->longitude);
        out_result->remaining_distance_km = round(direct * 100.0) / 100.0;
        out_result->estimated_duration_minutes = round((direct / speed * 60.0) * 10.0) / 10.0;
        time_t arrival_time = now + (time_t)(out_result->estimated_duration_minutes * 60.0);
        struct tm *tinfo = localtime(&arrival_time);
        if (tinfo) strftime(out_result->estimated_arrival_str, sizeof(out_result->estimated_arrival_str), "%H:%M", tinfo);
    }

    out_result->confidence = 0.95;
    return 1;
}

int eta_calculate_route_schedule(int route_id, int reference_bus_id, BusEtaResult *out_result) {
    RouteRecord *route = data_store_find_route_by_id(route_id);
    if (!route || route->num_stops < 2) return 0;

    int bus_id = reference_bus_id;
    if (bus_id <= 0) {
        /* Find first active bus on this route */
        BusRecord buses[MAX_BUSES];
        int count = data_store_get_all_buses(buses, MAX_BUSES);
        for (int i = 0; i < count; i++) {
            if (buses[i].route_id == route_id && strcmp(buses[i].status, "active") == 0) {
                bus_id = buses[i].id;
                break;
            }
        }
        if (bus_id <= 0) bus_id = 1;
    }

    int last_stop_id = route->stops[route->num_stops - 1].stop_id;
    return eta_calculate_bus_to_stop(bus_id, last_stop_id, out_result);
}
