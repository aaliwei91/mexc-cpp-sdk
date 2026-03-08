// Spot User Data Stream: order/account/deal updates.
// Requires: api key + secret to create listenKey; then connect WS with that key.
// Build with Boost (WebSocket support).
#include "mexc/spot/trade.hpp"
#include "mexc/spot/trade_websocket.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
#ifdef MEXC_USE_WEBSOCKET
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <api_key> <api_secret>\n";
        return 1;
    }
    std::string apiKey = argv[1];
    std::string apiSecret = argv[2];

    mexc::spot::SpotTrade trade(apiKey, apiSecret);
    auto res = trade.createUserDataStream();
    if (res.is_null() || !res.contains("listenKey")) {
        std::cerr << "Failed to create listenKey: " << res.dump() << "\n";
        return 1;
    }
    std::string listenKey = res["listenKey"].get<std::string>();
    std::cout << "ListenKey: " << listenKey << "\n";

    mexc::spot::SpotTradeWebSocket ws;
    ws.setOnMessage([](const std::string& msg) { std::cout << "MSG: " << msg << "\n"; });
    ws.setOnClose([]() { std::cout << "Connection closed.\n"; });
    if (!ws.connect(listenKey)) {
        std::cerr << "WebSocket connect failed\n";
        return 1;
    }
    ws.subscribeOrders();
    ws.subscribeAccount();
    ws.subscribeDeals();
    std::cout << "Subscribed to orders, account, deals. Run for a while (Ctrl+C to stop)...\n";
    ws.run();
#else
    std::cout << "Build with Boost to enable WebSocket.\n";
    (void)argc;
    (void)argv;
#endif
    return 0;
}
