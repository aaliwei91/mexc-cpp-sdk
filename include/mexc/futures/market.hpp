#pragma once

#include "mexc/base.hpp"
#include <json/value.h>
#include <string>
#include <map>

namespace mexc {
namespace futures {

// Futures market REST: https://api.mexc.com/api/v1/contract/...
class FuturesMarket {
public:
    FuturesMarket();  // no auth needed for market

    // GET api/v1/contract/ping
    Json::Value ping();
    // GET api/v1/contract/detail  optional symbol
    Json::Value contractDetail(const std::string& symbol = "");
    // GET api/v1/contract/support_currencies
    Json::Value supportCurrencies();
    // GET api/v1/contract/depth/{symbol}
    Json::Value depth(const std::string& symbol, int limit = 0);
    // GET api/v1/contract/depth_commits/{symbol}/{limit}
    Json::Value depthCommits(const std::string& symbol, int limit);
    // GET api/v1/contract/index_price/{symbol}
    Json::Value indexPrice(const std::string& symbol);
    // GET api/v1/contract/fair_price/{symbol}
    Json::Value fairPrice(const std::string& symbol);
    // GET api/v1/contract/funding_rate/{symbol}
    Json::Value fundingRate(const std::string& symbol);
    // GET api/v1/contract/kline/{symbol}  interval, start, end
    Json::Value kline(const std::string& symbol,
                         const std::map<std::string, std::string>& options = {});
    // GET api/v1/contract/kline/index_price/{symbol}
    Json::Value klineIndexPrice(const std::string& symbol,
                                   const std::map<std::string, std::string>& options = {});
    // GET api/v1/contract/kline/fair_price/{symbol}
    Json::Value klineFairPrice(const std::string& symbol,
                                  const std::map<std::string, std::string>& options = {});
    // GET api/v1/contract/deals/{symbol}
    Json::Value deals(const std::string& symbol, int limit = 100);
    // GET api/v1/contract/ticker  optional symbol
    Json::Value ticker(const std::string& symbol = "");
    // GET api/v1/contract/risk_reverse/{symbol}
    Json::Value riskReverse(const std::string& symbol);
    // GET api/v1/contract/risk_reverse/history
    Json::Value riskReverseHistory(const std::string& symbol, int pageNum = 1, int pageSize = 20);
    // GET api/v1/contract/funding_rate/history
    Json::Value fundingRateHistory(const std::string& symbol, int pageNum = 1, int pageSize = 20);

    void setRequestOptions(const RequestOptions& opts) { opts_ = opts; }

private:
    RequestOptions opts_;
    Json::Value publicGet(const std::string& path,
                             const std::map<std::string, std::string>& params = {});
};

} // namespace futures
} // namespace mexc
