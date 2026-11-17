/* http_server.h - Pure C multi-threaded HTTP server */
#ifndef TRANSITX_HTTP_SERVER_H
#define TRANSITX_HTTP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Start HTTP server on given port (blocking loop) */
void http_server_run(int port);

/* Stop HTTP server */
void http_server_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* TRANSITX_HTTP_SERVER_H */
