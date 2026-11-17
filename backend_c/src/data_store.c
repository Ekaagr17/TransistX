/* data_store.c - In-memory thread-safe transit dataset & dynamic simulation in pure C */
#include "data_store.h"
#include "haversine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
    #include <windows.h>
    static CRITICAL_SECTION g_store_cs;
    #define LOCK_STORE() EnterCriticalSection(&g_store_cs)
    #define UNLOCK_STORE() LeaveCriticalSection(&g_store_cs)
    #define INIT_STORE_LOCK() InitializeCriticalSection(&g_store_cs)
#else
    #include <pthread.h>
    static pthread_mutex_t g_store_mutex = PTHREAD_MUTEX_INITIALIZER;
    #define LOCK_STORE() pthread_mutex_lock(&g_store_mutex)
    #define UNLOCK_STORE() pthread_mutex_unlock(&g_store_mutex)
    #define INIT_STORE_LOCK() ((void)0)
#endif

/* Datasets */
static StopRecord g_stops[MAX_STOPS];
static int g_num_stops = 0;

static RouteRecord g_routes[MAX_ROUTES];
static int g_num_routes = 0;

static BusRecord g_buses[MAX_BUSES];
static int g_num_buses = 0;

static FavoriteRecord g_favorites[MAX_FAVORITES];
static int g_num_favorites = 0;

static int g_initialized = 0;

static void add_stop(int id, const char *name, double lat, double lon) {
    if (g_num_stops >= MAX_STOPS) return;
    g_stops[g_num_stops].id = id;
    strncpy(g_stops[g_num_stops].name, name, sizeof(g_stops[g_num_stops].name) - 1);
    g_stops[g_num_stops].latitude = lat;
    g_stops[g_num_stops].longitude = lon;
    g_num_stops++;
}

static void add_bus(int id, const char *num, const char *reg, int cap, const char *status, int route_id, const char *driver) {
    if (g_num_buses >= MAX_BUSES) return;
    g_buses[g_num_buses].id = id;
    strncpy(g_buses[g_num_buses].bus_number, num, sizeof(g_buses[g_num_buses].bus_number) - 1);
    strncpy(g_buses[g_num_buses].reg_number, reg, sizeof(g_buses[g_num_buses].reg_number) - 1);
    g_buses[g_num_buses].capacity = cap;
    strncpy(g_buses[g_num_buses].status, status, sizeof(g_buses[g_num_buses].status) - 1);
    g_buses[g_num_buses].route_id = route_id;
    strncpy(g_buses[g_num_buses].driver_name, driver, sizeof(g_buses[g_num_buses].driver_name) - 1);
    g_buses[g_num_buses].latitude = 30.3165;
    g_buses[g_num_buses].longitude = 78.0322;
    g_buses[g_num_buses].speed = 30.0;
    g_buses[g_num_buses].heading = 120.0;
    g_buses[g_num_buses].accuracy = 8.0;
    strcpy(g_buses[g_num_buses].last_updated, "Just now");
    g_buses[g_num_buses].current_stop_index = 0;
    g_buses[g_num_buses].next_stop_id = 1;
    g_buses[g_num_buses].is_driver_gps = 0;
    g_num_buses++;
}

