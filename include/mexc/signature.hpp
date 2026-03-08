#pragma once

#include <string>
#include <map>

namespace mexc {

// Spot signature: sign queryString (params sorted, with timestamp)
// signature = HMAC-SHA256(secretKey, queryString)
std::string spotSign(const std::string& secretKey,
                     const std::string& queryString);

// Futures signature: sign (apiKey + reqTime + bodyOrQueryString)
// For GET/DELETE: bodyOrQueryString = sorted query params
// For POST: bodyOrQueryString = raw JSON body
std::string futuresSign(const std::string& accessKey,
                        const std::string& secretKey,
                        const std::string& reqTime,
                        const std::string& paramString);

} // namespace mexc
