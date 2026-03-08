/**
 * Read and parse WebSocket JSONL output (from test_*_market_ws).
 * Each line: {"local_ts_ms": N, "channel": "...", "payload": "..."}
 * Usage: parse_ws_jsonl <file.jsonl> [file2.jsonl ...]
 */
#include "mexc/json_util.hpp"
#include <json/value.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static void processLine(int lineNo, const std::string& line, const std::string& path) {
    if (line.empty()) return;
    try {
        Json::Value j = mexc::jsonParse(line);
        if (j.isNull()) return;
        long long ts = j.isMember("local_ts_ms") ? static_cast<long long>(j["local_ts_ms"].asDouble()) : 0;
        std::string channel = j.isMember("channel") ? j["channel"].asString() : "";
        std::string payload = j.isMember("payload") ? j["payload"].asString() : "";
        if (lineNo <= 3 || lineNo % 100 == 0) {
            std::cout << "  [" << lineNo << "] local_ts_ms=" << ts << " channel=" << channel;
            if (payload.size() > 80) std::cout << " payload_len=" << payload.size() << " payload_preview=" << payload.substr(0, 80) << "...";
            else std::cout << " payload=" << payload;
            std::cout << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << path << ":" << lineNo << " parse error: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file.jsonl> [file2.jsonl ...]\n";
        return 1;
    }
    for (int i = 1; i < argc; ++i) {
        std::string path = argv[i];
        std::ifstream f(path);
        if (!f) {
            std::cerr << "Cannot read: " << path << "\n";
            continue;
        }
        std::cout << "--- " << path << " ---\n";
        std::string line;
        int lineNo = 0;
        while (std::getline(f, line)) {
            ++lineNo;
            processLine(lineNo, line, path);
        }
        std::cout << "  total lines: " << lineNo << "\n\n";
    }
    return 0;
}
