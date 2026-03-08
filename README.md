# MEXC C++ API

C++ implementation of MEXC Spot and Futures REST / WebSocket API, matching [MEXC-API-SDK](https://github.com/mxcdevelop/mexc-api-sdk) functionality.

## Dependencies

- C++17
- libcurl, OpenSSL (install on your system, e.g. `brew install curl openssl`)
- **JSON**: JsonCpp (fetched by Bazel via Bzlmod)
- Optional: Boost (≥1.70) for WebSocket support

## Build (Bazel)

Bazel is the recommended build system (JsonCpp is used; dependencies are managed by Bzlmod):

```bash
cd ~/mexc-cpp-api
bazel build //:mexc_cpp_api
bazel build //:example_spot_market   # Example
bazel build //:test_spot_market_rest   # Test
bazel build //:parse_rest_output   # Tool
```

Build all targets at once (excluding WebSocket):

```bash
bazel build //:mexc_cpp_api //:example_spot_market //:example_spot_trade //:example_futures_market //:example_futures_trade //:test_spot_market_rest //:test_futures_market_rest //:parse_rest_output //:parse_ws_jsonl
```

Examples with WebSocket enabled (requires Boost installed, e.g. `brew install boost`):

```bash
bazel build --define=MEXC_USE_WEBSOCKET=1 //:example_spot_ws //:example_futures_ws
```

Run examples (binaries are under `bazel-bin/` after building, or use `bazel run`):

```bash
bazel run //:example_spot_market
bazel run //:example_spot_trade -- YOUR_API_KEY YOUR_API_SECRET
```

If Bzlmod is not enabled, you can uncomment the `http_archive` block in `WORKSPACE` to fetch jsoncpp, and ensure `third_party/jsoncpp.BUILD` exists.

## Build (CMake, optional)

The project also includes CMake support. If using CMake, you need to switch dependencies from nlohmann/json to JsonCpp and link `jsoncpp_lib` yourself.

## Module overview

| Module | Description |
|--------|-------------|
| **Spot Market (REST)** | Spot market data: exchangeInfo, depth, trades, klines, ticker, etc. |
| **Spot Trade (REST)** | Spot trading: place/cancel orders, query orders, account, etc. |
| **Spot WebSocket** | Spot market stream (requires Boost) |
| **Futures Market (REST)** | Futures market data: ping, depth, kline, ticker, funding, etc. |
| **Futures Trade (REST)** | Futures trading: place/cancel orders, positions, funding, etc. |
| **Futures WebSocket** | Futures market stream (requires Boost) |

## Usage examples

### Spot market (no API key required)

```cpp
#include "mexc/spot/market.hpp"

mexc::spot::SpotMarket market;
auto depth = market.depth("BTCUSDT", 10);
auto ticker = market.tickerPrice("BTCUSDT");
auto klines = market.klines("BTCUSDT", "1m", {{"limit", "100"}});
```

### Spot trading (API key required)

```cpp
#include "mexc/spot/trade.hpp"

mexc::spot::SpotTrade trade(apiKey, apiSecret);
auto acc = trade.accountInfo();
auto order = trade.newOrder("BTCUSDT", "BUY", "LIMIT", {{"quantity", "0.001"}, {"price", "50000"}});
trade.cancelOrder("BTCUSDT", {{"orderId", "123"}});
```

### Futures market

```cpp
#include "mexc/futures/market.hpp"

mexc::futures::FuturesMarket market;
auto depth = market.depth("BTC_USDT");
auto ticker = market.ticker("BTC_USDT");
auto funding = market.fundingRate("BTC_USDT");
```

### Futures trading

```cpp
#include "mexc/futures/trade.hpp"

mexc::futures::FuturesTrade trade(apiKey, apiSecret);
auto assets = trade.accountAssets();
trade.orderCreate("BTC_USDT", 1, 1, "10", 20, {{"openType", "1"}, {"price", "50000"}});  // 1=open long, 1=limit
auto orders = trade.openOrders("BTC_USDT");
```

### Spot WebSocket (requires Boost)

Spot market stream uses **Protocol Buffers (binary)**; control messages (subscribe ack, PING/PONG) are **JSON text**. You must set both `setOnMessage` (for JSON) and `setOnBinary` (for protobuf), and use [websocket-proto](https://github.com/mexcdevelop/websocket-proto) to generate C++ deserialization code. Call `ping()` periodically to keep the connection alive (server responds with PONG).

```cpp
#include "mexc/spot/websocket.hpp"

mexc::spot::SpotWebSocket ws;
ws.setOnMessage([](const std::string& msg) { /* JSON: PONG, subscribe ack, etc. */ });
ws.setOnBinary([](const void* data, size_t size) { /* protobuf market data, use ParseFromArray */ });
ws.connect();
ws.subscribe({"spot@public.aggre.deals.v3.api.pb@100ms@BTCUSDT"});
ws.run();  // blocking; or use runAsync() + periodic ws.ping() to keep alive
```

### Futures WebSocket

```cpp
#include "mexc/futures/websocket.hpp"

mexc::futures::FuturesWebSocket ws;
ws.setOnMessage([](const std::string& msg) { /* ... */ });
ws.connect();
ws.subTicker("BTC_USDT");
ws.run();
```

## Running the example programs

```bash
# Spot market (no keys)
./example_spot_market

# Spot trading (pass key/secret)
./example_spot_trade YOUR_API_KEY YOUR_API_SECRET

# Futures market
./example_futures_market

# Futures trading
./example_futures_trade YOUR_API_KEY YOUR_API_SECRET

# WebSocket examples (if Boost is enabled)
./example_spot_ws
./example_futures_ws
```

## API endpoints

- Spot REST: `https://api.mexc.com/api/v3`
- Spot WebSocket: `wss://wbs-api.mexc.com/ws`
- Futures REST: `https://api.mexc.com` (paths like `/api/v1/contract/...`, `/api/v1/private/...`)
- Futures WebSocket: `wss://contract.mexc.com/edge`

## License

ISC
