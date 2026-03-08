#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace mexc {
namespace futures {

// Futures WebSocket: wss://contract.mexc.com/edge
// Subscribe: {"method":"sub.ticker","param":{"symbol":"BTC_USDT"}}
// Ping: {"method":"ping"}

class FuturesWebSocket {
public:
    using MessageCallback = std::function<void(const std::string& message)>;

    explicit FuturesWebSocket(const std::string& apiKey = "", const std::string& apiSecret = "");
    ~FuturesWebSocket();

    bool connect();
    void disconnect();
    bool isConnected() const;

    // Public channels (no login)
    bool subTicker(const std::string& symbol);      // sub.ticker
    bool unsubTicker(const std::string& symbol);
    bool subTickers();                              // all tickers
    bool unsubTickers();
    bool subDeal(const std::string& symbol);       // sub.deal
    bool unsubDeal(const std::string& symbol);
    bool subDepth(const std::string& symbol);
    bool unsubDepth(const std::string& symbol);
    bool subKline(const std::string& symbol, const std::string& interval);
    bool unsubKline(const std::string& symbol);
    bool subFundingRate(const std::string& symbol);
    bool unsubFundingRate(const std::string& symbol);

    void ping();
    void setOnMessage(MessageCallback cb) { onMessage_ = std::move(cb); }
    void setOnClose(std::function<void()> cb) { onClose_ = std::move(cb); }
    void run();
    void runAsync();

private:
    std::string apiKey_;
    std::string apiSecret_;
    MessageCallback onMessage_;
    std::function<void()> onClose_;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace futures
} // namespace mexc
