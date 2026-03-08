#pragma once

#include "mexc/base.hpp"
#include "mexc/signature.hpp"
#include <json/value.h>
#include <string>
#include <map>

namespace mexc {
namespace futures {

// Futures private API: https://api.mexc.com/api/v1/private/...
// Auth: ApiKey, Request-Time, Signature (HMAC-SHA256(secret, accessKey + reqTime + bodyOrQuery))
class FuturesTrade {
public:
    FuturesTrade(const std::string& apiKey, const std::string& apiSecret);

    // Account
    Json::Value accountAssets();
    Json::Value accountAsset(const std::string& currency);

    // Order: POST /api/v1/private/order/create (body: JSON camelCase)
    Json::Value orderCreate(const std::map<std::string, std::string>& params);
    Json::Value orderCreate(const std::string& symbol, int side, int orderType,
                               const std::string& vol, int leverage,
                               const std::map<std::string, std::string>& options = {});

    // POST /api/v1/private/order/cancel
    Json::Value orderCancel(const std::string& symbol, const std::string& orderId);
    // POST /api/v1/private/order/cancel_all
    Json::Value orderCancelAll(const std::string& symbol = "");

    // GET /api/v1/private/order/list/open_orders
    Json::Value openOrders(const std::string& symbol = "");
    // GET /api/v1/private/order/list/history_orders
    Json::Value historyOrders(const std::string& symbol = "",
                                 const std::map<std::string, std::string>& options = {});
    // GET /api/v1/private/order/get/{order_id}
    Json::Value getOrder(const std::string& orderId);

    // Position
    Json::Value openPositions(const std::string& symbol = "");

    void setRequestOptions(const RequestOptions& opts) { opts_ = opts; }

private:
    std::string apiKey_;
    std::string apiSecret_;
    RequestOptions opts_;

    Json::Value signedGet(const std::string& path,
                             const std::map<std::string, std::string>& params = {});
    Json::Value signedPost(const std::string& path, const std::string& body);
};

} // namespace futures
} // namespace mexc
