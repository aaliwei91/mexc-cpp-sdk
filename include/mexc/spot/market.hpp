#pragma once

#include "mexc/base.hpp"
#include <json/value.h>
#include <string>
#include <map>

namespace mexc {
namespace spot {

class SpotMarket {
public:
    SpotMarket(const std::string& apiKey = "", const std::string& apiSecret = "");

    // Exchange Information
    Json::Value exchangeInfo(const std::map<std::string, std::string>& options = {});

    // Order Book (depth)
    Json::Value depth(const std::string& symbol, int limit = 100);

    // Recent Trades
    Json::Value trades(const std::string& symbol, int limit = 500);

    // Historical Trades (requires API key for some exchanges; MEXC may allow without)
    Json::Value historicalTrades(const std::string& symbol, int limit = 500);

    // Aggregate Trades
    Json::Value aggTrades(const std::string& symbol,
                             const std::map<std::string, std::string>& options = {});

    // Klines/Candlestick
    Json::Value klines(const std::string& symbol,
                          const std::string& interval,
                          const std::map<std::string, std::string>& options = {});

    // Current Average Price
    Json::Value avgPrice(const std::string& symbol);

    // 24hr Ticker
    Json::Value ticker24hr(const std::string& symbol = "");

    // Symbol Price Ticker
    Json::Value tickerPrice(const std::string& symbol = "");

    // Book Ticker
    Json::Value bookTicker(const std::string& symbol = "");

    // Common (no auth)
    Json::Value ping();
    Json::Value time();

    void setRequestOptions(const RequestOptions& opts) { opts_ = opts; }

protected:
    std::string apiKey_;
    std::string apiSecret_;
    RequestOptions opts_;

private:

    Json::Value publicGet(const std::string& path,
                             const std::map<std::string, std::string>& params = {});
};

} // namespace spot
} // namespace mexc
