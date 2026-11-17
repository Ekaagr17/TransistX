/* http_server.c - Pure C multi-threaded REST HTTP Server for TransitX */
#include "http_server.h"
#include "data_store.h"
#include "eta_engine.h"
#include "journey_planner.h"
#include "websocket.h"
#include "haversine.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <process.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET http_socket_t;
    #define CLOSE_SOCK(s) closesocket(s)
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <pthread.h>
    typedef int http_socket_t;
    #define CLOSE_SOCK(s) close(s)
#endif

static volatile int g_server_running = 1;
static http_socket_t g_listen_socket = 0;

static void send_response(http_socket_t sock, int status_code, const char *status_msg, const char *content_type, const char *body) {
    size_t body_len = body ? strlen(body) : 0;
    char header[512];
    int h_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Authorization, Content-Type, Accept\r\n"
        "Connection: close\r\n\r\n",
        status_code, status_msg, content_type, body_len
    );

    send(sock, header, h_len, 0);
    if (body_len > 0) {
        send(sock, body, (int)body_len, 0);
    }
}

static void send_json_response(http_socket_t sock, int status_code, cJSON *json_obj) {
    char *rendered = cJSON_PrintUnformatted(json_obj);
    send_response(sock, status_code, status_code == 200 ? "OK" : (status_code == 201 ? "Created" : "Error"), "application/json", rendered);
    if (rendered) free(rendered);
    if (json_obj) cJSON_Delete(json_obj);
}

/* Helper to get query param */
static int get_query_param(const char *query, const char *param, char *out_val, size_t max_len) {
    if (!query || !param) return 0;
    char search[64];
    snprintf(search, sizeof(search), "%s=", param);
    const char *p = strstr(query, search);
    if (!p) return 0;
    p += strlen(search);
    size_t i = 0;
    while (*p && *p != '&' && *p != ' ' && i < max_len - 1) {
        out_val[i++] = *p++;
    }
    out_val[i] = '\0';
    return 1;
}

