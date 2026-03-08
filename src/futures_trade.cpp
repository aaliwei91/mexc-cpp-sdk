#include "mexc/futures/trade.hpp"
#include "mexc/base.hpp"
#include "mexc/json_util.hpp"
#include <chrono>

namespace mexc {
namespace futures {

namespace {
std::string reqTime() {
    return std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
}
} // namespace

FuturesTrade::FuturesTrade(const std::string& apiKey, const std::string& apiSecret)
    : apiKey_(apiKey), apiSecret_(apiSecret) {}

Json::Value FuturesTrade::signedGet(const std::string& path,
                                        const std::map<std::string, std::string>& params) {
    std::string rt = reqTime();
    std::map<std::string, std::string> p = params;
    removeEmptyParams(p);
    std::string paramStr = buildQueryString(p);
    std::string sig = futuresSign(apiKey_, apiSecret_, rt, paramStr);
    std::string url = FUTURES_BASE_URL + path;
    if (!paramStr.empty()) url += "?" + paramStr;
    std::map<std::string, std::string> headers = {
        {"Content-Type", "application/json"},
        {"ApiKey", apiKey_},
        {"Request-Time", rt},
        {"Signature", sig}
    };
    HttpResponse res = httpRequest("GET", url, "", headers, opts_);
    if (res.body.empty()) return Json::Value();
    Json::Value j = jsonParse(res.body);
    if (j.isMember("data")) return j["data"];
    return j;
}

Json::Value FuturesTrade::signedPost(const std::string& path, const std::string& body) {
    std::string rt = reqTime();
    std::string sig = futuresSign(apiKey_, apiSecret_, rt, body);
    std::string url = FUTURES_BASE_URL + path;
    std::map<std::string, std::string> headers = {
        {"Content-Type", "application/json"},
        {"ApiKey", apiKey_},
        {"Request-Time", rt},
        {"Signature", sig}
    };
    HttpResponse res = httpRequest("POST", url, body, headers, opts_);
    if (res.body.empty()) return Json::Value();
    Json::Value j = jsonParse(res.body);
    if (j.isMember("data")) return j["data"];
    return j;
}

Json::Value FuturesTrade::accountAssets() {
    return signedGet("/api/v1/private/account/assets");
}

Json::Value FuturesTrade::accountAsset(const std::string& currency) {
    return signedGet("/api/v1/private/account/asset/" + currency);
}

Json::Value FuturesTrade::orderCreate(const std::map<std::string, std::string>& params) {
    Json::Value j(Json::objectValue);
    for (const auto& kv : params) j[kv.first] = kv.second;
    return signedPost("/api/v1/private/order/create", jsonToString(j));
}

Json::Value FuturesTrade::orderCreate(const std::string& symbol, int side, int orderType,
                                          const std::string& vol, int leverage,
                                          const std::map<std::string, std::string>& options) {
    Json::Value j(Json::objectValue);
    j["symbol"] = symbol;
    j["side"] = side;
    j["orderType"] = orderType;
    j["vol"] = vol;
    j["leverage"] = leverage;
    for (const auto& o : options) j[o.first] = o.second;
    return signedPost("/api/v1/private/order/create", jsonToString(j));
}

Json::Value FuturesTrade::orderCancel(const std::string& symbol, const std::string& orderId) {
    Json::Value j(Json::objectValue);
    j["symbol"] = symbol;
    j["orderId"] = orderId;
    return signedPost("/api/v1/private/order/cancel", jsonToString(j));
}

Json::Value FuturesTrade::orderCancelAll(const std::string& symbol) {
    Json::Value j(Json::objectValue);
    if (!symbol.empty()) j["symbol"] = symbol;
    return signedPost("/api/v1/private/order/cancel_all", jsonToString(j));
}

Json::Value FuturesTrade::openOrders(const std::string& symbol) {
    std::map<std::string, std::string> p;
    if (!symbol.empty()) p["symbol"] = symbol;
    return signedGet("/api/v1/private/order/list/open_orders", p);
}

Json::Value FuturesTrade::historyOrders(const std::string& symbol,
                                            const std::map<std::string, std::string>& options) {
    std::map<std::string, std::string> p = options;
    if (!symbol.empty()) p["symbol"] = symbol;
    return signedGet("/api/v1/private/order/list/history_orders", p);
}

Json::Value FuturesTrade::getOrder(const std::string& orderId) {
    return signedGet("/api/v1/private/order/get/" + orderId);
}

Json::Value FuturesTrade::openPositions(const std::string& symbol) {
    std::map<std::string, std::string> p;
    if (!symbol.empty()) p["symbol"] = symbol;
    return signedGet("/api/v1/private/position/open_positions", p);
}

} // namespace futures
} // namespace mexc