void data_store_init(void) {
    if (g_initialized) return;
    INIT_STORE_LOCK();
    LOCK_STORE();

    /* 1. All 29 Real Dehradun Stops */
    add_stop(1,  "Clock Tower",              30.3165, 78.0322);
    add_stop(2,  "Paltan Bazaar",            30.3185, 78.0370);
    add_stop(3,  "Gandhi Road",              30.3148, 78.0400);
    add_stop(4,  "Nehru Colony",             30.3080, 78.0410);
    add_stop(5,  "Dalanwala",                30.3030, 78.0435);
    add_stop(6,  "ISBT Dehradun",            30.2724, 78.0462);

    add_stop(7,  "Rajpur Road",              30.3380, 78.0540);
    add_stop(8,  "Jakhan",                   30.3480, 78.0580);
    add_stop(9,  "Malsi",                    30.3600, 78.0620);
    add_stop(10, "Kolhu Khet",               30.3750, 78.0680);
    add_stop(11, "Barlowganj",               30.4200, 78.0730);
    add_stop(12, "Mussoorie Library Chowk",  30.4560, 78.0750);

    add_stop(13, "Clement Town",             30.2880, 78.0180);
    add_stop(14, "FRI Gate",                 30.3410, 78.0000);
    add_stop(15, "Race Course",              30.3250, 78.0260);
    add_stop(16, "Astley Hall",              30.3200, 78.0310);
    add_stop(17, "Karanpur",                 30.3220, 78.0350);
    add_stop(18, "Ballupur Chowk",           30.3310, 78.0480);

    add_stop(19, "Rispana Bridge",           30.3140, 78.0280);
    add_stop(20, "Laxmi Road",               30.3100, 78.0220);
    add_stop(21, "Chander Nagar",            30.3060, 78.0150);
    add_stop(22, "Niranjanpur",              30.2980, 78.0100);
    add_stop(23, "Premnagar",                30.2900, 78.0050);

    add_stop(24, "Survey Chowk",             30.3195, 78.0290);
    add_stop(25, "EC Road",                  30.3230, 78.0360);
    add_stop(26, "Saharanpur Chowk",         30.3270, 78.0430);
    add_stop(27, "Patel Nagar",              30.3330, 78.0500);
    add_stop(28, "Shimla Bypass",            30.3400, 78.0590);
    add_stop(29, "Pacific Mall",             30.3570, 78.0670);

    /* 2. All 5 Dehradun Routes with Ordered Stops */
    /* Route 1: Clock Tower -> ISBT */
    g_routes[0].id = 1;
    strcpy(g_routes[0].name, "Demo: Clock Tower - ISBT");
    strcpy(g_routes[0].code, "DR001");
    strcpy(g_routes[0].start_point, "Clock Tower");
    strcpy(g_routes[0].end_point, "ISBT Dehradun");
    g_routes[0].num_stops = 6;
    for (int i = 0; i < 6; i++) {
        g_routes[0].stops[i].stop_id = i + 1;
        g_routes[0].stops[i].stop_order = i + 1;
        g_routes[0].stops[i].dwell_seconds = (i == 0 || i == 5) ? 60 : 30;
    }

    /* Route 2: Rajpur Road -> Mussoorie */
    g_routes[1].id = 2;
    strcpy(g_routes[1].name, "Demo: Rajpur Road - Mussoorie");
    strcpy(g_routes[1].code, "DR002");
    strcpy(g_routes[1].start_point, "Rajpur Road");
    strcpy(g_routes[1].end_point, "Mussoorie Library Chowk");
    g_routes[1].num_stops = 6;
    for (int i = 0; i < 6; i++) {
        g_routes[1].stops[i].stop_id = 7 + i;
        g_routes[1].stops[i].stop_order = i + 1;
        g_routes[1].stops[i].dwell_seconds = (i == 0 || i == 5) ? 60 : 30;
    }

    /* Route 3: Clement Town -> Ballupur */
    g_routes[2].id = 3;
    strcpy(g_routes[2].name, "Demo: Clement Town - Ballupur");
    strcpy(g_routes[2].code, "DR003");
    strcpy(g_routes[2].start_point, "Clement Town");
    strcpy(g_routes[2].end_point, "Ballupur Chowk");
    g_routes[2].num_stops = 6;
    for (int i = 0; i < 6; i++) {
        g_routes[2].stops[i].stop_id = 13 + i;
        g_routes[2].stops[i].stop_order = i + 1;
        g_routes[2].stops[i].dwell_seconds = (i == 0 || i == 5) ? 60 : 30;
    }

    /* Route 4: Rispana -> Premnagar */
    g_routes[3].id = 4;
    strcpy(g_routes[3].name, "Demo: Rispana - Premnagar");
    strcpy(g_routes[3].code, "DR004");
    strcpy(g_routes[3].start_point, "Rispana Bridge");
    strcpy(g_routes[3].end_point, "Premnagar");
    g_routes[3].num_stops = 5;
    for (int i = 0; i < 5; i++) {
        g_routes[3].stops[i].stop_id = 19 + i;
        g_routes[3].stops[i].stop_order = i + 1;
        g_routes[3].stops[i].dwell_seconds = (i == 0 || i == 4) ? 60 : 30;
    }

    /* Route 5: Survey Chowk -> Pacific Mall */
    g_routes[4].id = 5;
    strcpy(g_routes[4].name, "Demo: Survey Chowk - Pacific Mall");
    strcpy(g_routes[4].code, "DR005");
    strcpy(g_routes[4].start_point, "Survey Chowk");
    strcpy(g_routes[4].end_point, "Pacific Mall");
    g_routes[4].num_stops = 6;
    for (int i = 0; i < 6; i++) {
        g_routes[4].stops[i].stop_id = 24 + i;
        g_routes[4].stops[i].stop_order = i + 1;
        g_routes[4].stops[i].dwell_seconds = (i == 0 || i == 5) ? 60 : 30;
    }
    g_num_routes = 5;

    /* 3. 16 Active / Standby Buses */
    add_bus(1,  "UK07PA7001", "UK-07-PA-7001", 42, "active", 1, "Suresh Panwar");
    add_bus(2,  "UK07PA7002", "UK-07-PA-7002", 44, "active", 1, "Manoj Bisht");
    add_bus(3,  "UK07PA7003", "UK-07-PA-7003", 40, "active", 1, "Deepak Thapa");
    add_bus(4,  "UK07PA7004", "UK-07-PA-7004", 48, "active", 2, "Rajesh Gusain");
    add_bus(5,  "UK07PA7005", "UK-07-PA-7005", 50, "active", 2, "Anil Rawat");
    add_bus(6,  "UK07PA7006", "UK-07-PA-7006", 52, "active", 2, "Vipin Joshi");
    add_bus(7,  "UK07PA7007", "UK-07-PA-7007", 45, "active", 3, "Prakash Bhatt");
    add_bus(8,  "UK07PA7008", "UK-07-PA-7008", 46, "active", 3, "Kamal Negi");
    add_bus(9,  "UK07PA7009", "UK-07-PA-7009", 54, "active", 3, "Harish Semwal");
    add_bus(10, "UK07PA7010", "UK-07-PA-7010", 42, "active", 4, "Ramesh Chauhan");
    add_bus(11, "UK07PA7011", "UK-07-PA-7011", 44, "active", 4, "Sunil Uniyal");
    add_bus(12, "UK07PA7012", "UK-07-PA-7012", 50, "active", 4, "Vinod Nautiyal");
    add_bus(13, "UK07PA7013", "UK-07-PA-7013", 48, "active", 5, "Dinesh Kandari");
    add_bus(14, "UK07PA7014", "UK-07-PA-7014", 52, "active", 5, "Sanjay Rana");
    add_bus(15, "UK07PA7015", "UK-07-PA-7015", 46, "active", 5, "Ajay Bartwal");
    add_bus(16, "UK07PA7016", "UK-07-PA-7016", 50, "inactive", 1, "Standby Driver");

    /* Initial dynamic position calculation */
    data_store_tick_dynamic_positions();

    g_initialized = 1;
    UNLOCK_STORE();
}

