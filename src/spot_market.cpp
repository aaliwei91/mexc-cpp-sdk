#include "mexc/spot/market.hpp"
#include "mexc/base.hpp"
#include "mexc/json_util.hpp"
#include <algorithm>
#include <cctype>

namespace mexc {
namespace spot {

namespace {
std::string toUpper(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}
} // namespace

SpotMarket::SpotMarket(const std::string& apiKey, const std::string& apiSecret)
    : apiKey_(apiKey), apiSecret_(apiSecret) {}

Json::Value SpotMarket::publicGet(const std::string& path,
                                      const std::map<std::string, std::string>& params) {
    std::string url = SPOT_BASE_URL + path;
    std::map<std::string, std::string> p = params;
    removeEmptyParams(p);
    if (!p.empty())
        url += "?" + buildQueryString(p);
    std::map<std::string, std::string> headers = {{"Content-Type", "application/json"}};
    if (!apiKey_.empty()) headers["X-MEXC-APIKEY"] = apiKey_;
    HttpResponse res = httpRequest("GET", url, "", headers, opts_);
    if (!res.ok() && !res.body.empty()) return jsonParse(res.body);
    if (res.body.empty()) return Json::Value();
    return jsonParse(res.body);
}

Json::Value SpotMarket::exchangeInfo(const std::map<std::string, std::string>& options) {
    std::map<std::string, std::string> opts = options;
    if (opts.count("symbol")) opts["symbol"] = toUpper(opts["symbol"]);
    if (opts.count("symbols")) {
        std::string s = opts["symbols"];
        std::string out;
        size_t i = 0;
        while (i < s.size()) {
            while (i < s.size() && (s[i] == ',' || s[i] == ' ')) ++i;
            size_t start = i;
            while (i < s.size() && s[i] != ',') ++i;
            if (start < i) {
                if (!out.empty()) out += ',';
                out += toUpper(s.substr(start, i - start));
            }
        }
        opts["symbols"] = out;
    }
    return publicGet("/exchangeInfo", opts);
}

Json::Value SpotMarket::depth(const std::string& symbol, int limit) {
    std::map<std::string, std::string> p = {{"symbol", toUpper(symbol)}, {"limit", std::to_string(limit)}};
    return publicGet("/depth", p);
}

Json::Value SpotMarket::trades(const std::string& symbol, int limit) {
    std::map<std::string, std::string> p = {{"symbol", toUpper(symbol)}, {"limit", std::to_string(limit)}};
    return publicGet("/trades", p);
}

Json::Value SpotMarket::historicalTrades(const std::string& symbol, int limit) {
    std::map<std::string, std::string> p = {{"symbol", toUpper(symbol)}, {"limit", std::to_string(limit)}};
    return publicGet("/historicalTrades", p);
}

Json::Value SpotMarket::aggTrades(const std::string& symbol,
                                     const std::map<std::string, std::string>& options) {
    std::map<std::string, std::string> p = options;
    p["symbol"] = toUpper(symbol);
    if (p.find("limit") == p.end()) p["limit"] = "500";
    return publicGet("/aggTrades", p);
}

Json::Value SpotMarket::klines(const std::string& symbol,
                                  const std::string& interval,
                                  const std::map<std::string, std::string>& options) {
    std::map<std::string, std::string> p = options;
    p["symbol"] = toUpper(symbol);
    p["interval"] = interval;
    if (p.find("limit") == p.end()) p["limit"] = "500";
    return publicGet("/klines", p);
}

Json::Value SpotMarket::avgPrice(const std::string& symbol) {
    return publicGet("/avgPrice", {{"symbol", toUpper(symbol)}});
}

Json::Value SpotMarket::ticker24hr(const std::string& symbol) {
    std::map<std::string, std::string> p;
    if (!symbol.empty()) p["symbol"] = toUpper(symbol);
    return publicGet("/ticker/24hr", p);
}

Json::Value SpotMarket::tickerPrice(const std::string& symbol) {
    std::map<std::string, std::string> p;
    if (!symbol.empty()) p["symbol"] = toUpper(symbol);
    return publicGet("/ticker/price", p);
}

Json::Value SpotMarket::bookTicker(const std::string& symbol) {
    std::map<std::string, std::string> p;
    if (!symbol.empty()) p["symbol"] = toUpper(symbol);
    return publicGet("/ticker/bookTicker", p);
}

Json::Value SpotMarket::ping() {
    return publicGet("/ping");
}

Json::Value SpotMarket::time() {
    return publicGet("/time");
}

} // namespace spot
} // namespace mexc
