#include "mexc/futures/trade.hpp"
#include "mexc/json_util.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <api_key> <api_secret>\n";
        return 1;
    }
    std::string apiKey = argv[1];
    std::string apiSecret = argv[2];

    mexc::futures::FuturesTrade trade(apiKey, apiSecret);
    std::cout << "=== MEXC Futures Trade ===\n";

    auto assets = trade.accountAssets();
    if (!assets.isNull()) {
        std::string s = mexc::jsonToString(assets);
        std::cout << "Account assets: " << s.substr(0, 400) << "...\n";
    }

    auto open = trade.openOrders();
    if (!open.isNull()) std::cout << "Open orders: " << mexc::jsonToString(open) << "\n";

    auto positions = trade.openPositions();
    if (!positions.isNull()) std::cout << "Open positions: " << mexc::jsonToString(positions) << "\n";

    return 0;
}
