// crow_all.h - Self-contained Crow-compatible C++ HTTP & WebSocket framework
// Compatible with C++14/C++17, MinGW 6.3+, MSVC, GCC, Clang, Docker Linux
#pragma once
#ifndef CROW_ALL_H
#define CROW_ALL_H

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0600
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <process.h>
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
    #define INVALID_SOCKET_VAL INVALID_SOCKET
    #define SOCKET_ERROR_VAL SOCKET_ERROR
    #define CLOSE_SOCKET(s) closesocket(s)
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <pthread.h>
    using socket_t = int;
    #define INVALID_SOCKET_VAL (-1)
    #define SOCKET_ERROR_VAL (-1)
    #define CLOSE_SOCKET(s) close(s)
#endif

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <memory>
#include <atomic>
#include <regex>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include "json.hpp"

namespace crow {

// Platform-independent Mutex
class Mutex {
private:
#ifdef _WIN32
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t mtx;
#endif
public:
    Mutex() {
#ifdef _WIN32
        InitializeCriticalSection(&cs);
#else
        pthread_mutex_init(&mtx, nullptr);
#endif
    }
    ~Mutex() {
#ifdef _WIN32
        DeleteCriticalSection(&cs);
#else
        pthread_mutex_destroy(&mtx);
#endif
    }
    void lock() {
#ifdef _WIN32
        EnterCriticalSection(&cs);
#else
        pthread_mutex_lock(&mtx);
#endif
    }
    void unlock() {
#ifdef _WIN32
        LeaveCriticalSection(&cs);
#else
        pthread_mutex_unlock(&mtx);
#endif
    }
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;
};

class LockGuard {
private:
    Mutex& mtx;
public:
    explicit LockGuard(Mutex& m) : mtx(m) { mtx.lock(); }
    ~LockGuard() { mtx.unlock(); }
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
};

// Platform-independent Thread launcher
#ifdef _WIN32
inline DWORD WINAPI ThreadWorkerProc(LPVOID p) {
    auto* fn = static_cast<std::function<void()>*>(p);
    (*fn)();
    delete fn;
    return 0;
}
inline void launch_thread(std::function<void()> f) {
    auto* holder = new std::function<void()>(f);
    HANDLE handle = CreateThread(nullptr, 0, ThreadWorkerProc, holder, 0, nullptr);
    if (handle) CloseHandle(handle);
}
#else
inline void launch_thread(std::function<void()> f) {
    auto* holder = new std::function<void()>(f);
    pthread_t th;
    pthread_create(&th, nullptr, [](void* p) -> void* {
        auto* fn = static_cast<std::function<void()>*>(p);
        (*fn)();
        delete fn;
        return nullptr;
    }, holder);
    pthread_detach(th);
}
#endif

// HTTP Methods
enum class HTTPMethod {
    Get,
    Post,
    Put,
    Delete,
    Patch,
    Options,
    Head
};

inline HTTPMethod string_to_method(const std::string& m) {
    if (m == "GET") return HTTPMethod::Get;
    if (m == "POST") return HTTPMethod::Post;
    if (m == "PUT") return HTTPMethod::Put;
    if (m == "DELETE") return HTTPMethod::Delete;
    if (m == "PATCH") return HTTPMethod::Patch;
    if (m == "OPTIONS") return HTTPMethod::Options;
    if (m == "HEAD") return HTTPMethod::Head;
    return HTTPMethod::Get;
}

inline std::string method_to_string(HTTPMethod m) {
    switch (m) {
        case HTTPMethod::Get: return "GET";
        case HTTPMethod::Post: return "POST";
        case HTTPMethod::Put: return "PUT";
        case HTTPMethod::Delete: return "DELETE";
        case HTTPMethod::Patch: return "PATCH";
        case HTTPMethod::Options: return "OPTIONS";
        case HTTPMethod::Head: return "HEAD";
    }
    return "GET";
}

inline HTTPMethod operator"" _method(const char* str, size_t) {
    return string_to_method(str);
}

// Query string helper
class query_string {
public:
    std::unordered_map<std::string, std::string> params;

