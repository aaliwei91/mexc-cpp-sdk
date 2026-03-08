#include "mexc/spot/websocket.hpp"

#if defined(MEXC_USE_WEBSOCKET) && MEXC_USE_WEBSOCKET
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/buffer.hpp>
#include "mexc/json_util.hpp"
#include <json/value.h>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#endif

namespace mexc {
namespace spot {

#if defined(MEXC_USE_WEBSOCKET) && MEXC_USE_WEBSOCKET
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

struct SpotWebSocket::Impl {
    std::string host = "wbs-api.mexc.com";
    std::string port = "443";
    std::string path = "/ws";
    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv12_client};
    beast::flat_buffer buffer;
    std::unique_ptr<websocket::stream<beast::ssl_stream<beast::tcp_stream>>> ws;
    std::atomic<bool> connected{false};
    std::atomic<bool> running{false};
    std::thread runThread;
    MessageCallback onMessage;
    BinaryCallback onBinary;
    std::function<void()> onClose;
    std::queue<std::string> writeQueue;
    std::mutex writeMutex;
};
#else
struct SpotWebSocket::Impl {};
#endif

SpotWebSocket::SpotWebSocket(const std::string& apiKey, const std::string& apiSecret)
    : apiKey_(apiKey), apiSecret_(apiSecret), impl_(std::make_unique<Impl>()) {
#if defined(MEXC_USE_WEBSOCKET) && MEXC_USE_WEBSOCKET
    impl_->onMessage = [this](const std::string& m) { if (onMessage_) onMessage_(m); };
    impl_->onBinary = [this](const void* d, size_t n) { if (onBinary_) onBinary_(d, n); };
    impl_->onClose = [this]() { if (onClose_) onClose_(); };
#endif
}

SpotWebSocket::~SpotWebSocket() {
    disconnect();
}

bool SpotWebSocket::connect() {
#if defined(MEXC_USE_WEBSOCKET) && MEXC_USE_WEBSOCKET
    try {
        impl_->ctx.set_default_verify_paths();
        impl_->ws = std::make_unique<websocket::stream<beast::ssl_stream<beast::tcp_stream>>>(impl_->ioc, impl_->ctx);
        tcp::resolver resolver(impl_->ioc);
        auto const results = resolver.resolve(impl_->host, impl_->port);
        get_lowest_layer(*impl_->ws).connect(results);
        SSL_set_tlsext_host_name(impl_->ws->next_layer().native_handle(), impl_->host.c_str());
        impl_->ws->next_layer().handshake(ssl::stream_base::client);
        impl_->ws->set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
        impl_->ws->handshake(impl_->host, impl_->path);
        impl_->connected = true;
        return true;
    } catch (const std::exception&) {
        return false;
    }
#else
    (void)apiKey_; (void)apiSecret_;
    return false;
#endif
}

void SpotWebSocket::disconnect() {
#if defined(MEXC_USE_WEBSOCKET) && MEXC_USE_WEBSOCKET
    impl_->running = false;
    impl_->connected = false;
    if (impl_->ws && impl_->ws->is_open()) {
        beast::error_code ec;
        impl_->ws->close(websocket::close_code::normal, ec);
    }
    if (impl_->runThread.joinable()) impl_->runThread.join();
#endif
}

bool SpotWebSocket::isConnected() const {
#if defined(MEXC_USE_WEBSOCKET) && MEXC_USE_WEBSOCKET
    return impl_->ws && impl_->ws->is_open();
#else
    return false;
#endif
}

bool SpotWebSocket::subscribe(const std::vector<std::string>& channels) {
#if defined(MEXC_USE_WEBSOCKET) && MEXC_USE_WEBSOCKET
    if (!impl_->ws || !impl_->ws->is_open()) return false;
    Json::Value j(Json::objectValue);
    j["method"] = "SUBSCRIPTION";
    j["params"] = Json::Value(Json::arrayValue);
    for (const auto& c : channels) j["params"].append(c);
    std::lock_guard<std::mutex> lk(impl_->writeMutex);
    impl_->writeQueue.push(jsonToString(j));
    return true;
#else
    (void)channels;
    return false;
#endif
}

bool SpotWebSocket::unsubscribe(const std::vector<std::string>& channels) {
#if defined(MEXC_USE_WEBSOCKET) && MEXC_USE_WEBSOCKET
    if (!impl_->ws || !impl_->ws->is_open()) return false;
    Json::Value j(Json::objectValue);
    j["method"] = "UNSUBSCRIPTION";
    j["params"] = Json::Value(Json::arrayValue);
    for (const auto& c : channels) j["params"].append(c);
    std::lock_guard<std::mutex> lk(impl_->writeMutex);
    impl_->writeQueue.push(jsonToString(j));
    return true;
#else
    (void)channels;
    return false;
#endif
}

void SpotWebSocket::ping() {
#if defined(MEXC_USE_WEBSOCKET) && MEXC_USE_WEBSOCKET
    if (!impl_->ws || !impl_->ws->is_open()) return;
    std::lock_guard<std::mutex> lk(impl_->writeMutex);
    impl_->writeQueue.push("{\"method\":\"PING\"}");
#endif
}

void SpotWebSocket::run() {
#if defined(MEXC_USE_WEBSOCKET) && MEXC_USE_WEBSOCKET
    if (!impl_->ws || !impl_->ws->is_open()) return;
    impl_->running = true;
    while (impl_->running && impl_->ws->is_open()) {
        {
            std::lock_guard<std::mutex> lk(impl_->writeMutex);
            while (!impl_->writeQueue.empty()) {
                auto msg = impl_->writeQueue.front();
                impl_->writeQueue.pop();
                impl_->ws->write(net::buffer(msg));
            }
        }
        impl_->buffer.consume(impl_->buffer.size());
        beast::error_code ec;
        impl_->ws->read(impl_->buffer, ec);
        if (ec) {
            if (ec == websocket::error::closed) impl_->onClose();
            break;
        }
        // MEXC: control (subscribe ack, PONG) = text/JSON; market data = binary protobuf
        if (impl_->ws->got_binary()) {
            auto seq = impl_->buffer.data();
            std::size_t n = boost::asio::buffer_size(seq);
            const char* p = static_cast<const char*>(boost::asio::buffer_cast<const void*>(*boost::asio::buffer_sequence_begin(seq)));
            impl_->onBinary(p, n);
        } else {
            impl_->onMessage(beast::buffers_to_string(impl_->buffer.data()));
        }
    }
    impl_->connected = false;
#endif
}

void SpotWebSocket::runAsync() {
#if defined(MEXC_USE_WEBSOCKET) && MEXC_USE_WEBSOCKET
    if (impl_->runThread.joinable()) return;
    impl_->runThread = std::thread([this]() { run(); });
#endif
}

} // namespace spot
} // namespace mexc
