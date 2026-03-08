#pragma once

#include <string>
#include <map>
#include <memory>

namespace mexc {

// Spot API base URL
inline const std::string SPOT_BASE_URL = "https://api.mexc.com/api/v3";
// Futures API base URL
inline const std::string FUTURES_BASE_URL = "https://api.mexc.com";

struct RequestOptions {
    int timeout_sec = 30;
    bool verbose = false;
};

// Build query string from key-value params (sorted for signature)
std::string buildQueryString(const std::map<std::string, std::string>& params);

// Remove empty/null values from params (for spot: don't send empty)
void removeEmptyParams(std::map<std::string, std::string>& params);

// HMAC-SHA256 hex digest
std::string hmacSha256Hex(const std::string& key, const std::string& data);

// HTTP response
struct HttpResponse {
    long status_code = 0;
    std::string body;
    std::string error;
    bool ok() const { return status_code >= 200 && status_code < 300; }
};

// Sync HTTP request
HttpResponse httpRequest(const std::string& method,
                         const std::string& url,
                         const std::string& body,
                         const std::map<std::string, std::string>& extraHeaders,
                         const RequestOptions& opts = {});

} // namespace mexc
