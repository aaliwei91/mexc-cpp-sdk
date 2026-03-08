// Minimal nlohmann::json-compatible header for mexc-cpp-api (no network required).
// For full nlohmann/json, replace this file with the official single-include from
// https://github.com/nlohmann/json/releases
#ifndef NLOHMANN_JSON_MINIMAL_HPP
#define NLOHMANN_JSON_MINIMAL_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <cmath>

namespace nlohmann {

class json {
public:
    enum type_t { null, boolean, number, string, array, object };
    using array_t = std::vector<json>;
    using object_t = std::map<std::string, json>;

    json() : type_(null) {}
    json(std::nullptr_t) : type_(null) {}
    json(bool b) : type_(boolean), b_(b) {}
    json(int i) : type_(number), n_(static_cast<double>(i)) {}
    json(unsigned u) : type_(number), n_(static_cast<double>(u)) {}
    json(long long l) : type_(number), n_(static_cast<double>(l)) {}
    json(double d) : type_(number), n_(d) {}
    json(const char* s) : type_(string), s_(s) {}
    json(const std::string& s) : type_(string), s_(s) {}
    json(std::string&& s) : type_(string), s_(std::move(s)) {}
    json(const array_t& a) : type_(array), a_(std::make_shared<array_t>(a)) {}
    json(const object_t& o) : type_(object), o_(std::make_shared<object_t>(o)) {}
    json(const std::map<std::string, std::string>& m) : type_(object), o_(std::make_shared<object_t>()) {
        for (const auto& p : m) (*o_)[p.first] = json(p.second);
    }
    json(const std::initializer_list<std::pair<std::string, json>>& init) : type_(object), o_(std::make_shared<object_t>()) {
        for (const auto& p : init) (*o_)[p.first] = p.second;
    }

    static json parse(const std::string& s) {
        size_t i = 0;
        return parse_value(s, i);
    }

    static json parse(const char* s) { return parse(std::string(s)); }

    bool is_null() const { return type_ == null; }
    bool is_boolean() const { return type_ == boolean; }
    bool is_number() const { return type_ == number; }
    bool is_string() const { return type_ == string; }
    bool is_array() const { return type_ == array; }
    bool is_object() const { return type_ == object; }

    bool contains(const std::string& key) const {
        return type_ == object && o_ && o_->find(key) != o_->end();
    }
    size_t size() const {
        if (type_ == array && a_) return a_->size();
        if (type_ == object && o_) return o_->size();
        return 0;
    }

    json& operator[](const std::string& key) {
        if (type_ != object) { type_ = object; o_ = std::make_shared<object_t>(); }
        return (*o_)[key];
    }
    const json& operator[](const std::string& key) const {
        static json null_json;
        if (type_ != object || !o_) return null_json;
        auto it = o_->find(key);
        if (it == o_->end()) return null_json;
        return it->second;
    }
    json& operator[](size_t i) {
        if (type_ != array) { type_ = array; a_ = std::make_shared<array_t>(); }
        if (a_->size() <= i) a_->resize(i + 1);
        return (*a_)[i];
    }
    const json& operator[](size_t i) const {
        static json null_json;
        if (type_ != array || !a_ || i >= a_->size()) return null_json;
        return (*a_)[i];
    }

    template<typename T> T get() const;

    json& operator=(const json& j) {
        type_ = j.type_;
        b_ = j.b_; n_ = j.n_; s_ = j.s_;
        a_ = j.a_; o_ = j.o_;
        return *this;
    }
    json& operator=(const std::string& s) { type_ = string; s_ = s; return *this; }
    json& operator=(const char* s) { type_ = string; s_ = s; return *this; }
    json& operator=(int i) { type_ = number; n_ = i; return *this; }
    json& operator=(long long i) { type_ = number; n_ = static_cast<double>(i); return *this; }
    json& operator=(double d) { type_ = number; n_ = d; return *this; }
    json& operator=(bool b) { type_ = boolean; b_ = b; return *this; }
    json& operator=(std::nullptr_t) { type_ = null; return *this; }
    json& operator=(const array_t& a) { type_ = array; a_ = std::make_shared<array_t>(a); return *this; }
    json& operator=(const object_t& o) { type_ = object; o_ = std::make_shared<object_t>(o); return *this; }
    json& operator=(const std::initializer_list<std::pair<std::string, json>>& init) {
        type_ = object; o_ = std::make_shared<object_t>();
        for (const auto& p : init) (*o_)[p.first] = p.second;
        return *this;
    }
    json& operator=(const std::vector<std::string>& v) {
        type_ = array; a_ = std::make_shared<array_t>();
        for (const auto& x : v) a_->push_back(json(x));
        return *this;
    }

    std::string dump(int /*indent*/ = -1) const {
        std::ostringstream out;
        dump_impl(out);
        return out.str();
    }

private:
    type_t type_ = null;
    bool b_ = false;
    double n_ = 0;
    std::string s_;
    std::shared_ptr<array_t> a_;
    std::shared_ptr<object_t> o_;

