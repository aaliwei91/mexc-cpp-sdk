/**
 * Test all Spot Market REST APIs. Writes each response to output/rest_spot/<api>.json
 * Run from repo root or set env MEXC_OUTPUT_DIR for output directory.
 */
#include "mexc/spot/market.hpp"
#include "mexc/json_util.hpp"
#include <fstream>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <sys/stat.h>

static std::string outputDir() {
    const char* env = std::getenv("MEXC_OUTPUT_DIR");
    if (env && env[0]) return std::string(env);
    return "output/rest_spot";
}

static void ensureDir(const std::string& path) {
    size_t p = 0;
    while ((p = path.find('/', p + 1)) != std::string::npos) {
        std::string d = path.substr(0, p);
        mkdir(d.c_str(), 0755);
    }
    mkdir(path.c_str(), 0755);
}

static void writeJson(const std::string& file, const Json::Value& j) {
    std::ofstream f(file);
    if (f) f << mexc::jsonToString(j, true);
}

static long long ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

int main() {
    mexc::spot::SpotMarket market;
    std::string out = outputDir();
    ensureDir(out);
    const std::string symbol = "BTCUSDT";

    #define RUN(name, call) do { \
        std::cout << "[ " << ms() << " ] " << name << " ... "; \
        try { \
            auto j = call; \
            writeJson(out + "/" name ".json", j); \
            std::cout << "ok -> " << out << "/" name ".json\n"; \
        } catch (const std::exception& e) { \
            std::cout << "error: " << e.what() << "\n"; \
        } \
    } while(0)

    RUN("ping", market.ping());
    RUN("time", market.time());
    RUN("exchangeInfo", market.exchangeInfo({}));
    RUN("exchangeInfo_symbol", market.exchangeInfo({{"symbol", symbol}}));
    RUN("depth", market.depth(symbol, 10));
    RUN("trades", market.trades(symbol, 20));
    RUN("historicalTrades", market.historicalTrades(symbol, 20));
    RUN("aggTrades", market.aggTrades(symbol, {{"limit", "20"}}));
    RUN("klines", market.klines(symbol, "1m", {{"limit", "5"}}));
    RUN("avgPrice", market.avgPrice(symbol));
    RUN("ticker24hr", market.ticker24hr(symbol));
    RUN("ticker24hr_all", market.ticker24hr());
    RUN("tickerPrice", market.tickerPrice(symbol));
    RUN("bookTicker", market.bookTicker(symbol));

    std::cout << "Done. Output in " << out << "/\n";
    return 0;
}
