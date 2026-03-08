#pragma once

#include <string>
#include <json/value.h>

namespace mexc {

// Parse JSON string to Json::Value. Returns null Value on parse error.
Json::Value jsonParse(const std::string& s);

// Serialize Json::Value to string. pretty: use indentation.
std::string jsonToString(const Json::Value& j, bool pretty = false);

} // namespace mexc