int data_store_get_all_stops(StopRecord *out_stops, int max_count) {
    LOCK_STORE();
    int count = g_num_stops < max_count ? g_num_stops : max_count;
    memcpy(out_stops, g_stops, count * sizeof(StopRecord));
    UNLOCK_STORE();
    return count;
}

StopRecord* data_store_find_stop_by_id(int stop_id) {
    for (int i = 0; i < g_num_stops; i++) {
        if (g_stops[i].id == stop_id) return &g_stops[i];
    }
    return NULL;
}

int data_store_get_all_routes(RouteRecord *out_routes, int max_count) {
    LOCK_STORE();
    int count = g_num_routes < max_count ? g_num_routes : max_count;
    memcpy(out_routes, g_routes, count * sizeof(RouteRecord));
    UNLOCK_STORE();
    return count;
}

RouteRecord* data_store_find_route_by_id(int route_id) {
    for (int i = 0; i < g_num_routes; i++) {
        if (g_routes[i].id == route_id) return &g_routes[i];
    }
    return NULL;
}

int data_store_get_all_buses(BusRecord *out_buses, int max_count) {
    data_store_tick_dynamic_positions();
    LOCK_STORE();
    int count = g_num_buses < max_count ? g_num_buses : max_count;
    memcpy(out_buses, g_buses, count * sizeof(BusRecord));
    UNLOCK_STORE();
    return count;
}

BusRecord* data_store_find_bus_by_id(int bus_id) {
    data_store_tick_dynamic_positions();
    for (int i = 0; i < g_num_buses; i++) {
        if (g_buses[i].id == bus_id) return &g_buses[i];
    }
    return NULL;
}

