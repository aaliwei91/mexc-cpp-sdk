/**
 * Read and parse REST API output JSON files (from test_*_market_rest).
 * Usage: parse_rest_output <file.json> [file2.json ...]
 *        parse_rest_output output/rest_spot/<files>.json
 *        parse_rest_output output/rest_futures/<files>.json
 */
#include "mexc/json_util.hpp"
#include <json/value.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) return "";
    std::ostringstream os;
    os << f.rdbuf();
    return os.str();
}

static void summarize(const std::string& path, const Json::Value& j) {
    std::cout << "--- " << path << " ---\n";
    if (j.isNull()) { std::cout << "(null)\n"; return; }
    if (j.isBool()) { std::cout << (j.asBool() ? "true" : "false") << "\n"; return; }
    if (j.isNumeric()) { std::cout << mexc::jsonToString(j) << "\n"; return; }
    if (j.isString()) { std::cout << j.asString() << "\n"; return; }
    if (j.isArray()) {
        std::cout << "array size=" << j.size();
        if (j.size() > 0 && j[0].isObject()) std::cout << " (objects)";
        std::cout << "\n";
        if (j.size() > 0 && j.size() <= 3) std::cout << mexc::jsonToString(j, true) << "\n";
        return;
    }
    if (j.isObject()) {
        std::cout << "object keys_count=" << j.size() << "\n";
        if (j.isMember("symbol")) std::cout << "  symbol: " << mexc::jsonToString(j["symbol"]) << "\n";
        if (j.isMember("listenKey")) std::cout << "  listenKey: " << mexc::jsonToString(j["listenKey"]) << "\n";
        if (j.isMember("serverTime")) std::cout << "  serverTime: " << mexc::jsonToString(j["serverTime"]) << "\n";
        if (j.isMember("success")) std::cout << "  success: " << mexc::jsonToString(j["success"]) << "\n";
        if (j.isMember("data") && j["data"].isArray()) std::cout << "  data.size: " << j["data"].size() << "\n";
        if (j.isMember("bids")) std::cout << "  bids: " << j["bids"].size() << " levels\n";
        if (j.isMember("asks")) std::cout << "  asks: " << j["asks"].size() << " levels\n";
    }
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file.json> [file2.json ...]\n";
        return 1;
    }
    for (int i = 1; i < argc; ++i) {
        std::string path = argv[i];
        std::string raw = readFile(path);
        if (raw.empty()) {
            std::cerr << "Cannot read: " << path << "\n";
            continue;
        }
        try {
            Json::Value j = mexc::jsonParse(raw);
            if (j.isNull()) { std::cerr << path << " parse error\n"; continue; }
            if (j.isMember("data") && !j["data"].isNull())
                summarize(path + " (data)", j["data"]);
            else
                summarize(path, j);
        } catch (const std::exception& e) {
            std::cerr << path << " parse error: " << e.what() << "\n";
        }
    }
    return 0;
}
