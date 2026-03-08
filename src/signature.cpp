#include "mexc/signature.hpp"
#include "mexc/base.hpp"

namespace mexc {

std::string spotSign(const std::string& secretKey, const std::string& queryString) {
    return hmacSha256Hex(secretKey, queryString);
}

std::string futuresSign(const std::string& accessKey,
                        const std::string& secretKey,
                        const std::string& reqTime,
                        const std::string& paramString) {
    std::string toSign = accessKey + reqTime + paramString;
    return hmacSha256Hex(secretKey, toSign);
}

} // namespace mexc