/* Tick dynamic positions according to wall-clock time */
void data_store_tick_dynamic_positions(void) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);

    /* 30-minute round trip cycle (1800 seconds) */
    const double cycle_seconds = 1800.0;

    for (int b = 0; b < g_num_buses; b++) {
        BusRecord *bus = &g_buses[b];
        if (strcmp(bus->status, "active") != 0) continue;
        if (bus->is_driver_gps) continue; /* Keep driver's real telemetry */

        RouteRecord *route = data_store_find_route_by_id(bus->route_id);
        if (!route || route->num_stops < 2) continue;

        /* Offset each bus on the same route by e.g. 8 minutes (480s) */
        double bus_stagger = (bus->id * 480.0);
        double t = fmod((double)now + bus_stagger, cycle_seconds);
        double half_cycle = cycle_seconds / 2.0;

        int forward = (t < half_cycle);
        double progress = forward ? (t / half_cycle) : ((cycle_seconds - t) / half_cycle);

        /* Map progress (0.0 to 1.0) across segments */
        int num_segments = route->num_stops - 1;
        double seg_progress = progress * (double)num_segments;
        int seg_idx = (int)seg_progress;
        if (seg_idx >= num_segments) seg_idx = num_segments - 1;
        double local_t = seg_progress - (double)seg_idx;

        int stop_a_id = route->stops[seg_idx].stop_id;
        int stop_b_id = route->stops[seg_idx + 1].stop_id;
        if (!forward) {
            int tmp = stop_a_id; stop_a_id = stop_b_id; stop_b_id = tmp;
            local_t = 1.0 - local_t;
        }

        StopRecord *sa = data_store_find_stop_by_id(stop_a_id);
        StopRecord *sb = data_store_find_stop_by_id(stop_b_id);

        if (sa && sb) {
            bus->latitude = sa->latitude + local_t * (sb->latitude - sa->latitude);
            bus->longitude = sa->longitude + local_t * (sb->longitude - sa->longitude);
            bus->heading = haversine_bearing(sa->latitude, sa->longitude, sb->latitude, sb->longitude);
            bus->speed = 28.0 + 8.0 * sin((double)now * 0.05 + bus->id);
            if (bus->speed < 10.0) bus->speed = 10.0;
            bus->current_stop_index = seg_idx;
            bus->next_stop_id = forward ? stop_b_id : stop_a_id;
            bus->accuracy = 6.0;
            strcpy(bus->last_updated, time_str);
        }
    }
}

int data_store_ingest_gps(int bus_id, double lat, double lng, double speed, double heading, double accuracy) {
    LOCK_STORE();
    for (int i = 0; i < g_num_buses; i++) {
        if (g_buses[i].id == bus_id) {
            g_buses[i].latitude = lat;
            g_buses[i].longitude = lng;
            g_buses[i].speed = speed;
            g_buses[i].heading = heading;
            g_buses[i].accuracy = accuracy;
            g_buses[i].is_driver_gps = 1;

            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            strftime(g_buses[i].last_updated, sizeof(g_buses[i].last_updated), "%H:%M:%S", tm_info);

            UNLOCK_STORE();
            return 1;
        }
    }
    UNLOCK_STORE();
    return 0;
}

int data_store_get_favorites(int user_id, int *out_route_ids, int max_count) {
    LOCK_STORE();
    int count = 0;
    for (int i = 0; i < g_num_favorites && count < max_count; i++) {
        if (g_favorites[i].user_id == user_id) {
            out_route_ids[count++] = g_favorites[i].route_id;
        }
    }
    UNLOCK_STORE();
    return count;
}

int data_store_add_favorite(int user_id, int route_id) {
    LOCK_STORE();
    for (int i = 0; i < g_num_favorites; i++) {
        if (g_favorites[i].user_id == user_id && g_favorites[i].route_id == route_id) {
            UNLOCK_STORE();
            return 1;
        }
    }
    if (g_num_favorites < MAX_FAVORITES) {
        g_favorites[g_num_favorites].id = g_num_favorites + 1;
        g_favorites[g_num_favorites].user_id = user_id;
        g_favorites[g_num_favorites].route_id = route_id;
        g_num_favorites++;
        UNLOCK_STORE();
        return 1;
    }
    UNLOCK_STORE();
    return 0;
}

int data_store_remove_favorite(int user_id, int route_id) {
    LOCK_STORE();
    for (int i = 0; i < g_num_favorites; i++) {
        if (g_favorites[i].user_id == user_id && g_favorites[i].route_id == route_id) {
            for (int j = i; j < g_num_favorites - 1; j++) {
                g_favorites[j] = g_favorites[j + 1];
            }
            g_num_favorites--;
            UNLOCK_STORE();
            return 1;
        }
    }
    UNLOCK_STORE();
    return 0;
}
