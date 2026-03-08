#include "mexc/spot/websocket.hpp"
#include <iostream>

int main() {
#ifdef MEXC_USE_WEBSOCKET
    mexc::spot::SpotWebSocket ws;
    ws.setOnMessage([](const std::string& msg) { std::cout << "MSG: " << msg << "\n"; });
    if (!ws.connect()) {
        std::cerr << "Connect failed\n";
        return 1;
    }
    ws.subscribe({"spot@public.aggre.deals.v3.api.pb@100ms@BTCUSDT"});
    std::cout << "Subscribed. Run a few reads (Ctrl+C to stop)...\n";
    ws.run();
#else
    std::cout << "Build with Boost to enable WebSocket (cmake -DMEXC_USE_WEBSOCKET=ON or install Boost)\n";
#endif
    return 0;
}
