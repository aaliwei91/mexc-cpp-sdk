#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace mexc {
namespace spot {

// Spot User Data Stream (trade/account updates).
// Connect: wss://wbs-api.mexc.com/ws?listenKey={listenKey}
// Get listenKey via SpotTrade::createUserDataStream(); extend with keepaliveUserDataStream() every ~30 min.
// Push data is Protocol Buffers (binary); control (PONG etc.) is JSON text. Use setOnBinary/setOnMessage.
// Ping: {"method":"PING"} -> server responds with {"id":0,"code":0,"msg":"PONG"} (keeps connection alive).
class SpotTradeWebSocket {
public:
    using MessageCallback = std::function<void(const std::string& message)>;
    using BinaryCallback = std::function<void(const void* data, size_t size)>;

    explicit SpotTradeWebSocket();
    ~SpotTradeWebSocket();

    // Connect using an existing listenKey (create via SpotTrade::createUserDataStream()).
    bool connect(const std::string& listenKey);
    void disconnect();
    bool isConnected() const;

    // Subscribe to user data channels (after connect).
    // spot@private.orders.v3.api.pb - order updates
    // spot@private.account.v3.api.pb - account/balance updates
    // spot@private.deals.v3.api.pb - trade/deal updates
    bool subscribeOrders();    // spot@private.orders.v3.api.pb
    bool subscribeAccount();   // spot@private.account.v3.api.pb
    bool subscribeDeals();     // spot@private.deals.v3.api.pb
    bool subscribe(const std::vector<std::string>& channels);
    bool unsubscribe(const std::vector<std::string>& channels);
    void ping();

    void setOnMessage(MessageCallback cb) { onMessage_ = std::move(cb); }
    void setOnBinary(BinaryCallback cb) { onBinary_ = std::move(cb); }
    void setOnClose(std::function<void()> cb) { onClose_ = std::move(cb); }
    void run();
    void runAsync();

private:
    MessageCallback onMessage_;
    BinaryCallback onBinary_;
    std::function<void()> onClose_;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace spot
} // namespace mexc
