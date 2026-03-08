#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace mexc {
namespace spot {

// Spot WebSocket base: wss://wbs-api.mexc.com/ws
// Control (JSON text): subscribe/unsubscribe ack, PING -> PONG (see api-docs PING/PONG Mechanism).
// Market data push: Protocol Buffers (binary). Use setOnBinary() and deserialize with MEXC proto:
//   https://github.com/mexcdevelop/websocket-proto
// Subscribe: {"method":"SUBSCRIPTION","params":["spot@public.aggre.deals.v3.api.pb@100ms@BTCUSDT"]}
// Ping: {"method":"PING"}  -> server responds with {"id":0,"code":0,"msg":"PONG"}

class SpotWebSocket {
public:
    using MessageCallback = std::function<void(const std::string& message)>;
    using BinaryCallback = std::function<void(const void* data, size_t size)>;

    explicit SpotWebSocket(const std::string& apiKey = "", const std::string& apiSecret = "");
    ~SpotWebSocket();

    bool connect();
    void disconnect();
    bool isConnected() const;

    // Subscribe to channel (e.g. spot@public.aggre.deals.v3.api.pb@100ms@BTCUSDT)
    bool subscribe(const std::vector<std::string>& channels);
    bool unsubscribe(const std::vector<std::string>& channels);
    void ping();

    void setOnMessage(MessageCallback cb) { onMessage_ = std::move(cb); }
    void setOnBinary(BinaryCallback cb) { onBinary_ = std::move(cb); }
    void setOnClose(std::function<void()> cb) { onClose_ = std::move(cb); }

    void run();  // blocking run loop (process messages)
    void runAsync(); // start async run in background thread

private:
    std::string apiKey_;
    std::string apiSecret_;
    MessageCallback onMessage_;
    BinaryCallback onBinary_;
    std::function<void()> onClose_;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace spot
} // namespace mexc
