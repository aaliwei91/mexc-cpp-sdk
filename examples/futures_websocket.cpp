#include "mexc/futures/websocket.hpp"
#include <iostream>

int main() {
#ifdef MEXC_USE_WEBSOCKET
    mexc::futures::FuturesWebSocket ws;
    ws.setOnMessage([](const std::string& msg) { std::cout << "MSG: " << msg << "\n"; });
    if (!ws.connect()) {
        std::cerr << "Connect failed\n";
        return 1;
    }
    ws.subTicker("BTC_USDT");
    std::cout << "Subscribed to BTC_USDT ticker. Run a few reads (Ctrl+C to stop)...\n";
    ws.run();
#else
    std::cout << "Build with Boost to enable WebSocket\n";
#endif
    return 0;
}
