#include "mexc/spot/market.hpp"
#include "mexc/json_util.hpp"
#include <iostream>

int main() {
    mexc::spot::SpotMarket market;  // no key needed for public market
    std::cout << "=== MEXC Spot Market REST ===\n";

    auto time = market.time();
    if (!time.isNull()) std::cout << "Time: " << mexc::jsonToString(time) << "\n";

    auto depth = market.depth("BTCUSDT", 10);
    if (!depth.isNull()) {
        std::string s = mexc::jsonToString(depth);
        std::cout << "Depth BTCUSDT (10): " << s.substr(0, 300) << "...\n";
    }

    auto ticker = market.tickerPrice("BTCUSDT");
    if (!ticker.isNull()) std::cout << "Ticker BTCUSDT: " << mexc::jsonToString(ticker) << "\n";

    auto klines = market.klines("BTCUSDT", "1m", {{"limit", "3"}});
    if (!klines.isNull() && klines.isArray()) std::cout << "Klines count: " << klines.size() << "\n";

    return 0;
}
