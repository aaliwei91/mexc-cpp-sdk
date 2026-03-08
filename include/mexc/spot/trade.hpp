#pragma once

#include "mexc/spot/market.hpp"
#include "mexc/base.hpp"
#include "mexc/signature.hpp"
#include <json/value.h>
#include <string>
#include <map>

namespace mexc {
namespace spot {

class SpotTrade : public SpotMarket {
public:
    SpotTrade(const std::string& apiKey, const std::string& apiSecret);

    // Common
    Json::Value ping();
    Json::Value time();

    // UserData
    Json::Value accountInfo();
    Json::Value accountTradeList(const std::string& symbol,
                                    const std::map<std::string, std::string>& options = {});

    // Trade
    Json::Value newOrderTest(const std::string& symbol,
                                const std::string& side,
                                const std::string& orderType,
                                const std::map<std::string, std::string>& options = {});
    Json::Value newOrder(const std::string& symbol,
                            const std::string& side,
                            const std::string& orderType,
                            const std::map<std::string, std::string>& options = {});
    Json::Value cancelOrder(const std::string& symbol,
                               const std::map<std::string, std::string>& options = {});
    Json::Value cancelOpenOrders(const std::string& symbol);
    Json::Value queryOrder(const std::string& symbol,
                              const std::map<std::string, std::string>& options = {});
    Json::Value openOrders(const std::string& symbol);
    Json::Value allOrders(const std::string& symbol,
                             const std::map<std::string, std::string>& options = {});

    // User Data Stream (for WebSocket trade/account updates)
    // POST /api/v3/userDataStream -> create listenKey (valid 60 min)
    Json::Value createUserDataStream();
    // PUT /api/v3/userDataStream -> extend listenKey validity 60 min (recommend every 30 min)
    Json::Value keepaliveUserDataStream(const std::string& listenKey);
    // DELETE /api/v3/userDataStream -> close stream
    Json::Value closeUserDataStream(const std::string& listenKey);
    // GET /api/v3/userDataStream -> list valid listenKeys
    Json::Value getUserDataStreamListenKeys();

private:
    Json::Value signedRequest(const std::string& method,
                                 const std::string& path,
                                 const std::map<std::string, std::string>& params = {},
                                 const std::string& body = "");
    static std::string toUpper(std::string s);
};

} // namespace spot
} // namespace mexc
