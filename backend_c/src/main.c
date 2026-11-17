/* main.c - TransitX Pure C Backend Entry Point */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "http_server.h"

static void handle_sigint(int sig) {
    (void)sig;
    printf("\n[TransitX C] Shutting down gracefully...\n");
    http_server_stop();
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);

    int port = 8080;
    const char *env_port = getenv("BACKEND_PORT");
    if (env_port) {
        port = atoi(env_port);
        if (port <= 0) port = 8080;
    }
    if (argc > 1) {
        int arg_port = atoi(argv[1]);
        if (arg_port > 0) port = arg_port;
    }

    printf("Starting TransitX Pure C Backend Server...\n");
    http_server_run(port);

    return 0;
}
