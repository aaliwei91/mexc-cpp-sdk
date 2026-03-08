#include "mexc/spot/trade.hpp"
#include "mexc/json_util.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <api_key> <api_secret>\n";
        return 1;
    }
    std::string apiKey = argv[1];
    std::string apiSecret = argv[2];

    mexc::spot::SpotTrade trade(apiKey, apiSecret);
    std::cout << "=== MEXC Spot Trade ===\n";

    auto t = trade.time();
    if (!t.isNull()) std::cout << "Time: " << mexc::jsonToString(t) << "\n";

    auto acc = trade.accountInfo();
    if (!acc.isNull() && acc.isMember("balances"))
        std::cout << "Account balances count: " << acc["balances"].size() << "\n";

    // Test order (does not place real order)
    auto testOrder = trade.newOrderTest("BTCUSDT", "BUY", "LIMIT", {{"quantity", "0.001"}, {"price", "50000"}});
    std::cout << "NewOrderTest result: " << mexc::jsonToString(testOrder) << "\n";

    return 0;
}
