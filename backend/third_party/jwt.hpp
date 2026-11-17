// jwt.hpp - Minimal JWT (JSON Web Token) implementation for TransitX
// Supports HS256 signing, token creation, verification, and claim extraction
#pragma once
#ifndef TRANSITX_JWT_HPP
#define TRANSITX_JWT_HPP

#include <string>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <cstdint>
#include "json.hpp"

namespace jwt {

namespace detail {

// Base64URL encoding/decoding
static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline std::string base64_encode(const std::string& input) {
    std::string ret;
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    const char* bytes = input.c_str();
    size_t len = input.size();
    size_t pos = 0;

    while (pos < len) {
        i = 0;
        for (i = 0; i < 3 && pos < len; i++, pos++)
            char_array_3[i] = static_cast<unsigned char>(bytes[pos]);
        for (int j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (int j = 0; j < i + 1; j++)
            ret += base64_chars[char_array_4[j]];
    }

    return ret;
}

inline std::string base64url_encode(const std::string& input) {
    std::string b64 = base64_encode(input);
    // Replace + with -, / with _, remove =
    for (auto& c : b64) {
        if (c == '+') c = '-';
        else if (c == '/') c = '_';
    }
    // Remove trailing =
    while (!b64.empty() && b64.back() == '=') b64.pop_back();
    return b64;
}

inline std::string base64url_decode(const std::string& input) {
    std::string b64 = input;
    for (auto& c : b64) {
        if (c == '-') c = '+';
        else if (c == '_') c = '/';
    }
    while (b64.size() % 4 != 0) b64 += '=';

    std::string ret;
    int i = 0;
    unsigned char char_array_4[4], char_array_3[3];
    size_t pos = 0;

    auto is_base64 = [](unsigned char c) -> bool {
        return (isalnum(c) || c == '+' || c == '/');
    };

    while (pos < b64.size() && b64[pos] != '=' && is_base64(static_cast<unsigned char>(b64[pos]))) {
        char_array_4[i++] = static_cast<unsigned char>(b64[pos]); pos++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                const char* p = strchr(base64_chars, char_array_4[i]);
                char_array_4[i] = p ? static_cast<unsigned char>(p - base64_chars) : 0;
            }
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            for (i = 0; i < 3; i++) ret += static_cast<char>(char_array_3[i]);
            i = 0;
        }
    }

    if (i) {
        for (int j = i; j < 4; j++) char_array_4[j] = 0;
        for (int j = 0; j < 4; j++) {
            const char* p = strchr(base64_chars, char_array_4[j]);
            char_array_4[j] = p ? static_cast<unsigned char>(p - base64_chars) : 0;
        }
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        for (int j = 0; j < i - 1; j++) ret += static_cast<char>(char_array_3[j]);
    }

