/**
 * Test one Futures Market WebSocket channel. Runs 60 seconds, writes JSONL with local timestamp.
 * Usage: test_futures_market_ws <channel_type> [symbol] [output_slug] [duration_sec]
 * channel_type: ticker | tickers | deal | depth | kline | funding_rate
 * Example: test_futures_market_ws ticker BTC_USDT futures_ticker 60
 * Output: output/ws_futures/<output_slug>.jsonl
 */
#ifdef MEXC_USE_WEBSOCKET
#include "mexc/futures/websocket.hpp"
#include "mexc/json_util.hpp"
#include <json/value.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <sys/stat.h>
#include <atomic>
#include <mutex>

static std::string outputBase() {
    const char* env = std::getenv("MEXC_OUTPUT_DIR");
    if (env && env[0]) return std::string(env) + "/ws_futures";
    return "output/ws_futures";
}
static void ensureDir(const std::string& path) {
    size_t p = 0;
    while ((p = path.find('/', p + 1)) != std::string::npos) {
        std::string d = path.substr(0, p);
        mkdir(d.c_str(), 0755);
    }
    mkdir(path.c_str(), 0755);
}
static long long ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <channel_type> [symbol] [output_slug] [duration_sec]\n"
                  << "  channel_type: ticker | tickers | deal | depth | kline | funding_rate\n";
        return 1;
    }
    std::string channelType = argv[1];
    std::string symbol = argc >= 3 ? argv[2] : "BTC_USDT";
    std::string slug = argc >= 4 ? argv[3] : channelType;
    int duration = (argc >= 5) ? std::atoi(argv[4]) : 60;
    if (duration <= 0) duration = 60;

    std::string outDir = outputBase();
    ensureDir(outDir);
    std::string outPath = outDir + "/" + slug + ".jsonl";
    std::ofstream outFile(outPath, std::ios::app);
    if (!outFile) {
        std::cerr << "Cannot open " << outPath << "\n";
        return 1;
    }
    std::atomic<bool> running{true};
    std::mutex fileMutex;
    std::string channelLabel = channelType + (channelType == "tickers" ? "" : "_" + symbol);

    mexc::futures::FuturesWebSocket ws;
    ws.setOnMessage([&](const std::string& msg) {
        long long ts = ms();
        Json::Value line(Json::objectValue);
        line["local_ts_ms"] = static_cast<double>(ts);
        line["channel"] = channelLabel;
        line["payload"] = msg;
        std::lock_guard<std::mutex> lk(fileMutex);
        outFile << mexc::jsonToString(line) << "\n";
        outFile.flush();
    });
    ws.setOnClose([&]() { running = false; });

    if (!ws.connect()) {
        std::cerr << "Connect failed\n";
        return 1;
    }
    bool ok = false;
    if (channelType == "ticker") ok = ws.subTicker(symbol);
    else if (channelType == "tickers") ok = ws.subTickers();
    else if (channelType == "deal") ok = ws.subDeal(symbol);
    else if (channelType == "depth") ok = ws.subDepth(symbol);
    else if (channelType == "kline") ok = ws.subKline(symbol, "Min1");
    else if (channelType == "funding_rate") ok = ws.subFundingRate(symbol);
    else {
        std::cerr << "Unknown channel_type: " << channelType << "\n";
        return 1;
    }
    if (!ok) {
        std::cerr << "Subscribe failed\n";
        return 1;
    }
    std::cout << "[ " << ms() << " ] Subscribed to " << channelLabel << ", logging for " << duration << "s to " << outPath << "\n";
    ws.runAsync();
    for (int i = 0; i < duration && running; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ws.ping();
    }
    ws.disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "[ " << ms() << " ] Done. Output: " << outPath << "\n";
    return 0;
}
#else
#include <iostream>
int main() {
    std::cout << "Build with Boost for WebSocket.\n";
    return 0;
}
#endif
