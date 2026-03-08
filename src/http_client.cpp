#include "mexc/base.hpp"
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

namespace mexc {

static size_t writeCallback(void* ptr, size_t size, size_t nmemb, std::string* out) {
    size_t total = size * nmemb;
    out->append(static_cast<char*>(ptr), total);
    return total;
}

static std::string urlEncode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    for (char c : value) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << std::uppercase << static_cast<int>(static_cast<unsigned char>(c));
        }
    }
    return escaped.str();
}

std::string buildQueryString(const std::map<std::string, std::string>& params) {
    std::ostringstream oss;
    bool first = true;
    for (const auto& p : params) {
        if (!first) oss << '&';
        oss << p.first << '=' << urlEncode(p.second);
        first = false;
    }
    return oss.str();
}

void removeEmptyParams(std::map<std::string, std::string>& params) {
    for (auto it = params.begin(); it != params.end(); ) {
        if (it->second.empty()) it = params.erase(it);
        else ++it;
    }
}

std::string hmacSha256Hex(const std::string& key, const std::string& data) {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    HMAC(EVP_sha256(), key.data(), static_cast<int>(key.size()),
         reinterpret_cast<const unsigned char*>(data.data()), data.size(), digest, &len);
    std::ostringstream oss;
    oss << std::hex;
    for (unsigned int i = 0; i < len; ++i)
        oss << std::setfill('0') << std::setw(2) << static_cast<int>(digest[i]);
    return oss.str();
}

HttpResponse httpRequest(const std::string& method,
                         const std::string& url,
                         const std::string& body,
                         const std::map<std::string, std::string>& extraHeaders,
                         const RequestOptions& opts) {
    HttpResponse res;
    CURL* curl = curl_easy_init();
    if (!curl) {
        res.error = "curl_easy_init failed";
        return res;
    }
    struct curl_slist* headers = nullptr;
    for (const auto& h : extraHeaders)
        headers = curl_slist_append(headers, (h.first + ": " + h.second).c_str());
    if (headers)
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, opts.timeout_sec);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res.body);
    if (opts.verbose)
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    if (method == "GET") {
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    } else if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
    } else if (method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }

    CURLcode code = curl_easy_perform(curl);
    if (code != CURLE_OK)
        res.error = curl_easy_strerror(code);
    else
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.status_code);

    if (headers) curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return res;
}

} // namespace mexc
