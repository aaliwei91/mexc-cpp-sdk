#include "mexc/spot/trade.hpp"
#include "mexc/json_util.hpp"
#include <chrono>
#include <cctype>
#include <algorithm>

namespace mexc {
namespace spot {

std::string SpotTrade::toUpper(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

SpotTrade::SpotTrade(const std::string& apiKey, const std::string& apiSecret)
    : SpotMarket(apiKey, apiSecret) {}

Json::Value SpotTrade::signedRequest(const std::string& method,
                                        const std::string& path,
                                        const std::map<std::string, std::string>& params,
                                        const std::string& body) {
    std::map<std::string, std::string> p = params;
    removeEmptyParams(p);
    std::string timestamp = std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    p["timestamp"] = timestamp;
    std::string query = buildQueryString(p);
    std::string sig = spotSign(apiSecret_, query);
    std::string url = SPOT_BASE_URL + path + "?" + query + "&signature=" + sig;
    std::map<std::string, std::string> headers = {{"Content-Type", "application/json"}, {"X-MEXC-APIKEY", apiKey_}};
    HttpResponse res = httpRequest(method, url, body, headers, opts_);
    if (res.body.empty()) return Json::Value();
    return jsonParse(res.body);
}

Json::Value SpotTrade::ping() {
    std::string url = SPOT_BASE_URL + "/ping";
    std::map<std::string, std::string> headers;
    if (!apiKey_.empty()) headers["X-MEXC-APIKEY"] = apiKey_;
    HttpResponse res = httpRequest("GET", url, "", headers, opts_);
    if (res.body.empty()) return Json::Value();
    return jsonParse(res.body);
}

Json::Value SpotTrade::time() {
    std::string url = SPOT_BASE_URL + "/time";
    std::map<std::string, std::string> headers;
    if (!apiKey_.empty()) headers["X-MEXC-APIKEY"] = apiKey_;
    HttpResponse res = httpRequest("GET", url, "", headers, opts_);
    if (res.body.empty()) return Json::Value();
    return jsonParse(res.body);
}

Json::Value SpotTrade::accountInfo() {
    return signedRequest("GET", "/account");
}

Json::Value SpotTrade::accountTradeList(const std::string& symbol,
                                           const std::map<std::string, std::string>& options) {
    std::map<std::string, std::string> p = options;
    p["symbol"] = toUpper(symbol);
    if (p.find("limit") == p.end()) p["limit"] = "500";
    return signedRequest("GET", "/myTrades", p);
}

Json::Value SpotTrade::newOrderTest(const std::string& symbol,
                                       const std::string& side,
                                       const std::string& orderType,
                                       const std::map<std::string, std::string>& options) {
    std::map<std::string, std::string> p = options;
    p["symbol"] = toUpper(symbol);
    p["side"] = toUpper(side);
    p["type"] = toUpper(orderType);
    return signedRequest("POST", "/order/test", p);
}

Json::Value SpotTrade::newOrder(const std::string& symbol,
                                   const std::string& side,
                                   const std::string& orderType,
                                   const std::map<std::string, std::string>& options) {
    std::map<std::string, std::string> p = options;
    p["symbol"] = toUpper(symbol);
    p["side"] = toUpper(side);
    p["type"] = toUpper(orderType);
    return signedRequest("POST", "/order", p);
}

Json::Value SpotTrade::cancelOrder(const std::string& symbol,
                                      const std::map<std::string, std::string>& options) {
    std::map<std::string, std::string> p = options;
    p["symbol"] = toUpper(symbol);
    return signedRequest("DELETE", "/order", p);
}

Json::Value SpotTrade::cancelOpenOrders(const std::string& symbol) {
    return signedRequest("DELETE", "/openOrders", {{"symbol", toUpper(symbol)}});
}

Json::Value SpotTrade::queryOrder(const std::string& symbol,
                                     const std::map<std::string, std::string>& options) {
    std::map<std::string, std::string> p = options;
    p["symbol"] = toUpper(symbol);
    return signedRequest("GET", "/order", p);
}

Json::Value SpotTrade::openOrders(const std::string& symbol) {
    return signedRequest("GET", "/openOrders", {{"symbol", toUpper(symbol)}});
}

Json::Value SpotTrade::allOrders(const std::string& symbol,
                                    const std::map<std::string, std::string>& options) {
    std::map<std::string, std::string> p = options;
    p["symbol"] = toUpper(symbol);
    if (p.find("limit") == p.end()) p["limit"] = "500";
    return signedRequest("GET", "/allOrders", p);
}

Json::Value SpotTrade::createUserDataStream() {
    return signedRequest("POST", "/userDataStream", {});
}

Json::Value SpotTrade::keepaliveUserDataStream(const std::string& listenKey) {
    return signedRequest("PUT", "/userDataStream", {{"listenKey", listenKey}});
}

Json::Value SpotTrade::closeUserDataStream(const std::string& listenKey) {
    return signedRequest("DELETE", "/userDataStream", {{"listenKey", listenKey}});
}

Json::Value SpotTrade::getUserDataStreamListenKeys() {
    return signedRequest("GET", "/userDataStream", {});
}

} // namespace spot
} // namespace mexc