    void parse(const std::string& qs) {
        params.clear();
        std::stringstream ss(qs);
        std::string item;
        while (std::getline(ss, item, '&')) {
            size_t eq = item.find('=');
            if (eq != std::string::npos) {
                params[url_decode(item.substr(0, eq))] = url_decode(item.substr(eq + 1));
            } else if (!item.empty()) {
                params[url_decode(item)] = "";
            }
        }
    }

    const char* get(const std::string& name) const {
        auto it = params.find(name);
        if (it != params.end()) return it->second.c_str();
        return nullptr;
    }

    std::string get_string(const std::string& name, const std::string& default_val = "") const {
        auto it = params.find(name);
        if (it != params.end()) return it->second;
        return default_val;
    }

    static std::string url_decode(const std::string& in) {
        std::string out;
        out.reserve(in.size());
        for (size_t i = 0; i < in.size(); ++i) {
            if (in[i] == '%') {
                if (i + 2 < in.size()) {
                    int val = 0;
                    std::istringstream is(in.substr(i + 1, 2));
                    if (is >> std::hex >> val) {
                        out += static_cast<char>(val);
                        i += 2;
                    } else {
                        out += in[i];
                    }
                }
            } else if (in[i] == '+') {
                out += ' ';
            } else {
                out += in[i];
            }
        }
        return out;
    }
};

// HTTP Request
struct request {
    HTTPMethod method;
    std::string method_name;
    std::string raw_url;
    std::string url;
    std::string query;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
    query_string url_params;
    std::string remote_ip_address;
    std::map<std::string, std::string> path_params;

    std::string get_header_value(const std::string& key) const {
        std::string lower_key = key;
        std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
        for (const auto& kv : headers) {
            std::string k = kv.first;
            std::transform(k.begin(), k.end(), k.begin(), ::tolower);
            if (k == lower_key) return kv.second;
        }
        return "";
    }
};

// HTTP Response
struct response {
    int code;
    std::string body;
    std::unordered_map<std::string, std::string> headers;

    response() : code(200) {
        set_header("Content-Type", "application/json; charset=utf-8");
    }

    response(int c) : code(c) {
        set_header("Content-Type", "application/json; charset=utf-8");
    }

    response(const std::string& b) : code(200), body(b) {
        set_header("Content-Type", "application/json; charset=utf-8");
    }

    response(const char* b) : code(200), body(b ? b : "") {
        set_header("Content-Type", "application/json; charset=utf-8");
    }

    response(int c, const std::string& b) : code(c), body(b) {
        set_header("Content-Type", "application/json; charset=utf-8");
    }

    response(int c, const char* b) : code(c), body(b ? b : "") {
        set_header("Content-Type", "application/json; charset=utf-8");
    }

    response(int c, const nlohmann::json& j) : code(c), body(j.dump()) {
        set_header("Content-Type", "application/json; charset=utf-8");
    }

    response(const nlohmann::json& j) : code(200), body(j.dump()) {
        set_header("Content-Type", "application/json; charset=utf-8");
    }

    void set_header(const std::string& k, const std::string& v) {
        headers[k] = v;
    }

    void add_header(const std::string& k, const std::string& v) {
        headers[k] = v;
    }

    void write(const std::string& str) {
        body += str;
    }

    void end() {}
    void end(const std::string& str) {
        body += str;
    }

    std::string status_message() const {
        switch (code) {
            case 200: return "OK";
            case 201: return "Created";
            case 202: return "Accepted";
            case 204: return "No Content";
            case 400: return "Bad Request";
            case 401: return "Unauthorized";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 405: return "Method Not Allowed";
            case 409: return "Conflict";
            case 422: return "Unprocessable Entity";
            case 429: return "Too Many Requests";
            case 500: return "Internal Server Error";
            default: return "OK";
        }
    }
};

// WebSocket connection
namespace websocket {

class connection {
public:
    socket_t sock;
    std::string id;
    std::unordered_map<std::string, std::string> userdata;
    Mutex send_mutex;
    std::atomic<bool> is_closed;

    connection(socket_t s, const std::string& conn_id) : sock(s), id(conn_id), is_closed(false) {}

