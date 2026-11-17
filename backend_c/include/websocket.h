/* websocket.h - Pure C WebSocket RFC 6455 Handler for Live Bus Streaming */
#ifndef TRANSITX_WEBSOCKET_C_H
#define TRANSITX_WEBSOCKET_C_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
    #include <winsock2.h>
    typedef SOCKET ws_socket_t;
#else
    typedef int ws_socket_t;
#endif

/* Initialize WebSocket subsystem */
void websocket_init(void);

/* Check if HTTP request is a WebSocket upgrade request and handle it */
int websocket_handle_upgrade(ws_socket_t sock, const char *raw_request);

/* Broadcast a text JSON message to all currently connected WebSocket clients */
void websocket_broadcast_text(const char *json_text);

/* Get number of active WebSocket connections */
int websocket_get_active_client_count(void);

#ifdef __cplusplus
}
#endif

#endif /* TRANSITX_WEBSOCKET_C_H */