#ifdef _WIN32
static unsigned int __stdcall http_worker_thread(void *arg)
#else
static void* http_worker_thread(void *arg)
#endif
{
    http_socket_t client_sock = (http_socket_t)(intptr_t)arg;
    char buffer[4096];
    int received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        CLOSE_SOCK(client_sock);
#ifdef _WIN32
        return 0;
#else
        return NULL;
#endif
    }
    buffer[received] = '\0';

    /* Check for WebSocket Upgrade */
    if (strstr(buffer, "Upgrade: websocket") || strstr(buffer, "upgrade: websocket")) {
        if (websocket_handle_upgrade((ws_socket_t)client_sock, buffer)) {
            /* Socket ownership passed to websocket reader thread */
#ifdef _WIN32
            return 0;
#else
            return NULL;
#endif
        }
    }

    /* Parse request line */
    char method[16] = {0}, uri[256] = {0}, proto[16] = {0};
    sscanf(buffer, "%15s %255s %15s", method, uri, proto);

    /* Split URI and Query string */
    char path[256] = {0};
    char *q_mark = strchr(uri, '?');
    char query[256] = {0};
    if (q_mark) {
        size_t path_len = q_mark - uri;
        strncpy(path, uri, path_len);
        path[path_len] = '\0';
        strncpy(query, q_mark + 1, sizeof(query) - 1);
    } else {
        strncpy(path, uri, sizeof(path) - 1);
    }

    /* Handle CORS Preflight */
    if (strcmp(method, "OPTIONS") == 0) {
        send_response(client_sock, 204, "No Content", "text/plain", "");
        CLOSE_SOCK(client_sock);
#ifdef _WIN32
        return 0;
#else
        return NULL;
#endif
    }

    /* Extract body if POST/PUT */
    const char *body_ptr = strstr(buffer, "\r\n\r\n");
    if (body_ptr) body_ptr += 4;
    else body_ptr = "";

    /* Route Dispatch */

    /* 1. GET / */
    if (strcmp(method, "GET") == 0 && strcmp(path, "/") == 0) {
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "name", "TransitX Pure C Backend");
        cJSON_AddStringToObject(root, "version", "2.0.0-C");
        cJSON_AddStringToObject(root, "status", "operational");
        cJSON_AddStringToObject(root, "engine", "ANSI C11 High-Performance Engine");
        cJSON_AddStringToObject(root, "city", "Dehradun, Uttarakhand, India");
        cJSON_AddStringToObject(root, "websocket", "/ws/live-buses");
        send_json_response(client_sock, 200, root);
    }
    /* 2. GET /api/health */
    else if (strcmp(method, "GET") == 0 && strcmp(path, "/api/health") == 0) {
        time_t now = time(NULL);
        struct tm *tinfo = localtime(&now);
        char t_str[32];
        strftime(t_str, sizeof(t_str), "%Y-%m-%d %H:%M:%S", tinfo);

        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "status", "healthy");
        cJSON_AddStringToObject(root, "server", "pure_c_transitx");
        cJSON_AddStringToObject(root, "timestamp", t_str);
        cJSON_AddNumberToObject(root, "active_buses", 15);
        cJSON_AddNumberToObject(root, "total_routes", 5);
        cJSON_AddNumberToObject(root, "websocket_clients", websocket_get_active_client_count());
        send_json_response(client_sock, 200, root);
    }
    /* 3. GET /api/stops/nearby */
    else if (strcmp(method, "GET") == 0 && strcmp(path, "/api/stops/nearby") == 0) {
        char lat_s[32] = {0}, lng_s[32] = {0}, rad_s[32] = {0};
        get_query_param(query, "lat", lat_s, sizeof(lat_s));
        get_query_param(query, "lng", lng_s, sizeof(lng_s));
        get_query_param(query, "radius", rad_s, sizeof(rad_s));

        double lat = atof(lat_s);
        double lng = atof(lng_s);
        double radius = (strlen(rad_s) > 0) ? atof(rad_s) : 2000.0;

        StopRecord stops[MAX_STOPS];
        int num_stops = data_store_get_all_stops(stops, MAX_STOPS);

        cJSON *arr = cJSON_CreateArray();
        for (int i = 0; i < num_stops; i++) {
            double dist = haversine_distance_meters(lat, lng, stops[i].latitude, stops[i].longitude);
            if (dist <= radius) {
                cJSON *item = cJSON_CreateObject();
                cJSON_AddNumberToObject(item, "id", stops[i].id);
                cJSON_AddStringToObject(item, "stop_name", stops[i].name);
                cJSON_AddNumberToObject(item, "latitude", stops[i].latitude);
                cJSON_AddNumberToObject(item, "longitude", stops[i].longitude);
                cJSON_AddNumberToObject(item, "distance_meters", round(dist * 10.0) / 10.0);
                cJSON_AddItemToArray(arr, item);
            }
        }
        send_json_response(client_sock, 200, arr);
    }
    /* 4. GET /api/stops */
    else if (strcmp(method, "GET") == 0 && strcmp(path, "/api/stops") == 0) {
        StopRecord stops[MAX_STOPS];
        int num_stops = data_store_get_all_stops(stops, MAX_STOPS);
        cJSON *arr = cJSON_CreateArray();
        for (int i = 0; i < num_stops; i++) {
            cJSON *item = cJSON_CreateObject();
            cJSON_AddNumberToObject(item, "id", stops[i].id);
            cJSON_AddStringToObject(item, "stop_name", stops[i].name);
            cJSON_AddNumberToObject(item, "latitude", stops[i].latitude);
            cJSON_AddNumberToObject(item, "longitude", stops[i].longitude);
            cJSON_AddItemToArray(arr, item);
        }
        send_json_response(client_sock, 200, arr);
    }
    /* 5. GET /api/stops/:id */
    else if (strcmp(method, "GET") == 0 && strncmp(path, "/api/stops/", 11) == 0) {
        int stop_id = atoi(path + 11);
        StopRecord *s = data_store_find_stop_by_id(stop_id);
        if (s) {
            cJSON *item = cJSON_CreateObject();
            cJSON_AddNumberToObject(item, "id", s->id);
            cJSON_AddStringToObject(item, "stop_name", s->name);
            cJSON_AddNumberToObject(item, "latitude", s->latitude);
            cJSON_AddNumberToObject(item, "longitude", s->longitude);
            send_json_response(client_sock, 200, item);
        } else {
            cJSON *err = cJSON_CreateObject();
            cJSON_AddStringToObject(err, "error", "Stop not found");
            send_json_response(client_sock, 404, err);
        }
    }
    /* 6. GET /api/buses */
    else if (strcmp(method, "GET") == 0 && strcmp(path, "/api/buses") == 0) {
        BusRecord buses[MAX_BUSES];
        int count = data_store_get_all_buses(buses, MAX_BUSES);
        cJSON *arr = cJSON_CreateArray();
        for (int i = 0; i < count; i++) {
            cJSON *item = cJSON_CreateObject();
            cJSON_AddNumberToObject(item, "id", buses[i].id);
            cJSON_AddStringToObject(item, "bus_number", buses[i].bus_number);
            cJSON_AddStringToObject(item, "registration_number", buses[i].reg_number);
            cJSON_AddNumberToObject(item, "capacity", buses[i].capacity);
            cJSON_AddStringToObject(item, "status", buses[i].status);
            cJSON_AddNumberToObject(item, "route_id", buses[i].route_id);
            cJSON_AddStringToObject(item, "driver_name", buses[i].driver_name);
            cJSON_AddNumberToObject(item, "latitude", buses[i].latitude);
            cJSON_AddNumberToObject(item, "longitude", buses[i].longitude);
            cJSON_AddNumberToObject(item, "speed", round(buses[i].speed * 10.0) / 10.0);
            cJSON_AddNumberToObject(item, "heading", round(buses[i].heading));
            cJSON_AddNumberToObject(item, "accuracy", buses[i].accuracy);
            cJSON_AddStringToObject(item, "updated_at", buses[i].last_updated);
            cJSON_AddNumberToObject(item, "next_stop_id", buses[i].next_stop_id);
            cJSON_AddItemToArray(arr, item);
        }
        send_json_response(client_sock, 200, arr);
    }
    /* 7. GET /api/buses/:id */
    else if (strcmp(method, "GET") == 0 && strncmp(path, "/api/buses/", 11) == 0 && strstr(path, "/location") == NULL && strstr(path, "/eta") == NULL) {
        int bus_id = atoi(path + 11);
        BusRecord *b = data_store_find_bus_by_id(bus_id);
        if (b) {
            cJSON *item = cJSON_CreateObject();
            cJSON_AddNumberToObject(item, "id", b->id);
            cJSON_AddStringToObject(item, "bus_number", b->bus_number);
            cJSON_AddStringToObject(item, "registration_number", b->reg_number);
            cJSON_AddNumberToObject(item, "capacity", b->capacity);
            cJSON_AddStringToObject(item, "status", b->status);
            cJSON_AddNumberToObject(item, "route_id", b->route_id);
            cJSON_AddStringToObject(item, "driver_name", b->driver_name);
            cJSON_AddNumberToObject(item, "latitude", b->latitude);
            cJSON_AddNumberToObject(item, "longitude", b->longitude);
            cJSON_AddNumberToObject(item, "speed", round(b->speed * 10.0) / 10.0);
            cJSON_AddNumberToObject(item, "heading", round(b->heading));
            cJSON_AddStringToObject(item, "updated_at", b->last_updated);
            cJSON_AddNumberToObject(item, "next_stop_id", b->next_stop_id);
            send_json_response(client_sock, 200, item);
        } else {
            cJSON *err = cJSON_CreateObject();
            cJSON_AddStringToObject(err, "error", "Bus not found");
            send_json_response(client_sock, 404, err);
        }
    }
    /* 8. GET /api/routes */
    else if (strcmp(method, "GET") == 0 && strcmp(path, "/api/routes") == 0) {
        RouteRecord routes[MAX_ROUTES];
        int count = data_store_get_all_routes(routes, MAX_ROUTES);
        cJSON *arr = cJSON_CreateArray();
        for (int i = 0; i < count; i++) {
            cJSON *item = cJSON_CreateObject();
            cJSON_AddNumberToObject(item, "id", routes[i].id);
            cJSON_AddStringToObject(item, "route_name", routes[i].name);
            cJSON_AddStringToObject(item, "route_code", routes[i].code);
            cJSON_AddStringToObject(item, "start_point", routes[i].start_point);
            cJSON_AddStringToObject(item, "end_point", routes[i].end_point);
            cJSON_AddNumberToObject(item, "num_stops", routes[i].num_stops);

            cJSON *stops_arr = cJSON_CreateArray();
            for (int s = 0; s < routes[i].num_stops; s++) {
                StopRecord *sr = data_store_find_stop_by_id(routes[i].stops[s].stop_id);
                if (sr) {
                    cJSON *si = cJSON_CreateObject();
                    cJSON_AddNumberToObject(si, "stop_id", sr->id);
                    cJSON_AddStringToObject(si, "stop_name", sr->name);
                    cJSON_AddNumberToObject(si, "stop_order", routes[i].stops[s].stop_order);
                    cJSON_AddNumberToObject(si, "latitude", sr->latitude);
                    cJSON_AddNumberToObject(si, "longitude", sr->longitude);
                    cJSON_AddItemToArray(stops_arr, si);
                }
            }
            cJSON_AddItemToObject(item, "stops", stops_arr);
            cJSON_AddItemToArray(arr, item);
        }
        send_json_response(client_sock, 200, arr);
    }
    /* 9. GET /api/routes/:id/schedule */
    else if (strcmp(method, "GET") == 0 && strncmp(path, "/api/routes/", 12) == 0 && strstr(path, "/schedule") != NULL) {
        int route_id = atoi(path + 12);
        BusEtaResult result;
        if (eta_calculate_route_schedule(route_id, 0, &result)) {
            cJSON *root = cJSON_CreateObject();
            cJSON_AddNumberToObject(root, "route_id", route_id);
            cJSON_AddNumberToObject(root, "reference_bus_id", result.bus_id);
            cJSON_AddStringToObject(root, "reference_bus_number", result.bus_number);

            cJSON *stops_arr = cJSON_CreateArray();
            for (int i = 0; i < result.num_stop_etas; i++) {
                cJSON *si = cJSON_CreateObject();
                cJSON_AddNumberToObject(si, "stop_id", result.stop_etas[i].stop_id);
                cJSON_AddStringToObject(si, "stop_name", result.stop_etas[i].stop_name);
                cJSON_AddNumberToObject(si, "distance_km", result.stop_etas[i].distance_km);
                cJSON_AddNumberToObject(si, "minutes_from_now", result.stop_etas[i].duration_minutes);
                cJSON_AddStringToObject(si, "arrival_time", result.stop_etas[i].arrival_time_str);
                cJSON_AddItemToArray(stops_arr, si);
            }
            cJSON_AddItemToObject(root, "schedule", stops_arr);
            send_json_response(client_sock, 200, root);
        } else {
            cJSON *err = cJSON_CreateObject();
            cJSON_AddStringToObject(err, "error", "Route schedule unavailable");
            send_json_response(client_sock, 404, err);
        }
    }
    /* 10. POST /api/plan-journey */
    else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/plan-journey") == 0) {
        cJSON *body = cJSON_Parse(body_ptr);
        if (!body) {
            cJSON *err = cJSON_CreateObject();
            cJSON_AddStringToObject(err, "error", "Invalid JSON body");
            send_json_response(client_sock, 400, err);
        } else {
            cJSON *start_item = cJSON_GetObjectItem(body, "start_stop_id");
            cJSON *end_item = cJSON_GetObjectItem(body, "end_stop_id");
            cJSON *lat_item = cJSON_GetObjectItem(body, "passenger_lat");
            cJSON *lng_item = cJSON_GetObjectItem(body, "passenger_lng");

            JourneyPlanRequest j_req;
            memset(&j_req, 0, sizeof(j_req));
            j_req.start_stop_id = start_item ? start_item->valueint : 1;
            j_req.end_stop_id = end_item ? end_item->valueint : 6;
            if (lat_item && lng_item) {
                j_req.passenger_lat = lat_item->valuedouble;
                j_req.passenger_lng = lng_item->valuedouble;
                j_req.has_passenger_loc = 1;
            }

            JourneyPlanResult res;
            if (plan_journey(&j_req, &res)) {
                cJSON *root = cJSON_CreateObject();
                cJSON_AddNumberToObject(root, "start_stop_id", res.start_stop_id);
                cJSON_AddStringToObject(root, "start_stop_name", res.start_stop_name);
                cJSON_AddNumberToObject(root, "end_stop_id", res.end_stop_id);
                cJSON_AddStringToObject(root, "end_stop_name", res.end_stop_name);
                cJSON_AddNumberToObject(root, "num_candidates", res.num_candidates);

                cJSON *cands_arr = cJSON_CreateArray();
                for (int i = 0; i < res.num_candidates; i++) {
                    JourneyCandidate *c = &res.candidates[i];
                    cJSON *ci = cJSON_CreateObject();
                    cJSON_AddNumberToObject(ci, "bus_id", c->bus_id);
                    cJSON_AddStringToObject(ci, "bus_number", c->bus_number);
                    cJSON_AddNumberToObject(ci, "route_id", c->route_id);
                    cJSON_AddStringToObject(ci, "route_name", c->route_name);
                    cJSON_AddStringToObject(ci, "route_code", c->route_code);
                    cJSON_AddStringToObject(ci, "boarding_arrival_time", c->boarding_arrival_time);
                    cJSON_AddNumberToObject(ci, "wait_time_minutes", c->wait_time_minutes);
                    cJSON_AddStringToObject(ci, "destination_arrival_time", c->destination_arrival_time);
                    cJSON_AddNumberToObject(ci, "in_transit_minutes", c->in_transit_minutes);
                    cJSON_AddNumberToObject(ci, "total_journey_minutes", c->total_journey_minutes);
                    cJSON_AddNumberToObject(ci, "walk_distance_meters", c->walk_distance_meters);
                    cJSON_AddNumberToObject(ci, "walk_time_minutes", c->walk_time_minutes);
                    cJSON_AddBoolToObject(ci, "is_best_pick", c->is_best_pick);
                    cJSON_AddStringToObject(ci, "recommendation_reason", c->recommendation_reason);

                    cJSON *stops_arr = cJSON_CreateArray();
                    for (int s = 0; s < c->num_intermediate_stops; s++) {
                        cJSON *si = cJSON_CreateObject();
                        cJSON_AddNumberToObject(si, "stop_id", c->intermediate_stops[s].stop_id);
                        cJSON_AddStringToObject(si, "stop_name", c->intermediate_stops[s].stop_name);
                        cJSON_AddStringToObject(si, "arrival_time", c->intermediate_stops[s].arrival_time_str);
                        cJSON_AddNumberToObject(si, "minutes_from_now", c->intermediate_stops[s].duration_minutes);
                        cJSON_AddItemToArray(stops_arr, si);
                    }
                    cJSON_AddItemToObject(ci, "timetable", stops_arr);
                    cJSON_AddItemToArray(cands_arr, ci);
                }
                cJSON_AddItemToObject(root, "candidates", cands_arr);
                send_json_response(client_sock, 200, root);
            } else {
                cJSON *err = cJSON_CreateObject();
                cJSON_AddStringToObject(err, "error", "No direct route candidates between selected stops");
                send_json_response(client_sock, 404, err);
            }
            cJSON_Delete(body);
        }
    }
    /* 11. POST /api/gps/update */
    else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/gps/update") == 0) {
        cJSON *body = cJSON_Parse(body_ptr);
        if (!body) {
            cJSON *err = cJSON_CreateObject();
            cJSON_AddStringToObject(err, "error", "Invalid JSON");
            send_json_response(client_sock, 400, err);
        } else {
            int bus_id = cJSON_GetObjectItem(body, "bus_id") ? cJSON_GetObjectItem(body, "bus_id")->valueint : 1;
            double lat = cJSON_GetObjectItem(body, "latitude") ? cJSON_GetObjectItem(body, "latitude")->valuedouble : 30.3165;
            double lng = cJSON_GetObjectItem(body, "longitude") ? cJSON_GetObjectItem(body, "longitude")->valuedouble : 78.0322;
            double speed = cJSON_GetObjectItem(body, "speed") ? cJSON_GetObjectItem(body, "speed")->valuedouble : 30.0;
            double heading = cJSON_GetObjectItem(body, "heading") ? cJSON_GetObjectItem(body, "heading")->valuedouble : 0.0;
            double acc = cJSON_GetObjectItem(body, "accuracy") ? cJSON_GetObjectItem(body, "accuracy")->valuedouble : 5.0;

            if (data_store_ingest_gps(bus_id, lat, lng, speed, heading, acc)) {
                /* Broadcast to WebSocket clients */
                char ws_msg[256];
                snprintf(ws_msg, sizeof(ws_msg),
                    "{\"type\":\"location_update\",\"bus_id\":%d,\"latitude\":%.6f,\"longitude\":%.6f,\"speed\":%.1f,\"heading\":%.1f}",
                    bus_id, lat, lng, speed, heading);
                websocket_broadcast_text(ws_msg);

                cJSON *resp = cJSON_CreateObject();
                cJSON_AddStringToObject(resp, "status", "success");
                cJSON_AddStringToObject(resp, "message", "GPS telemetry accepted and broadcasted");
                cJSON_AddNumberToObject(resp, "bus_id", bus_id);
                send_json_response(client_sock, 200, resp);
            } else {
                cJSON *err = cJSON_CreateObject();
                cJSON_AddStringToObject(err, "error", "Failed to ingest GPS update");
                send_json_response(client_sock, 400, err);
            }
            cJSON_Delete(body);
        }
    }
    /* 12. GET /api/favorites */
    else if (strcmp(method, "GET") == 0 && strcmp(path, "/api/favorites") == 0) {
        int route_ids[16];
        int count = data_store_get_favorites(1, route_ids, 16);
        cJSON *arr = cJSON_CreateArray();
        for (int i = 0; i < count; i++) {
            RouteRecord *r = data_store_find_route_by_id(route_ids[i]);
            if (r) {
                cJSON *item = cJSON_CreateObject();
                cJSON_AddNumberToObject(item, "route_id", r->id);
                cJSON_AddStringToObject(item, "route_name", r->name);
                cJSON_AddStringToObject(item, "route_code", r->code);
                cJSON_AddItemToArray(arr, item);
            }
        }
        send_json_response(client_sock, 200, arr);
    }
    /* 13. POST /api/favorites */
    else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/favorites") == 0) {
        cJSON *body = cJSON_Parse(body_ptr);
        int route_id = (body && cJSON_GetObjectItem(body, "route_id")) ? cJSON_GetObjectItem(body, "route_id")->valueint : 1;
        data_store_add_favorite(1, route_id);
        if (body) cJSON_Delete(body);

        cJSON *resp = cJSON_CreateObject();
        cJSON_AddStringToObject(resp, "message", "Route saved to favorites");
        cJSON_AddNumberToObject(resp, "route_id", route_id);
        send_json_response(client_sock, 201, resp);
    }
    /* 14. DELETE /api/favorites/:id */
    else if (strcmp(method, "DELETE") == 0 && strncmp(path, "/api/favorites/", 15) == 0) {
        int route_id = atoi(path + 15);
        data_store_remove_favorite(1, route_id);
        cJSON *resp = cJSON_CreateObject();
        cJSON_AddStringToObject(resp, "message", "Favorite removed");
        cJSON_AddNumberToObject(resp, "route_id", route_id);
        send_json_response(client_sock, 200, resp);
    }
    /* Default: 404 */
    else {
        cJSON *err = cJSON_CreateObject();
        cJSON_AddStringToObject(err, "error", "Endpoint not found");
        cJSON_AddStringToObject(err, "path", path);
        send_json_response(client_sock, 404, err);
    }

    CLOSE_SOCK(client_sock);
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