    void send_text(const std::string& data) {
        if (is_closed) return;
        LockGuard lock(send_mutex);
        std::vector<uint8_t> frame;
        frame.push_back(0x81); // FIN + text opcode

        size_t len = data.size();
        if (len <= 125) {
            frame.push_back(static_cast<uint8_t>(len));
        } else if (len <= 65535) {
            frame.push_back(126);
            frame.push_back(static_cast<uint8_t>((len >> 8) & 0xff));
            frame.push_back(static_cast<uint8_t>(len & 0xff));
        } else {
            frame.push_back(127);
            for (int i = 7; i >= 0; i--) {
                frame.push_back(static_cast<uint8_t>((len >> (i * 8)) & 0xff));
            }
        }
        frame.insert(frame.end(), data.begin(), data.end());
        ::send(sock, reinterpret_cast<const char*>(frame.data()), static_cast<int>(frame.size()), 0);
    }

    void send_binary(const std::string& data) {
        if (is_closed) return;
        LockGuard lock(send_mutex);
        std::vector<uint8_t> frame;
        frame.push_back(0x82); // FIN + binary opcode
        size_t len = data.size();
        if (len <= 125) {
            frame.push_back(static_cast<uint8_t>(len));
        } else if (len <= 65535) {
            frame.push_back(126);
            frame.push_back(static_cast<uint8_t>((len >> 8) & 0xff));
            frame.push_back(static_cast<uint8_t>(len & 0xff));
        } else {
            frame.push_back(127);
            for (int i = 7; i >= 0; i--) {
                frame.push_back(static_cast<uint8_t>((len >> (i * 8)) & 0xff));
            }
        }
        frame.insert(frame.end(), data.begin(), data.end());
        ::send(sock, reinterpret_cast<const char*>(frame.data()), static_cast<int>(frame.size()), 0);
    }

