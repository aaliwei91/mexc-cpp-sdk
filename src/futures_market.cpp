#include "mexc/futures/market.hpp"
#include "mexc/base.hpp"
#include "mexc/json_util.hpp"

namespace mexc {
namespace futures {

FuturesMarket::FuturesMarket() = default;

Json::Value FuturesMarket::publicGet(const std::string& path,
                                         const std::map<std::string, std::string>& params) {
    std::string url = FUTURES_BASE_URL + path;
    std::map<std::string, std::string> p = params;
    removeEmptyParams(p);
    if (!p.empty()) url += "?" + buildQueryString(p);
    std::map<std::string, std::string> headers = {{"Content-Type", "application/json"}};
    HttpResponse res = httpRequest("GET", url, "", headers, opts_);
    if (res.body.empty()) return Json::Value();
    Json::Value j = jsonParse(res.body);
    if (j.isMember("data")) return j["data"];
    return j;
}

Json::Value FuturesMarket::ping() {
    HttpResponse res = httpRequest("GET", FUTURES_BASE_URL + "/api/v1/contract/ping", "", {}, opts_);
    if (res.body.empty()) return Json::Value();
    return jsonParse(res.body);
}

Json::Value FuturesMarket::contractDetail(const std::string& symbol) {
    if (symbol.empty()) return publicGet("/api/v1/contract/detail");
    return publicGet("/api/v1/contract/detail", {{"symbol", symbol}});
}

Json::Value FuturesMarket::supportCurrencies() {
    return publicGet("/api/v1/contract/support_currencies");
}

Json::Value FuturesMarket::depth(const std::string& symbol, int limit) {
    std::string path = "/api/v1/contract/depth/" + symbol;
    std::map<std::string, std::string> p;
    if (limit > 0) p["limit"] = std::to_string(limit);
    return publicGet(path, p);
}

Json::Value FuturesMarket::depthCommits(const std::string& symbol, int limit) {
    return publicGet("/api/v1/contract/depth_commits/" + symbol + "/" + std::to_string(limit));
}

Json::Value FuturesMarket::indexPrice(const std::string& symbol) {
    return publicGet("/api/v1/contract/index_price/" + symbol);
}

Json::Value FuturesMarket::fairPrice(const std::string& symbol) {
    return publicGet("/api/v1/contract/fair_price/" + symbol);
}

Json::Value FuturesMarket::fundingRate(const std::string& symbol) {
    return publicGet("/api/v1/contract/funding_rate/" + symbol);
}

Json::Value FuturesMarket::kline(const std::string& symbol,
                                    const std::map<std::string, std::string>& options) {
    return publicGet("/api/v1/contract/kline/" + symbol, options);
}

Json::Value FuturesMarket::klineIndexPrice(const std::string& symbol,
                                              const std::map<std::string, std::string>& options) {
    std::string url = FUTURES_BASE_URL + "/api/v1/contract/kline/index_price/" + symbol;
    if (!options.empty()) url += "?" + buildQueryString(options);
    HttpResponse res = httpRequest("GET", url, "", {{"Content-Type", "application/json"}}, opts_);
    if (res.body.empty()) return Json::Value();
    Json::Value full = jsonParse(res.body);
    if (full.isMember("data")) return full["data"];
    return full;
}

Json::Value FuturesMarket::klineFairPrice(const std::string& symbol,
                                             const std::map<std::string, std::string>& options) {
    std::string url = FUTURES_BASE_URL + "/api/v1/contract/kline/fair_price/" + symbol;
    if (!options.empty()) url += "?" + buildQueryString(options);
    HttpResponse res = httpRequest("GET", url, "", {{"Content-Type", "application/json"}}, opts_);
    if (res.body.empty()) return Json::Value();
    Json::Value full = jsonParse(res.body);
    if (full.isMember("data")) return full["data"];
    return full;
}

Json::Value FuturesMarket::deals(const std::string& symbol, int limit) {
    return publicGet("/api/v1/contract/deals/" + symbol, {{"limit", std::to_string(limit)}});
}

Json::Value FuturesMarket::ticker(const std::string& symbol) {
    std::map<std::string, std::string> p;
    if (!symbol.empty()) p["symbol"] = symbol;
    return publicGet("/api/v1/contract/ticker", p);
}

Json::Value FuturesMarket::riskReverse(const std::string& symbol) {
    return publicGet("/api/v1/contract/risk_reverse/" + symbol);
}

Json::Value FuturesMarket::riskReverseHistory(const std::string& symbol, int pageNum, int pageSize) {
    std::map<std::string, std::string> p = {{"symbol", symbol},
                                            {"page_num", std::to_string(pageNum)},
                                            {"page_size", std::to_string(pageSize)}};
    return publicGet("/api/v1/contract/risk_reverse/history", p);
}

Json::Value FuturesMarket::fundingRateHistory(const std::string& symbol, int pageNum, int pageSize) {
    std::map<std::string, std::string> p = {{"symbol", symbol},
                                            {"page_num", std::to_string(pageNum)},
                                            {"page_size", std::to_string(pageSize)}};
    return publicGet("/api/v1/contract/funding_rate/history", p);
}

} // namespace futures
} // namespace mexc