#ifdef _WIN32
static unsigned int __stdcall ws_broadcaster_thread(void *arg)
#else
static void* ws_broadcaster_thread(void *arg)
#endif
{
    (void)arg;
    while (g_server_running) {
#ifdef _WIN32
        Sleep(2000);
#else
        sleep(2);
#endif
        if (websocket_get_active_client_count() > 0) {
            BusRecord buses[MAX_BUSES];
            int n = data_store_get_all_buses(buses, MAX_BUSES);
            cJSON *root = cJSON_CreateObject();
            cJSON_AddStringToObject(root, "type", "all_buses_telemetry");
            cJSON *arr = cJSON_CreateArray();
            for (int i = 0; i < n; i++) {
                cJSON *b = cJSON_CreateObject();
                cJSON_AddNumberToObject(b, "bus_id", buses[i].id);
                cJSON_AddStringToObject(b, "bus_number", buses[i].bus_number);
                cJSON_AddNumberToObject(b, "route_id", buses[i].route_id);
                cJSON_AddNumberToObject(b, "latitude", buses[i].latitude);
                cJSON_AddNumberToObject(b, "longitude", buses[i].longitude);
                cJSON_AddNumberToObject(b, "speed", buses[i].speed);
                cJSON_AddNumberToObject(b, "heading", buses[i].heading);
                cJSON_AddNumberToObject(b, "next_stop_id", buses[i].next_stop_id);
                cJSON_AddItemToArray(arr, b);
            }
            cJSON_AddItemToObject(root, "buses", arr);
            char *rendered = cJSON_PrintUnformatted(root);
            if (rendered) {
                websocket_broadcast_text(rendered);
                free(rendered);
            }
            cJSON_Delete(root);
        }
    }
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

void http_server_run(int port) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    websocket_init();
    data_store_init();

    g_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (g_listen_socket == (http_socket_t)-1) {
        fprintf(stderr, "[Pure C HTTP] Failed to create socket\n");
        return;
    }

    int opt = 1;
    setsockopt(g_listen_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons((unsigned short)port);

    if (bind(g_listen_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "[Pure C HTTP] Bind to port %d failed\n", port);
        CLOSE_SOCK(g_listen_socket);
        return;
    }

    if (listen(g_listen_socket, SOMAXCONN) < 0) {
        fprintf(stderr, "[Pure C HTTP] Listen failed\n");
        CLOSE_SOCK(g_listen_socket);
        return;
    }

#ifdef _WIN32
    uintptr_t bcast_handle = _beginthreadex(NULL, 0, ws_broadcaster_thread, NULL, 0, NULL);
    if (bcast_handle) CloseHandle((HANDLE)bcast_handle);
#else
    pthread_t bcast_th;
    pthread_create(&bcast_th, NULL, ws_broadcaster_thread, NULL);
    pthread_detach(bcast_th);
#endif

    printf("========================================================\n");
    printf("   TransitX Pure C Backend Server listening on port %d  \n", port);
    printf("   Endpoints:\n");
    printf("     GET  /api/health\n");
    printf("     GET  /api/stops\n");
    printf("     GET  /api/stops/nearby?lat=&lng=&radius=\n");
    printf("     GET  /api/buses\n");
    printf("     GET  /api/buses/:id\n");
    printf("     GET  /api/routes\n");
    printf("     GET  /api/routes/:id/schedule\n");
    printf("     POST /api/plan-journey\n");
    printf("     POST /api/gps/update\n");
    printf("     GET/POST/DEL /api/favorites\n");
    printf("     WS   /ws/live-buses\n");
    printf("========================================================\n");

    while (g_server_running) {
        struct sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        http_socket_t client_sock = accept(g_listen_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock == (http_socket_t)-1) {
            if (!g_server_running) break;
            continue;
        }

#ifdef _WIN32
        uintptr_t handle = _beginthreadex(NULL, 0, http_worker_thread, (void*)(intptr_t)client_sock, 0, NULL);
        if (handle) CloseHandle((HANDLE)handle);
#else
        pthread_t th;
        pthread_create(&th, NULL, http_worker_thread, (void*)(intptr_t)client_sock);
        pthread_detach(th);
#endif
    }

    CLOSE_SOCK(g_listen_socket);
#ifdef _WIN32
    WSACleanup();
#endif
}

void http_server_stop(void) {
    g_server_running = 0;
    if (g_listen_socket != (http_socket_t)-1) {
        CLOSE_SOCK(g_listen_socket);
    }
}
