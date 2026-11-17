/* data_store.h - In-memory thread-safe transit dataset & dynamic simulation in pure C */
#ifndef TRANSITX_DATA_STORE_H
#define TRANSITX_DATA_STORE_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_STOPS 32
#define MAX_ROUTES 8
#define MAX_ROUTE_STOPS 16
#define MAX_BUSES 20
#define MAX_FAVORITES 64

typedef struct {
    int id;
    char name[64];
    double latitude;
    double longitude;
} StopRecord;

typedef struct {
    int stop_id;
    int stop_order;
    int dwell_seconds;
} RouteStopItem;

typedef struct {
    int id;
    char name[64];
    char code[16];
    char start_point[64];
    char end_point[64];
    int num_stops;
    RouteStopItem stops[MAX_ROUTE_STOPS];
} RouteRecord;

typedef struct {
    int id;
    char bus_number[24];
    char reg_number[24];
    int capacity;
    char status[16]; // "active", "inactive"
    int route_id;
    char driver_name[48];

    /* Dynamic / Live Telemetry */
    double latitude;
    double longitude;
    double speed;     // km/h
    double heading;   // 0 - 360
    double accuracy;  // meters
    char last_updated[32];
    int current_stop_index;
    int next_stop_id;
    int is_driver_gps; // 1 if updated by real driver GPS
} BusRecord;

typedef struct {
    int id;
    int user_id;
    int route_id;
} FavoriteRecord;

/* Initialize datasets with 29 Dehradun stops, 5 routes, 16 buses */
void data_store_init(void);

/* Stops access */
int data_store_get_all_stops(StopRecord *out_stops, int max_count);
StopRecord* data_store_find_stop_by_id(int stop_id);

/* Routes access */
int data_store_get_all_routes(RouteRecord *out_routes, int max_count);
RouteRecord* data_store_find_route_by_id(int route_id);

/* Buses access & dynamic positions */
int data_store_get_all_buses(BusRecord *out_buses, int max_count);
BusRecord* data_store_find_bus_by_id(int bus_id);

/* Updates simulated positions for all buses according to wall-clock time */
void data_store_tick_dynamic_positions(void);

/* Driver live GPS update ingestion */
int data_store_ingest_gps(int bus_id, double lat, double lng, double speed, double heading, double accuracy);

/* Favorites */
int data_store_get_favorites(int user_id, int *out_route_ids, int max_count);
int data_store_add_favorite(int user_id, int route_id);
int data_store_remove_favorite(int user_id, int route_id);

#ifdef __cplusplus
}
#endif

#endif /* TRANSITX_DATA_STORE_H */