    return ret;
}

// HMAC-SHA256 implementation
// SHA-256 constants
static const uint32_t k256[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

inline uint32_t rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
inline uint32_t sigma0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
inline uint32_t sigma1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
inline uint32_t gamma0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
inline uint32_t gamma1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

struct Sha256 {
    uint32_t state[8];
    uint64_t count;
    uint8_t buffer[64];

    Sha256() { reset(); }

    void reset() {
        state[0] = 0x6a09e667; state[1] = 0xbb67ae85;
        state[2] = 0x3c6ef372; state[3] = 0xa54ff53a;
        state[4] = 0x510e527f; state[5] = 0x9b05688c;
        state[6] = 0x1f83d9ab; state[7] = 0x5be0cd19;
        count = 0;
        memset(buffer, 0, 64);
    }

    void transform(const uint8_t block[64]) {
        uint32_t w[64];
        for (int i = 0; i < 16; i++) {
            w[i] = (static_cast<uint32_t>(block[i * 4]) << 24) |
                   (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
                   static_cast<uint32_t>(block[i * 4 + 3]);
        }
        for (int i = 16; i < 64; i++)
            w[i] = gamma1(w[i - 2]) + w[i - 7] + gamma0(w[i - 15]) + w[i - 16];

        uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
        uint32_t e = state[4], f = state[5], g = state[6], h = state[7];

        for (int i = 0; i < 64; i++) {
            uint32_t t1 = h + sigma1(e) + ch(e, f, g) + k256[i] + w[i];
            uint32_t t2 = sigma0(a) + maj(a, b, c);
            h = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
        }

        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
        state[4] += e; state[5] += f; state[6] += g; state[7] += h;
    }

    void update(const uint8_t* data, size_t len) {
        for (size_t i = 0; i < len; i++) {
            buffer[count % 64] = data[i];
            count++;
            if (count % 64 == 0) transform(buffer);
        }
    }

    void update(const std::string& s) { update(reinterpret_cast<const uint8_t*>(s.c_str()), s.size()); }

    std::string final_raw() {
        uint64_t bits = count * 8;
        uint8_t pad = static_cast<uint8_t>(count % 64);
        uint8_t padding[64] = {0x80};
        size_t pad_len = (pad < 56) ? (56 - pad) : (120 - pad);
        update(padding, pad_len);
        uint8_t bits_be[8];
        for (int i = 7; i >= 0; i--) { bits_be[i] = static_cast<uint8_t>(bits & 0xff); bits >>= 8; }
        update(bits_be, 8);
        std::string result(32, '\0');
        for (int i = 0; i < 8; i++) {
            result[i * 4]     = static_cast<char>((state[i] >> 24) & 0xff);
            result[i * 4 + 1] = static_cast<char>((state[i] >> 16) & 0xff);
            result[i * 4 + 2] = static_cast<char>((state[i] >> 8) & 0xff);
            result[i * 4 + 3] = static_cast<char>(state[i] & 0xff);
        }
        return result;
    }
};

inline std::string sha256(const std::string& data) {
    Sha256 h;
    h.update(data);
    return h.final_raw();
}

inline std::string hmac_sha256(const std::string& key, const std::string& data) {
    std::string k = key;
    if (k.size() > 64) k = sha256(k);
    while (k.size() < 64) k += '\0';

    std::string o_key_pad(64, '\0');
    std::string i_key_pad(64, '\0');
    for (int i = 0; i < 64; i++) {
        o_key_pad[i] = static_cast<char>(k[i] ^ 0x5c);
        i_key_pad[i] = static_cast<char>(k[i] ^ 0x36);
    }

    std::string inner = sha256(i_key_pad + data);
    return sha256(o_key_pad + inner);
}

} // namespace detail

struct decoded_jwt {
    nlohmann::json header;
    nlohmann::json payload;
    std::string signature;
};

// Create a JWT token
inline std::string create(const nlohmann::json& payload, const std::string& secret, int expiry_seconds = 3600) {
    nlohmann::json header;
    header["alg"] = "HS256";
    header["typ"] = "JWT";

    nlohmann::json p = payload;
    p["iat"] = static_cast<int64_t>(std::time(nullptr));
    p["exp"] = static_cast<int64_t>(std::time(nullptr) + expiry_seconds);

    std::string header_enc = detail::base64url_encode(header.dump());
    std::string payload_enc = detail::base64url_encode(p.dump());
    std::string sign_input = header_enc + "." + payload_enc;
    std::string sig = detail::hmac_sha256(secret, sign_input);
    std::string sig_enc = detail::base64url_encode(sig);

    return sign_input + "." + sig_enc;
}

// Decode a JWT token (without verification)
inline decoded_jwt decode(const std::string& token) {
    size_t first_dot = token.find('.');
    size_t second_dot = token.find('.', first_dot + 1);

    if (first_dot == std::string::npos || second_dot == std::string::npos)
        throw std::runtime_error("Invalid JWT format");

    decoded_jwt result;
    result.header = nlohmann::json::parse(detail::base64url_decode(token.substr(0, first_dot)));
    result.payload = nlohmann::json::parse(detail::base64url_decode(token.substr(first_dot + 1, second_dot - first_dot - 1)));
    result.signature = token.substr(second_dot + 1);
    return result;
}

// Verify a JWT token
inline bool verify(const std::string& token, const std::string& secret) {
    try {
        size_t second_dot = token.rfind('.');
        if (second_dot == std::string::npos) return false;

        std::string sign_input = token.substr(0, second_dot);
        std::string expected_sig = detail::base64url_encode(detail::hmac_sha256(secret, sign_input));
        std::string actual_sig = token.substr(second_dot + 1);

        if (expected_sig != actual_sig) return false;

        decoded_jwt d = decode(token);
        if (d.payload.contains("exp")) {
            int64_t exp = d.payload["exp"].get<int64_t>();
            if (std::time(nullptr) > exp) return false;
        }
        return true;
    } catch (...) {
        return false;
    }
}

// Extract claims from a verified token
inline nlohmann::json get_claims(const std::string& token, const std::string& secret) {
    if (!verify(token, secret))
        throw std::runtime_error("Invalid or expired token");
    return decode(token).payload;
}

} // namespace jwt

#endif // TRANSITX_JWT_HPP
