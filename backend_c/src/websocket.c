/* websocket.c - Pure C WebSocket RFC 6455 Handler */
#include "websocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
    static CRITICAL_SECTION g_ws_cs;
    #define WS_LOCK() EnterCriticalSection(&g_ws_cs)
    #define WS_UNLOCK() LeaveCriticalSection(&g_ws_cs)
    #define WS_CLOSE_SOCK(s) closesocket(s)
#else
    #include <unistd.h>
    #include <pthread.h>
    static pthread_mutex_t g_ws_cs = PTHREAD_MUTEX_INITIALIZER;
    #define WS_LOCK() pthread_mutex_lock(&g_ws_cs)
    #define WS_UNLOCK() pthread_mutex_unlock(&g_ws_cs)
    #define WS_CLOSE_SOCK(s) close(s)
#endif

#define MAX_WS_CLIENTS 64

static ws_socket_t g_clients[MAX_WS_CLIENTS];
static int g_num_clients = 0;
static int g_ws_initialized = 0;

/* SHA1 Implementation */
static void sha1_transform(uint32_t state[5], const uint8_t buffer[64]) {
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
    uint32_t w[80];
    for (int i = 0; i < 16; i++) {
        w[i] = ((uint32_t)buffer[i * 4] << 24) |
               ((uint32_t)buffer[i * 4 + 1] << 16) |
               ((uint32_t)buffer[i * 4 + 2] << 8) |
               (uint32_t)buffer[i * 4 + 3];
    }
    for (int i = 16; i < 80; i++) {
        uint32_t v = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
        w[i] = (v << 1) | (v >> 31);
    }
    for (int i = 0; i < 80; i++) {
        uint32_t f, k;
        if (i < 20) { f = (b & c) | ((~b) & d); k = 0x5A827999; }
        else if (i < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1; }
        else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
        else { f = b ^ c ^ d; k = 0xCA62C1D6; }
        uint32_t temp = ((a << 5) | (a >> 27)) + f + e + k + w[i];
        e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    }
    state[0] += a; state[1] += b; state[2] += c; state[3] += d; state[4] += e;
}

