// json.hpp - Minimal nlohmann-compatible JSON library for TransitX
// Single-header JSON implementation supporting object, array, string, number, bool, null
#pragma once
#ifndef TRANSITX_JSON_HPP
#define TRANSITX_JSON_HPP

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <initializer_list>
#include <iostream>
#include <cstring>
#include <memory>

namespace nlohmann {

class json {
public:
    enum class value_t {
        null_value,
        object,
        array,
        string,
        number_integer,
        number_float,
        boolean
    };

private:
    value_t type_;
    std::string str_val_;
    int64_t int_val_;
    double float_val_;
    bool bool_val_;
    std::map<std::string, json> obj_val_;
    std::vector<json> arr_val_;

    static std::string escape_string(const std::string& s) {
        std::string result;
        result.reserve(s.size() + 2);
        result += '"';
        for (char c : s) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        char buf[8];
                        snprintf(buf, sizeof(buf), "\\u%04x", static_cast<int>(c));
                        result += buf;
                    } else {
                        result += c;
                    }
            }
        }
        result += '"';
        return result;
    }

    static void skip_whitespace(const std::string& s, size_t& pos) {
        while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' || s[pos] == '\r'))
            ++pos;
    }

    static std::string parse_string_value(const std::string& s, size_t& pos) {
        if (pos >= s.size() || s[pos] != '"')
            throw std::runtime_error("Expected '\"' at position " + std::to_string(pos));
        ++pos;
        std::string result;
        while (pos < s.size() && s[pos] != '"') {
            if (s[pos] == '\\') {
                ++pos;
                if (pos >= s.size()) throw std::runtime_error("Unexpected end of string");
                switch (s[pos]) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case 'u': {
                        if (pos + 4 >= s.size()) throw std::runtime_error("Invalid unicode escape");
                        std::string hex = s.substr(pos + 1, 4);
                        int codepoint = std::stoi(hex, nullptr, 16);
                        if (codepoint < 0x80) {
                            result += static_cast<char>(codepoint);
                        } else if (codepoint < 0x800) {
                            result += static_cast<char>(0xC0 | (codepoint >> 6));
                            result += static_cast<char>(0x80 | (codepoint & 0x3F));
                        } else {
                            result += static_cast<char>(0xE0 | (codepoint >> 12));
                            result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                            result += static_cast<char>(0x80 | (codepoint & 0x3F));
                        }
                        pos += 4;
                        break;
                    }
                    default: result += s[pos];
                }
            } else {
                result += s[pos];
            }
            ++pos;
        }
        if (pos >= s.size()) throw std::runtime_error("Unterminated string");
        ++pos; // skip closing quote
        return result;
    }

    static json parse_value(const std::string& s, size_t& pos) {
        skip_whitespace(s, pos);
        if (pos >= s.size()) throw std::runtime_error("Unexpected end of input");

        if (s[pos] == '"') {
            return json(parse_string_value(s, pos));
        } else if (s[pos] == '{') {
            return parse_object(s, pos);
        } else if (s[pos] == '[') {
            return parse_array(s, pos);
        } else if (s[pos] == 't') {
            if (s.substr(pos, 4) == "true") { pos += 4; return json(true); }
            throw std::runtime_error("Invalid value at position " + std::to_string(pos));
        } else if (s[pos] == 'f') {
            if (s.substr(pos, 5) == "false") { pos += 5; return json(false); }
            throw std::runtime_error("Invalid value at position " + std::to_string(pos));
        } else if (s[pos] == 'n') {
            if (s.substr(pos, 4) == "null") { pos += 4; return json(); }
            throw std::runtime_error("Invalid value at position " + std::to_string(pos));
        } else if (s[pos] == '-' || (s[pos] >= '0' && s[pos] <= '9')) {
            return parse_number(s, pos);
        }
        throw std::runtime_error("Unexpected character '" + std::string(1, s[pos]) + "' at position " + std::to_string(pos));
    }

    static json parse_object(const std::string& s, size_t& pos) {
        json result = json::object();
        ++pos; // skip '{'
        skip_whitespace(s, pos);
        if (pos < s.size() && s[pos] == '}') { ++pos; return result; }
        while (pos < s.size()) {
            skip_whitespace(s, pos);
            std::string key = parse_string_value(s, pos);
            skip_whitespace(s, pos);
            if (pos >= s.size() || s[pos] != ':') throw std::runtime_error("Expected ':' in object");
            ++pos;
            result[key] = parse_value(s, pos);
            skip_whitespace(s, pos);
            if (pos < s.size() && s[pos] == ',') { ++pos; continue; }
            if (pos < s.size() && s[pos] == '}') { ++pos; return result; }
            throw std::runtime_error("Expected ',' or '}' in object");
        }
        throw std::runtime_error("Unterminated object");
    }

    static json parse_array(const std::string& s, size_t& pos) {
        json result = json::array();
        ++pos; // skip '['
        skip_whitespace(s, pos);
        if (pos < s.size() && s[pos] == ']') { ++pos; return result; }
        while (pos < s.size()) {
            result.push_back(parse_value(s, pos));
            skip_whitespace(s, pos);
            if (pos < s.size() && s[pos] == ',') { ++pos; continue; }
            if (pos < s.size() && s[pos] == ']') { ++pos; return result; }
            throw std::runtime_error("Expected ',' or ']' in array");
        }
        throw std::runtime_error("Unterminated array");
    }

    static json parse_number(const std::string& s, size_t& pos) {
        size_t start = pos;
        bool is_float = false;
        if (s[pos] == '-') ++pos;
        while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') ++pos;
        if (pos < s.size() && s[pos] == '.') { is_float = true; ++pos; while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') ++pos; }
        if (pos < s.size() && (s[pos] == 'e' || s[pos] == 'E')) { is_float = true; ++pos; if (pos < s.size() && (s[pos] == '+' || s[pos] == '-')) ++pos; while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') ++pos; }
        std::string num_str = s.substr(start, pos - start);
        if (is_float) return json(std::stod(num_str));
        return json(static_cast<int64_t>(std::stoll(num_str)));
    }

public:
    // Constructors
    json() : type_(value_t::null_value), int_val_(0), float_val_(0), bool_val_(false) {}
    json(std::nullptr_t) : type_(value_t::null_value), int_val_(0), float_val_(0), bool_val_(false) {}
    json(bool v) : type_(value_t::boolean), int_val_(0), float_val_(0), bool_val_(v) {}
    json(int v) : type_(value_t::number_integer), int_val_(v), float_val_(0), bool_val_(false) {}
    json(int64_t v) : type_(value_t::number_integer), int_val_(v), float_val_(0), bool_val_(false) {}
    json(size_t v) : type_(value_t::number_integer), int_val_(static_cast<int64_t>(v)), float_val_(0), bool_val_(false) {}
    json(double v) : type_(value_t::number_float), int_val_(0), float_val_(v), bool_val_(false) {}
    json(const char* v) : type_(value_t::string), str_val_(v), int_val_(0), float_val_(0), bool_val_(false) {}
    json(const std::string& v) : type_(value_t::string), str_val_(v), int_val_(0), float_val_(0), bool_val_(false) {}
    json(std::string&& v) : type_(value_t::string), str_val_(std::move(v)), int_val_(0), float_val_(0), bool_val_(false) {}
    json(std::initializer_list<std::pair<const std::string, json>> init)
        : type_(value_t::object), int_val_(0), float_val_(0), bool_val_(false), obj_val_(init) {}

    // Named constructors
    static json object() { json j; j.type_ = value_t::object; return j; }
    static json array() { json j; j.type_ = value_t::array; return j; }

    // Type checks
    bool is_null() const { return type_ == value_t::null_value; }
    bool is_object() const { return type_ == value_t::object; }
    bool is_array() const { return type_ == value_t::array; }
    bool is_string() const { return type_ == value_t::string; }
    bool is_number() const { return type_ == value_t::number_integer || type_ == value_t::number_float; }
    bool is_number_integer() const { return type_ == value_t::number_integer; }
    bool is_number_float() const { return type_ == value_t::number_float; }
    bool is_boolean() const { return type_ == value_t::boolean; }

    // Value access
    template<typename T>
    T get() const;

    // operator[] for objects
    json& operator[](const std::string& key) {
        if (type_ == value_t::null_value) type_ = value_t::object;
        if (type_ != value_t::object) throw std::runtime_error("Cannot use operator[] with non-object type");
        return obj_val_[key];
    }

    const json& operator[](const std::string& key) const {
        if (type_ != value_t::object) throw std::runtime_error("Cannot use operator[] with non-object type");
        auto it = obj_val_.find(key);
        if (it == obj_val_.end()) {
            static const json null_json;
            return null_json;
        }
        return it->second;
    }

    // operator[] for arrays
    json& operator[](size_t index) {
        if (type_ != value_t::array) throw std::runtime_error("Cannot use operator[] with non-array type");
        if (index >= arr_val_.size()) throw std::out_of_range("Array index out of range");
        return arr_val_[index];
    }

    const json& operator[](size_t index) const {
        if (type_ != value_t::array) throw std::runtime_error("Cannot use operator[] with non-array type");
        if (index >= arr_val_.size()) throw std::out_of_range("Array index out of range");
        return arr_val_[index];
    }

    // Array operations
    void push_back(const json& val) {
        if (type_ == value_t::null_value) type_ = value_t::array;
        if (type_ != value_t::array) throw std::runtime_error("Cannot push_back to non-array");
        arr_val_.push_back(val);
    }

    void push_back(json&& val) {
        if (type_ == value_t::null_value) type_ = value_t::array;
        if (type_ != value_t::array) throw std::runtime_error("Cannot push_back to non-array");
        arr_val_.push_back(std::move(val));
    }

    // Size
    size_t size() const {
        if (type_ == value_t::array) return arr_val_.size();
        if (type_ == value_t::object) return obj_val_.size();
        return 0;
    }

    bool empty() const {
        if (type_ == value_t::array) return arr_val_.empty();
        if (type_ == value_t::object) return obj_val_.empty();
        if (type_ == value_t::string) return str_val_.empty();
        if (type_ == value_t::null_value) return true;
        return false;
    }

    // Contains
    bool contains(const std::string& key) const {
        if (type_ != value_t::object) return false;
        return obj_val_.find(key) != obj_val_.end();
    }

    size_t count(const std::string& key) const {
        if (type_ != value_t::object) return 0;
        return obj_val_.count(key);
    }

    // Erase
    size_t erase(const std::string& key) {
        if (type_ != value_t::object) return 0;
        return obj_val_.erase(key);
    }

    // Value method with default
    template<typename T>
    T value(const std::string& key, const T& default_val) const;

    std::string value(const std::string& key, const char* default_val) const;

    // Iterators for objects
    auto begin() -> decltype(obj_val_.begin()) { return obj_val_.begin(); }
    auto end() -> decltype(obj_val_.end()) { return obj_val_.end(); }
    auto begin() const -> decltype(obj_val_.begin()) { return obj_val_.begin(); }
    auto end() const -> decltype(obj_val_.end()) { return obj_val_.end(); }

    // Array iterators
    auto array_begin() -> decltype(arr_val_.begin()) { return arr_val_.begin(); }
    auto array_end() -> decltype(arr_val_.end()) { return arr_val_.end(); }
    auto array_begin() const -> decltype(arr_val_.begin()) { return arr_val_.begin(); }
    auto array_end() const -> decltype(arr_val_.end()) { return arr_val_.end(); }

    const std::vector<json>& array_items() const { return arr_val_; }
    std::vector<json>& array_items() { return arr_val_; }

    // Items for range-for over objects
    const std::map<std::string, json>& items() const { return obj_val_; }
    std::map<std::string, json>& items() { return obj_val_; }

    // Dump to string
    std::string dump(int indent = -1, char indent_char = ' ', int current_indent = 0) const {
        std::string result;
        bool pretty = indent >= 0;
        std::string newline = pretty ? "\n" : "";
        std::string ind = pretty ? std::string(static_cast<size_t>(current_indent + indent), indent_char) : "";
        std::string base_ind = pretty ? std::string(static_cast<size_t>(current_indent), indent_char) : "";

        switch (type_) {
            case value_t::null_value: return "null";
            case value_t::boolean: return bool_val_ ? "true" : "false";
            case value_t::number_integer: return std::to_string(int_val_);
            case value_t::number_float: {
                if (std::isnan(float_val_) || std::isinf(float_val_)) return "null";
                std::ostringstream oss;
                oss << float_val_;
                std::string s = oss.str();
                if (s.find('.') == std::string::npos && s.find('e') == std::string::npos) s += ".0";
                return s;
            }
            case value_t::string: return escape_string(str_val_);
            case value_t::object: {
                if (obj_val_.empty()) return "{}";
                result = "{" + newline;
                bool first = true;
                for (auto& kv : obj_val_) {
                    if (!first) result += "," + newline;
                    first = false;
                    result += ind + escape_string(kv.first) + ":" + (pretty ? " " : "");
                    result += kv.second.dump(indent, indent_char, current_indent + indent);
                }
                result += newline + base_ind + "}";
                return result;
            }
            case value_t::array: {
                if (arr_val_.empty()) return "[]";
                result = "[" + newline;
                bool first = true;
                for (auto& v : arr_val_) {
                    if (!first) result += "," + newline;
                    first = false;
                    result += ind + v.dump(indent, indent_char, current_indent + indent);
                }
                result += newline + base_ind + "]";
                return result;
            }
        }
        return "null";
    }

    // Parse from string
    static json parse(const std::string& s) {
        size_t pos = 0;
        json result = parse_value(s, pos);
        skip_whitespace(s, pos);
        return result;
    }

    // to_string
    friend std::string to_string(const json& j) { return j.dump(); }

    // Stream operators
    friend std::ostream& operator<<(std::ostream& os, const json& j) {
        os << j.dump();
        return os;
    }

    // Type accessor
    value_t type() const { return type_; }

    // Merge patch (simple)
    void merge_patch(const json& patch) {
        if (patch.is_object()) {
            if (!is_object()) { type_ = value_t::object; obj_val_.clear(); }
            for (auto& kv : patch.obj_val_) {
                if (kv.second.is_null()) {
                    obj_val_.erase(kv.first);
                } else {
                    obj_val_[kv.first].merge_patch(kv.second);
                }
            }
        } else {
            *this = patch;
        }
    }
};

// Template specializations for get<T>
template<> inline std::string json::get<std::string>() const {
    if (type_ == value_t::string) return str_val_;
    if (type_ == value_t::number_integer) return std::to_string(int_val_);
    if (type_ == value_t::number_float) { std::ostringstream oss; oss << float_val_; return oss.str(); }
    if (type_ == value_t::boolean) return bool_val_ ? "true" : "false";
    if (type_ == value_t::null_value) return "";
    throw std::runtime_error("Cannot convert to string");
}

template<> inline int json::get<int>() const {
    if (type_ == value_t::number_integer) return static_cast<int>(int_val_);
    if (type_ == value_t::number_float) return static_cast<int>(float_val_);
    if (type_ == value_t::string) return std::stoi(str_val_);
    throw std::runtime_error("Cannot convert to int");
}

template<> inline int64_t json::get<int64_t>() const {
    if (type_ == value_t::number_integer) return int_val_;
    if (type_ == value_t::number_float) return static_cast<int64_t>(float_val_);
    if (type_ == value_t::string) return std::stoll(str_val_);
    throw std::runtime_error("Cannot convert to int64_t");
}

template<> inline size_t json::get<size_t>() const {
    if (type_ == value_t::number_integer) return static_cast<size_t>(int_val_);
    if (type_ == value_t::number_float) return static_cast<size_t>(float_val_);
    throw std::runtime_error("Cannot convert to size_t");
}

template<> inline double json::get<double>() const {
    if (type_ == value_t::number_float) return float_val_;
    if (type_ == value_t::number_integer) return static_cast<double>(int_val_);
    if (type_ == value_t::string) return std::stod(str_val_);
    throw std::runtime_error("Cannot convert to double");
}

template<> inline bool json::get<bool>() const {
    if (type_ == value_t::boolean) return bool_val_;
    if (type_ == value_t::number_integer) return int_val_ != 0;
    throw std::runtime_error("Cannot convert to bool");
}

template<typename T>
inline T json::value(const std::string& key, const T& default_val) const {
    if (type_ != value_t::object) return default_val;
    auto it = obj_val_.find(key);
    if (it == obj_val_.end() || it->second.is_null()) return default_val;
    return it->second.get<T>();
}

inline std::string json::value(const std::string& key, const char* default_val) const {
    if (type_ != value_t::object) return default_val ? default_val : "";
    auto it = obj_val_.find(key);
    if (it == obj_val_.end() || it->second.is_null()) return default_val ? default_val : "";
    return it->second.get<std::string>();
}

// Allow implicit conversion from json to common types
inline void to_json(json& j, const std::string& v) { j = json(v); }
inline void to_json(json& j, const char* v) { j = json(v); }
inline void to_json(json& j, int v) { j = json(v); }
inline void to_json(json& j, int64_t v) { j = json(v); }
inline void to_json(json& j, double v) { j = json(v); }
inline void to_json(json& j, bool v) { j = json(v); }

inline void from_json(const json& j, std::string& v) { v = j.get<std::string>(); }
inline void from_json(const json& j, int& v) { v = j.get<int>(); }
inline void from_json(const json& j, int64_t& v) { v = j.get<int64_t>(); }
inline void from_json(const json& j, double& v) { v = j.get<double>(); }
inline void from_json(const json& j, bool& v) { v = j.get<bool>(); }

} // namespace nlohmann

#endif // TRANSITX_JSON_HPP
