#include "mexc/futures/market.hpp"
#include "mexc/json_util.hpp"
#include <iostream>

int main() {
    mexc::futures::FuturesMarket market;
    std::cout << "=== MEXC Futures Market REST ===\n";

    auto ping = market.ping();
    if (!ping.isNull()) std::cout << "Ping: " << mexc::jsonToString(ping) << "\n";

    auto depth = market.depth("BTC_USDT");
    if (!depth.isNull()) {
        std::string s = mexc::jsonToString(depth);
        std::cout << "Depth BTC_USDT (first 200 chars): " << s.substr(0, 200) << "...\n";
    }

    auto ticker = market.ticker("BTC_USDT");
    if (!ticker.isNull()) {
        std::string s = mexc::jsonToString(ticker);
        std::cout << "Ticker BTC_USDT: " << s.substr(0, 300) << "...\n";
    }

    auto funding = market.fundingRate("BTC_USDT");
    if (!funding.isNull()) std::cout << "FundingRate: " << mexc::jsonToString(funding) << "\n";

    return 0;
}