static void compute_sha1(const uint8_t *data, size_t len, uint8_t digest[20]) {
    uint32_t state[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    uint64_t total_bits = len * 8;
    size_t offset = 0;

    while (len >= 64) {
        sha1_transform(state, data + offset);
        offset += 64;
        len -= 64;
    }

    uint8_t block[64] = {0};
    memcpy(block, data + offset, len);
    block[len] = 0x80;

    if (len >= 56) {
        sha1_transform(state, block);
        memset(block, 0, 64);
    }

    for (int i = 7; i >= 0; i--) {
        block[56 + (7 - i)] = (uint8_t)((total_bits >> (i * 8)) & 0xFF);
    }
    sha1_transform(state, block);

    for (int i = 0; i < 5; i++) {
        digest[i * 4]     = (uint8_t)((state[i] >> 24) & 0xFF);
        digest[i * 4 + 1] = (uint8_t)((state[i] >> 16) & 0xFF);
        digest[i * 4 + 2] = (uint8_t)((state[i] >> 8) & 0xFF);
        digest[i * 4 + 3] = (uint8_t)(state[i] & 0xFF);
    }
}

static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void base64_encode_20(const uint8_t in[20], char out[32]) {
    int i, j = 0;
    for (i = 0; i < 18; i += 3) {
        uint32_t val = (in[i] << 16) | (in[i + 1] << 8) | in[i + 2];
        out[j++] = b64_table[(val >> 18) & 0x3F];
        out[j++] = b64_table[(val >> 12) & 0x3F];
        out[j++] = b64_table[(val >> 6) & 0x3F];
        out[j++] = b64_table[val & 0x3F];
    }
    uint32_t val = (in[18] << 16) | (in[19] << 8);
    out[j++] = b64_table[(val >> 18) & 0x3F];
    out[j++] = b64_table[(val >> 12) & 0x3F];
    out[j++] = b64_table[(val >> 6) & 0x3F];
    out[j++] = '=';
    out[j] = '\0';
}

void websocket_init(void) {
    if (g_ws_initialized) return;
#ifdef _WIN32
    InitializeCriticalSection(&g_ws_cs);
#endif
    g_ws_initialized = 1;
}

static void remove_client_locked(ws_socket_t sock) {
    for (int i = 0; i < g_num_clients; i++) {
        if (g_clients[i] == sock) {
            for (int j = i; j < g_num_clients - 1; j++) {
                g_clients[j] = g_clients[j + 1];
            }
            g_num_clients--;
            break;
        }
    }
}

#ifdef _WIN32
static unsigned int __stdcall ws_client_reader(void *arg)
#else
static void* ws_client_reader(void *arg)
#endif
{
    ws_socket_t sock = (ws_socket_t)(intptr_t)arg;
    uint8_t hdr[2];

    while (1) {
        int r = recv(sock, (char*)hdr, 2, 0);
        if (r <= 0) break;

        uint8_t opcode = hdr[0] & 0x0F;
        int masked = (hdr[1] & 0x80) != 0;
        uint64_t payload_len = hdr[1] & 0x7F;

        if (payload_len == 126) {
            uint8_t ext[2];
            if (recv(sock, (char*)ext, 2, 0) <= 0) break;
            payload_len = ((uint64_t)ext[0] << 8) | ext[1];
        } else if (payload_len == 127) {
            uint8_t ext[8];
            if (recv(sock, (char*)ext, 8, 0) <= 0) break;
            payload_len = 0;
            for (int i = 0; i < 8; i++) payload_len = (payload_len << 8) | ext[i];
        }

        uint8_t mask_key[4] = {0};
        if (masked) {
            if (recv(sock, (char*)mask_key, 4, 0) <= 0) break;
        }

        /* Discard / read payload bytes */
        char buf[512];
        uint64_t to_read = payload_len;
        while (to_read > 0) {
            int chunk = (to_read > sizeof(buf)) ? sizeof(buf) : (int)to_read;
            int n = recv(sock, buf, chunk, 0);
            if (n <= 0) goto done;
            to_read -= n;
        }

        if (opcode == 0x8) break; /* Close */
        if (opcode == 0x9) {
            /* Reply with Pong */
            uint8_t pong[2] = {0x8A, 0x00};
            send(sock, (char*)pong, 2, 0);
        }
    }

done:
    WS_LOCK();
    remove_client_locked(sock);
    WS_UNLOCK();
    WS_CLOSE_SOCK(sock);
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

int websocket_handle_upgrade(ws_socket_t sock, const char *raw_request) {
    const char *key_hdr = strstr(raw_request, "Sec-WebSocket-Key:");
    if (!key_hdr) key_hdr = strstr(raw_request, "sec-websocket-key:");
    if (!key_hdr) return 0;

    key_hdr += strlen("Sec-WebSocket-Key:");
    while (*key_hdr == ' ' || *key_hdr == '\t') key_hdr++;

    char client_key[64] = {0};
    int k_len = 0;
    while (*key_hdr && *key_hdr != '\r' && *key_hdr != '\n' && k_len < 60) {
        client_key[k_len++] = *key_hdr++;
    }
    client_key[k_len] = '\0';

    const char *magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    char combined[128];
    snprintf(combined, sizeof(combined), "%s%s", client_key, magic);

    uint8_t digest[20];
    compute_sha1((const uint8_t*)combined, strlen(combined), digest);

    char accept_key[32];
    base64_encode_20(digest, accept_key);

    char response[256];
    int resp_len = snprintf(response, sizeof(response),
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n\r\n",
        accept_key
    );

    send(sock, response, resp_len, 0);

    /* Register client */
    WS_LOCK();
    if (g_num_clients < MAX_WS_CLIENTS) {
        g_clients[g_num_clients++] = sock;
    }
    WS_UNLOCK();

    /* Launch client reader thread */
#ifdef _WIN32
    uintptr_t handle = _beginthreadex(NULL, 0, ws_client_reader, (void*)(intptr_t)sock, 0, NULL);
    if (handle) CloseHandle((HANDLE)handle);
#else
    pthread_t th;
    pthread_create(&th, NULL, ws_client_reader, (void*)(intptr_t)sock);
    pthread_detach(th);
#endif

    return 1;
}

void websocket_broadcast_text(const char *json_text) {
    if (!json_text) return;
    size_t len = strlen(json_text);

    uint8_t header[10];
    int header_len = 0;

    header[0] = 0x81; /* FIN + Text frame */
    if (len <= 125) {
        header[1] = (uint8_t)len;
        header_len = 2;
    } else if (len <= 65535) {
        header[1] = 126;
        header[2] = (uint8_t)((len >> 8) & 0xFF);
        header[3] = (uint8_t)(len & 0xFF);
        header_len = 4;
    } else {
        header[1] = 127;
        for (int i = 7; i >= 0; i--) {
            header[2 + (7 - i)] = (uint8_t)((len >> (i * 8)) & 0xFF);
        }
        header_len = 10;
    }

    WS_LOCK();
    for (int i = 0; i < g_num_clients; i++) {
        ws_socket_t s = g_clients[i];
        send(s, (const char*)header, header_len, 0);
        send(s, json_text, (int)len, 0);
    }
    WS_UNLOCK();
}

int websocket_get_active_client_count(void) {
    WS_LOCK();
    int count = g_num_clients;
    WS_UNLOCK();
    return count;
}
