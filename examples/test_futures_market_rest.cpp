/**
 * Test all Futures Market REST APIs. Writes each response to output/rest_futures/<api>.json
 */
#include "mexc/futures/market.hpp"
#include "mexc/json_util.hpp"
#include <fstream>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <sys/stat.h>

static std::string outputDir() {
    const char* env = std::getenv("MEXC_OUTPUT_DIR");
    if (env && env[0]) return std::string(env);
    return "output/rest_futures";
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
    mexc::futures::FuturesMarket market;
    std::string out = outputDir();
    ensureDir(out);
    const std::string symbol = "BTC_USDT";

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
    RUN("contractDetail", market.contractDetail());
    RUN("contractDetail_symbol", market.contractDetail(symbol));
    RUN("supportCurrencies", market.supportCurrencies());
    RUN("depth", market.depth(symbol));
    RUN("depthCommits", market.depthCommits(symbol, 5));
    RUN("indexPrice", market.indexPrice(symbol));
    RUN("fairPrice", market.fairPrice(symbol));
    RUN("fundingRate", market.fundingRate(symbol));
    RUN("kline", market.kline(symbol, {{"interval", "Min15"}, {"limit", "5"}}));
    RUN("klineIndexPrice", market.klineIndexPrice(symbol, {{"interval", "Min15"}}));
    RUN("klineFairPrice", market.klineFairPrice(symbol, {{"interval", "Min15"}}));
    RUN("deals", market.deals(symbol, 20));
    RUN("ticker", market.ticker(symbol));
    RUN("ticker_all", market.ticker());
    RUN("riskReverse", market.riskReverse(symbol));
    RUN("riskReverseHistory", market.riskReverseHistory(symbol, 1, 5));
    RUN("fundingRateHistory", market.fundingRateHistory(symbol, 1, 5));

    std::cout << "Done. Output in " << out << "/\n";
    return 0;
}