    void close(const std::string& /*reason*/ = "") {
        bool expected = false;
        if (!is_closed.compare_exchange_strong(expected, true)) return;
        std::vector<uint8_t> frame = {0x88, 0x00};
        ::send(sock, reinterpret_cast<const char*>(frame.data()), static_cast<int>(frame.size()), 0);
        CLOSE_SOCKET(sock);
    }
};

} // namespace websocket

// SHA1 & Base64 for WebSocket Handshake
namespace detail {

inline std::string sha1_base64(const std::string& input) {
    uint32_t h0 = 0x67452301;
    uint32_t h1 = 0xEFCDAB89;
    uint32_t h2 = 0x98BADCFE;
    uint32_t h3 = 0x10325476;
    uint32_t h4 = 0xC3D2E1F0;

    std::vector<uint8_t> msg(input.begin(), input.end());
    uint64_t bit_len = msg.size() * 8;
    msg.push_back(0x80);
    while ((msg.size() % 64) != 56) msg.push_back(0x00);
    for (int i = 7; i >= 0; i--) msg.push_back(static_cast<uint8_t>((bit_len >> (i * 8)) & 0xff));

    for (size_t chunk = 0; chunk < msg.size(); chunk += 64) {
        uint32_t w[80];
        for (int i = 0; i < 16; i++) {
            w[i] = (static_cast<uint32_t>(msg[chunk + i * 4]) << 24) |
                   (static_cast<uint32_t>(msg[chunk + i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(msg[chunk + i * 4 + 2]) << 8) |
                   static_cast<uint32_t>(msg[chunk + i * 4 + 3]);
        }
        for (int i = 16; i < 80; i++) {
            uint32_t v = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
            w[i] = (v << 1) | (v >> 31);
        }

        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;
        for (int i = 0; i < 80; i++) {
            uint32_t f, k;
            if (i < 20) { f = (b & c) | ((~b) & d); k = 0x5A827999; }
            else if (i < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1; }
            else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
            else { f = b ^ c ^ d; k = 0xCA62C1D6; }
            uint32_t temp = ((a << 5) | (a >> 27)) + f + e + k + w[i];
            e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
        }
        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
    }

    uint8_t hash[20];
    uint32_t parts[5] = {h0, h1, h2, h3, h4};
    for (int i = 0; i < 5; i++) {
        hash[i * 4]     = (parts[i] >> 24) & 0xff;
        hash[i * 4 + 1] = (parts[i] >> 16) & 0xff;
        hash[i * 4 + 2] = (parts[i] >> 8) & 0xff;
        hash[i * 4 + 3] = parts[i] & 0xff;
    }

    static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string res;
    for (int i = 0; i < 20; i += 3) {
        uint32_t val = (hash[i] << 16) | ((i + 1 < 20 ? hash[i + 1] : 0) << 8) | (i + 2 < 20 ? hash[i + 2] : 0);
        res += b64_table[(val >> 18) & 0x3f];
        res += b64_table[(val >> 12) & 0x3f];
        res += (i + 1 < 20) ? b64_table[(val >> 6) & 0x3f] : '=';
        res += (i + 2 < 20) ? b64_table[val & 0x3f] : '=';
    }
    return res;
}

} // namespace detail

// Route handler abstractions
class DynamicRule {
public:
    std::string pattern;
    std::vector<HTTPMethod> allowed_methods;
    std::regex regex_pattern;
    std::vector<std::string> param_names;
    std::function<response(const request&)> handler;

    DynamicRule(const std::string& p) : pattern(p) {
        build_regex(p);
    }

    DynamicRule& methods(HTTPMethod m) {
        allowed_methods.push_back(m);
        return *this;
    }

    DynamicRule& methods(std::initializer_list<HTTPMethod> m_list) {
        for (auto m : m_list) allowed_methods.push_back(m);
        return *this;
    }

    void build_regex(const std::string& p) {
        std::string reg = "^";
        size_t i = 0;
        int param_idx = 0;
        while (i < p.size()) {
            if (p[i] == '<') {
                size_t close = p.find('>', i);
                if (close != std::string::npos) {
                    std::string type = p.substr(i + 1, close - i - 1);
                    param_names.push_back("arg" + std::to_string(param_idx++));
                    if (type == "int") reg += "([0-9]+)";
                    else if (type == "double" || type == "float") reg += "([0-9]+(?:\\.[0-9]+)?)";
                    else reg += "([^/]+)";
                    i = close + 1;
                    continue;
                }
            } else if (p[i] == ':') {
                size_t slash = p.find('/', i);
                std::string name = (slash != std::string::npos) ? p.substr(i + 1, slash - i - 1) : p.substr(i + 1);
                param_names.push_back(name);
                reg += "([^/]+)";
                i = (slash != std::string::npos) ? slash : p.size();
                continue;
            }
            if (p[i] == '.' || p[i] == '+' || p[i] == '?' || p[i] == '*' || p[i] == '^' || p[i] == '$') {
                reg += '\\';
            }
            reg += p[i++];
        }
        reg += "(?:/)?$";
        regex_pattern = std::regex(reg);
    }

    // Direct request handler
    void operator()(std::function<response(const request&)> f) {
        handler = f;
    }

    // Single int parameter handler (e.g. /api/buses/<int>)
    void operator()(std::function<response(const request&, int)> f) {
        handler = [f](const request& req) {
            int arg0 = 0;
            if (!req.path_params.empty()) {
                try { arg0 = std::stoi(req.path_params.begin()->second); } catch (...) {}
            }
            return f(req, arg0);
        };
    }

    // Single string parameter handler
    void operator()(std::function<response(const request&, std::string)> f) {
        handler = [f](const request& req) {
            std::string arg0 = req.path_params.empty() ? "" : req.path_params.begin()->second;
            return f(req, arg0);
        };
    }

    // Two int parameters handler (e.g. /api/buses/<int>/eta/<int>)
    void operator()(std::function<response(const request&, int, int)> f) {
        handler = [f](const request& req) {
            int a0 = 0, a1 = 0;
            auto it = req.path_params.begin();
            if (it != req.path_params.end()) {
                try { a0 = std::stoi(it->second); } catch (...) {}
                ++it;
            }
            if (it != req.path_params.end()) {
                try { a1 = std::stoi(it->second); } catch (...) {}
            }
            return f(req, a0, a1);
        };
    }

    // Int + String handler
    void operator()(std::function<response(const request&, int, std::string)> f) {
        handler = [f](const request& req) {
            int a0 = 0; std::string a1;
            auto it = req.path_params.begin();
            if (it != req.path_params.end()) {
                try { a0 = std::stoi(it->second); } catch (...) {}
                ++it;
            }
            if (it != req.path_params.end()) {
                a1 = it->second;
            }
            return f(req, a0, a1);
        };
    }
};

// WebSocket route rule
class WebSocketRule {
public:
    std::string path;
    std::function<void(websocket::connection&)> open_handler;
    std::function<void(websocket::connection&, const std::string&, bool)> message_handler;
    std::function<void(websocket::connection&, const std::string&)> close_handler;
    std::function<void(websocket::connection&)> error_handler;

    WebSocketRule(const std::string& p) : path(p) {}

    WebSocketRule& onopen(std::function<void(websocket::connection&)> h) {
        open_handler = h;
        return *this;
    }

    WebSocketRule& onmessage(std::function<void(websocket::connection&, const std::string&, bool)> h) {
        message_handler = h;
        return *this;
    }

    WebSocketRule& onclose(std::function<void(websocket::connection&, const std::string&)> h) {
        close_handler = h;
        return *this;
    }

    WebSocketRule& onerror(std::function<void(websocket::connection&)> h) {
        error_handler = h;
        return *this;
    }
};

// Crow Application Class
class SimpleApp {
private:
    uint16_t port_num = 8080;
    std::string bind_address = "0.0.0.0";
    bool is_multithreaded = true;
    std::atomic<bool> is_running;
    socket_t server_socket = INVALID_SOCKET_VAL;

    std::vector<std::shared_ptr<DynamicRule>> routes;
    std::unordered_map<std::string, std::shared_ptr<WebSocketRule>> ws_routes;
    Mutex routes_mutex;

public:
    SimpleApp() : is_running(false) {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }

    ~SimpleApp() {
        stop();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    SimpleApp& port(uint16_t p) {
        port_num = p;
        return *this;
    }

    SimpleApp& bindaddr(const std::string& addr) {
        bind_address = addr;
        return *this;
    }

    SimpleApp& multithreaded() {
        is_multithreaded = true;
        return *this;
    }

    SimpleApp& concurrency(int /*n*/) {
        return *this;
    }

    DynamicRule& route_dynamic(const std::string& pattern) {
        LockGuard lock(routes_mutex);
        auto rule = std::make_shared<DynamicRule>(pattern);
        routes.push_back(rule);
        return *rule;
    }

    WebSocketRule& route_websocket(const std::string& path) {
        LockGuard lock(routes_mutex);
        auto rule = std::make_shared<WebSocketRule>(path);
        ws_routes[path] = rule;
        return *rule;
    }

    void stop() {
        bool expected = true;
        if (!is_running.compare_exchange_strong(expected, false)) return;
        if (server_socket != INVALID_SOCKET_VAL) {
            CLOSE_SOCKET(server_socket);
            server_socket = INVALID_SOCKET_VAL;
        }
    }

    void run() {
        server_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_socket == INVALID_SOCKET_VAL) {
            std::cerr << "[Crow] Failed to create socket." << std::endl;
            return;
        }

        int opt = 1;
#ifdef _WIN32
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_num);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        if (::bind(server_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == SOCKET_ERROR_VAL) {
            std::cerr << "[Crow] Failed to bind to port " << port_num << std::endl;
            CLOSE_SOCKET(server_socket);
            return;
        }

        if (::listen(server_socket, SOMAXCONN) == SOCKET_ERROR_VAL) {
            std::cerr << "[Crow] Failed to listen." << std::endl;
            CLOSE_SOCKET(server_socket);
            return;
        }

        is_running = true;
        std::cout << "[TransitX Crow Server] Running on http://" << bind_address << ":" << port_num << std::endl;

        while (is_running) {
            sockaddr_in client_addr{};
#ifdef _WIN32
            int client_len = sizeof(client_addr);
#else
            socklen_t client_len = sizeof(client_addr);
#endif
            socket_t client_socket = ::accept(server_socket, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
            if (client_socket == INVALID_SOCKET_VAL) {
                if (!is_running) break;
                continue;
            }

            char* ip_str = inet_ntoa(client_addr.sin_addr);
            std::string client_ip = ip_str ? ip_str : "127.0.0.1";

            if (is_multithreaded) {
                launch_thread([this, client_socket, client_ip]() {
                    this->handle_connection(client_socket, client_ip);
                });
            } else {
                handle_connection(client_socket, client_ip);
            }
        }
    }

private:
    void handle_connection(socket_t sock, const std::string& client_ip) {
        std::string raw_request;
        char buffer[4096];
        int bytes_read = 0;

        while ((bytes_read = ::recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[bytes_read] = '\0';
            raw_request.append(buffer, bytes_read);
            if (raw_request.find("\r\n\r\n") != std::string::npos) break;
        }

        if (raw_request.empty()) {
            CLOSE_SOCKET(sock);
            return;
        }

        size_t header_end = raw_request.find("\r\n\r\n");
        std::string header_str = raw_request.substr(0, header_end);
        std::string initial_body = (header_end + 4 < raw_request.size()) ? raw_request.substr(header_end + 4) : "";

        std::istringstream stream(header_str);
        std::string line;
        if (!std::getline(stream, line)) {
            CLOSE_SOCKET(sock);
            return;
        }
        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::istringstream line_stream(line);
        std::string method_str, uri, http_ver;
        line_stream >> method_str >> uri >> http_ver;

        request req;
        req.method_name = method_str;
        req.method = string_to_method(method_str);
        req.raw_url = uri;
        req.remote_ip_address = client_ip;

        size_t q_pos = uri.find('?');
        if (q_pos != std::string::npos) {
            req.url = uri.substr(0, q_pos);
            req.query = uri.substr(q_pos + 1);
            req.url_params.parse(req.query);
        } else {
            req.url = uri;
        }

        size_t content_length = 0;
        while (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string k = line.substr(0, colon);
                std::string v = line.substr(colon + 1);
                while (!v.empty() && (v.front() == ' ' || v.front() == '\t')) v.erase(0, 1);
                req.headers[k] = v;
                std::string lower_k = k;
                std::transform(lower_k.begin(), lower_k.end(), lower_k.begin(), ::tolower);
                if (lower_k == "content-length") {
                    try { content_length = std::stoul(v); } catch (...) {}
                }
            }
        }

        req.body = initial_body;
        while (req.body.size() < content_length) {
            bytes_read = ::recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (bytes_read <= 0) break;
            req.body.append(buffer, bytes_read);
        }

        std::string upgrade = req.get_header_value("Upgrade");
        std::string sec_key = req.get_header_value("Sec-WebSocket-Key");
        if (upgrade == "websocket" && !sec_key.empty()) {
            handle_websocket_upgrade(sock, req, sec_key);
            return;
        }

        if (req.method == HTTPMethod::Options) {
            response res(204);
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS, PATCH");
            res.set_header("Access-Control-Allow-Headers", "Authorization, Content-Type, Accept, X-Requested-With");
            res.set_header("Access-Control-Max-Age", "86400");
            send_response(sock, res);
            CLOSE_SOCKET(sock);
            return;
        }

        response res(404, std::string("{\"error\":\"Not Found\",\"status\":404}"));
        bool matched = false;

        {
            LockGuard lock(routes_mutex);
            for (const auto& rule : routes) {
                std::smatch match;
                if (std::regex_match(req.url, match, rule->regex_pattern)) {
                    if (!rule->allowed_methods.empty()) {
                        bool method_ok = false;
                        for (auto m : rule->allowed_methods) {
                            if (m == req.method) { method_ok = true; break; }
                        }
                        if (!method_ok) continue;
                    }

                    for (size_t i = 1; i < match.size(); ++i) {
                        std::string val = match[i].str();
                        if (i - 1 < rule->param_names.size()) {
                            req.path_params[rule->param_names[i - 1]] = val;
                        }
                    }

                    try {
                        res = rule->handler(req);
                    } catch (const std::exception& e) {
                        res = response(500, nlohmann::json({{"error", e.what()}, {"status", 500}}));
                    } catch (...) {
                        res = response(500, nlohmann::json({{"error", "Unknown internal server error"}, {"status", 500}}));
                    }
                    matched = true;
                    break;
                }
            }
        }

        if (!matched && req.method != HTTPMethod::Options) {
            res = response(404, nlohmann::json({{"error", "Endpoint not found: " + req.url}, {"status", 404}}));
        }

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS, PATCH");
        res.set_header("Access-Control-Allow-Headers", "Authorization, Content-Type, Accept");

        send_response(sock, res);
        CLOSE_SOCKET(sock);
    }

    void send_response(socket_t sock, const response& res) {
        std::ostringstream ss;
        ss << "HTTP/1.1 " << res.code << " " << res.status_message() << "\r\n";
        for (const auto& kv : res.headers) {
            ss << kv.first << ": " << kv.second << "\r\n";
        }
        ss << "Content-Length: " << res.body.size() << "\r\n";
        ss << "Connection: close\r\n\r\n";
        ss << res.body;

        std::string out = ss.str();
        ::send(sock, out.data(), static_cast<int>(out.size()), 0);
    }

    void handle_websocket_upgrade(socket_t sock, const request& req, const std::string& sec_key) {
        std::shared_ptr<WebSocketRule> ws_rule = nullptr;
        {
            LockGuard lock(routes_mutex);
            auto it = ws_routes.find(req.url);
            if (it != ws_routes.end()) ws_rule = it->second;
        }

        if (!ws_rule) {
            response res(404, std::string("WebSocket endpoint not found"));
            send_response(sock, res);
            CLOSE_SOCKET(sock);
            return;
        }

        std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        std::string accept_key = detail::sha1_base64(sec_key + magic);

        std::ostringstream ss;
        ss << "HTTP/1.1 101 Switching Protocols\r\n"
           << "Upgrade: websocket\r\n"
           << "Connection: Upgrade\r\n"
           << "Sec-WebSocket-Accept: " << accept_key << "\r\n\r\n";

        std::string response_str = ss.str();
        ::send(sock, response_str.data(), static_cast<int>(response_str.size()), 0);

        auto conn = std::make_shared<websocket::connection>(sock, std::to_string(sock));
        if (ws_rule->open_handler) {
            ws_rule->open_handler(*conn);
        }

        uint8_t hdr[2];
        while (!conn->is_closed) {
            int r = ::recv(sock, reinterpret_cast<char*>(hdr), 2, 0);
            if (r <= 0) break;

            uint8_t opcode = hdr[0] & 0x0F;
            bool masked = (hdr[1] & 0x80) != 0;
            uint64_t payload_len = hdr[1] & 0x7F;

            if (payload_len == 126) {
                uint8_t ext[2];
                if (::recv(sock, reinterpret_cast<char*>(ext), 2, 0) <= 0) break;
                payload_len = (static_cast<uint64_t>(ext[0]) << 8) | ext[1];
            } else if (payload_len == 127) {
                uint8_t ext[8];
                if (::recv(sock, reinterpret_cast<char*>(ext), 8, 0) <= 0) break;
                payload_len = 0;
                for (int i = 0; i < 8; i++) payload_len = (payload_len << 8) | ext[i];
            }

            uint8_t mask_key[4] = {0};
            if (masked) {
                if (::recv(sock, reinterpret_cast<char*>(mask_key), 4, 0) <= 0) break;
            }

            std::string payload;
            payload.resize(static_cast<size_t>(payload_len));
            size_t total_read = 0;
            while (total_read < payload_len) {
                int n = ::recv(sock, &payload[total_read], static_cast<int>(payload_len - total_read), 0);
                if (n <= 0) break;
                total_read += n;
            }

            if (masked) {
                for (size_t i = 0; i < payload_len; i++) {
                    payload[i] ^= mask_key[i % 4];
                }
            }

            if (opcode == 0x8) {
                break;
            } else if (opcode == 0x9) {
                std::vector<uint8_t> pong = {0x8A, 0x00};
                ::send(sock, reinterpret_cast<const char*>(pong.data()), static_cast<int>(pong.size()), 0);
            } else if (opcode == 0x1) {
                if (ws_rule->message_handler) {
                    ws_rule->message_handler(*conn, payload, false);
                }
            } else if (opcode == 0x2) {
                if (ws_rule->message_handler) {
                    ws_rule->message_handler(*conn, payload, true);
                }
            }
        }

        if (ws_rule->close_handler) {
            ws_rule->close_handler(*conn, "Connection closed");
        }
        conn->close();
    }
};

using App = SimpleApp;

#define CROW_ROUTE(app, path) app.route_dynamic(path)
#define CROW_WEBSOCKET_ROUTE(app, path) app.route_websocket(path)

} // namespace crow

#endif // CROW_ALL_H
