/**
 * Test one Spot Market WebSocket channel. Runs 60 seconds, writes JSONL with local timestamp.
 * MEXC spot market push is Protocol Buffers (binary); control (PONG, subscribe ack) is JSON text.
 * Usage: test_spot_market_ws <channel> [output_slug] [duration_sec]
 * Example: test_spot_market_ws "spot@public.aggre.deals.v3.api.pb@100ms@BTCUSDT" spot_deals 60
 * Output: output/ws_spot/<output_slug>.jsonl
 *   Text: {"local_ts_ms", "channel", "type":"text", "payload": "..."}  (PONG, subscribe ack)
 *   Binary: {"local_ts_ms", "channel", "type":"binary", "payload_len": N}  (protobuf market data)
 */
#ifdef MEXC_USE_WEBSOCKET
#include "mexc/spot/websocket.hpp"
#include "mexc/json_util.hpp"
#include <json/value.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <sys/stat.h>
#include <atomic>

static std::string outputBase() {
    const char* env = std::getenv("MEXC_OUTPUT_DIR");
    if (env && env[0]) return std::string(env) + "/ws_spot";
    return "output/ws_spot";
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
        std::cerr << "Usage: " << argv[0] << " <channel> [output_slug] [duration_sec]\n"
                  << "  channel e.g. spot@public.aggre.deals.v3.api.pb@100ms@BTCUSDT\n";
        return 1;
    }
    std::string channel = argv[1];
    std::string slug = argc >= 3 ? argv[2] : "channel";
    int duration = (argc >= 4) ? std::atoi(argv[3]) : 60;
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

    mexc::spot::SpotWebSocket ws;
    ws.setOnMessage([&](const std::string& msg) {
        long long ts = ms();
        Json::Value line(Json::objectValue);
        line["local_ts_ms"] = static_cast<double>(ts);
        line["channel"] = channel;
        line["type"] = "text";
        line["payload"] = msg;
        std::lock_guard<std::mutex> lk(fileMutex);
        outFile << mexc::jsonToString(line) << "\n";
        outFile.flush();
    });
    ws.setOnBinary([&](const void* data, size_t size) {
        long long ts = ms();
        Json::Value line(Json::objectValue);
        line["local_ts_ms"] = static_cast<double>(ts);
        line["channel"] = channel;
        line["type"] = "binary";
        line["payload_len"] = static_cast<unsigned int>(size);
        std::lock_guard<std::mutex> lk(fileMutex);
        outFile << mexc::jsonToString(line) << "\n";
        outFile.flush();
    });
    ws.setOnClose([&]() { running = false; });

    if (!ws.connect()) {
        std::cerr << "Connect failed\n";
        return 1;
    }
    if (!ws.subscribe({channel})) {
        std::cerr << "Subscribe failed\n";
        return 1;
    }
    std::cout << "[ " << ms() << " ] Subscribed to " << channel << ", logging for " << duration << "s to " << outPath << "\n";
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