    static void skip_ws(const std::string& s, size_t& i) {
        while (i < s.size() && (std::isspace(static_cast<unsigned char>(s[i])))) ++i;
    }
    static json parse_value(const std::string& s, size_t& i) {
        skip_ws(s, i);
        if (i >= s.size()) return json();
        if (s[i] == '"') return parse_string(s, i);
        if (s[i] == '[') return parse_array(s, i);
        if (s[i] == '{') return parse_object(s, i);
        if (s[i] == 'n' && s.substr(i, 4) == "null") { i += 4; return json(); }
        if (s[i] == 't' && s.substr(i, 4) == "true") { i += 4; return json(true); }
        if (s[i] == 'f' && s.substr(i, 5) == "false") { i += 5; return json(false); }
        if (s[i] == '-' || std::isdigit(static_cast<unsigned char>(s[i]))) return parse_number(s, i);
        throw std::runtime_error("json parse error at " + std::to_string(i));
    }
    static json parse_string(const std::string& s, size_t& i) {
        if (s[i] != '"') return json();
        ++i;
        std::string r;
        while (i < s.size() && s[i] != '"') {
            if (s[i] == '\\') {
                ++i;
                if (i >= s.size()) break;
                if (s[i] == 'n') r += '\n';
                else if (s[i] == 'r') r += '\r';
                else if (s[i] == 't') r += '\t';
                else if (s[i] == '"') r += '"';
                else if (s[i] == '\\') r += '\\';
                else r += s[i];
                ++i;
            } else r += s[i++];
        }
        if (i < s.size()) ++i;
        return json(std::move(r));
    }
    static json parse_number(const std::string& s, size_t& i) {
        size_t start = i;
        if (s[i] == '-') ++i;
        while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) ++i;
        if (i < s.size() && s[i] == '.') {
            ++i;
            while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) ++i;
        }
        if (i < s.size() && (s[i] == 'e' || s[i] == 'E')) {
            ++i;
            if (i < s.size() && (s[i] == '+' || s[i] == '-')) ++i;
            while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) ++i;
        }
        return json(std::stod(s.substr(start, i - start)));
    }
    static json parse_array(const std::string& s, size_t& i) {
        if (s[i] != '[') return json();
        ++i;
        array_t arr;
        skip_ws(s, i);
        if (i < s.size() && s[i] == ']') { ++i; return json(arr); }
        while (i < s.size()) {
            arr.push_back(parse_value(s, i));
            skip_ws(s, i);
            if (i >= s.size()) break;
            if (s[i] == ']') { ++i; return json(arr); }
            if (s[i] == ',') ++i;
        }
        return json(arr);
    }
    static json parse_object(const std::string& s, size_t& i) {
        if (s[i] != '{') return json();
        ++i;
        object_t obj;
        skip_ws(s, i);
        if (i < s.size() && s[i] == '}') { ++i; return json(obj); }
        while (i < s.size()) {
            skip_ws(s, i);
            if (i >= s.size() || s[i] != '"') break;
            json key = parse_string(s, i);
            if (!key.is_string()) break;
            skip_ws(s, i);
            if (i >= s.size() || s[i] != ':') break;
            ++i;
            obj[key.s_] = parse_value(s, i);
            skip_ws(s, i);
            if (i >= s.size()) break;
            if (s[i] == '}') { ++i; return json(obj); }
            if (s[i] == ',') ++i;
        }
        return json(obj);
    }
    void dump_impl(std::ostringstream& out) const {
        switch (type_) {
            case null: out << "null"; break;
            case boolean: out << (b_ ? "true" : "false"); break;
            case number:
                if (std::floor(n_) == n_ && n_ >= -1e15 && n_ <= 1e15)
                    out << static_cast<long long>(n_);
                else out << n_;
                break;
            case string:
                out << '"';
                for (unsigned char c : s_) {
                    if (c == '"' || c == '\\') out << '\\';
                    out << static_cast<char>(c);
                }
                out << '"';
                break;
            case array:
                out << '[';
                if (a_) for (size_t i = 0; i < a_->size(); ++i) { if (i) out << ','; (*a_)[i].dump_impl(out); }
                out << ']';
                break;
            case object:
                out << '{';
                if (o_) {
                    bool first = true;
                    for (const auto& p : *o_) {
                        if (!first) out << ',';
                        out << '"' << p.first << '"' << ':';
                        p.second.dump_impl(out);
                        first = false;
                    }
                }
                out << '}';
                break;
        }
    }
};

template<> inline std::string json::get<std::string>() const { return s_; }
template<> inline double json::get<double>() const { return n_; }
template<> inline int json::get<int>() const { return static_cast<int>(n_); }
template<> inline bool json::get<bool>() const { return b_; }

} // namespace nlohmann

#endif
