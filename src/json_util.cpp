#include "mexc/json_util.hpp"
#include <json/reader.h>
#include <json/writer.h>
#include <sstream>

namespace mexc {

Json::Value jsonParse(const std::string& s) {
    if (s.empty()) return Json::Value();
    Json::CharReaderBuilder builder;
    std::istringstream iss(s);
    Json::Value root;
    std::string errs;
    if (!Json::parseFromStream(builder, iss, &root, &errs))
        return Json::Value();
    return root;
}

std::string jsonToString(const Json::Value& j, bool pretty) {
    if (j.isNull()) return "null";
    Json::StreamWriterBuilder wb;
    if (pretty)
        wb["indentation"] = "  ";
    return Json::writeString(wb, j);
}

} // namespace mexc
