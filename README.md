# MEXC C++ API

C++ 实现 MEXC 现货与合约 REST / WebSocket API，对应 [MEXC-API-SDK](https://github.com/mxcdevelop/mexc-api-sdk) 功能。

## 依赖

- C++17
- libcurl、OpenSSL（系统安装，如 `brew install curl openssl`）
- **JSON**：JsonCpp（由 Bazel 通过 Bzlmod 拉取）
- 可选：Boost (≥1.70)，用于 WebSocket 支持

## 构建（Bazel）

推荐使用 Bazel 构建（已使用 JsonCpp，依赖由 Bzlmod 管理）：

```bash
cd ~/mexc-cpp-api
bazel build //:mexc_cpp_api
bazel build //:example_spot_market   # 示例
bazel build //:test_spot_market_rest   # 测试
bazel build //:parse_rest_output   # 工具
```

一次性构建所有目标（不含 WebSocket）：

```bash
bazel build //:mexc_cpp_api //:example_spot_market //:example_spot_trade //:example_futures_market //:example_futures_trade //:test_spot_market_rest //:test_futures_market_rest //:parse_rest_output //:parse_ws_jsonl
```

启用 WebSocket 的示例（需系统安装 Boost，如 `brew install boost`）：

```bash
bazel build --define=MEXC_USE_WEBSOCKET=1 //:example_spot_ws //:example_futures_ws
```

运行示例（使用 Bazel 构建后的二进制在 `bazel-bin/` 下，或通过 `bazel run`）：

```bash
bazel run //:example_spot_market
bazel run //:example_spot_trade -- YOUR_API_KEY YOUR_API_SECRET
```

若未启用 Bzlmod，可在 `WORKSPACE` 中取消注释 `http_archive` 方式拉取 jsoncpp，并确保存在 `third_party/jsoncpp.BUILD`。

## 构建（CMake，可选）

项目仍保留 CMake 配置；若需用 CMake，需自行将依赖从 nlohmann/json 改为 JsonCpp 并链接 `jsoncpp_lib`。

## 模块概览

| 模块 | 说明 |
|------|------|
| **Spot Market (REST)** | 现货行情：exchangeInfo, depth, trades, klines, ticker 等 |
| **Spot Trade (REST)** | 现货交易：下单、撤单、查询订单、账户等 |
| **Spot WebSocket** | 现货行情推送（需 Boost） |
| **Futures Market (REST)** | 合约行情：ping, depth, kline, ticker, funding 等 |
| **Futures Trade (REST)** | 合约交易：下单、撤单、持仓、资金等 |
| **Futures WebSocket** | 合约行情推送（需 Boost） |

## 使用示例

### 现货行情（无需 API Key）

```cpp
#include "mexc/spot/market.hpp"

mexc::spot::SpotMarket market;
auto depth = market.depth("BTCUSDT", 10);
auto ticker = market.tickerPrice("BTCUSDT");
auto klines = market.klines("BTCUSDT", "1m", {{"limit", "100"}});
```

### 现货交易（需 API Key）

```cpp
#include "mexc/spot/trade.hpp"

mexc::spot::SpotTrade trade(apiKey, apiSecret);
auto acc = trade.accountInfo();
auto order = trade.newOrder("BTCUSDT", "BUY", "LIMIT", {{"quantity", "0.001"}, {"price", "50000"}});
trade.cancelOrder("BTCUSDT", {{"orderId", "123"}});
```

### 合约行情

```cpp
#include "mexc/futures/market.hpp"

mexc::futures::FuturesMarket market;
auto depth = market.depth("BTC_USDT");
auto ticker = market.ticker("BTC_USDT");
auto funding = market.fundingRate("BTC_USDT");
```

### 合约交易

```cpp
#include "mexc/futures/trade.hpp"

mexc::futures::FuturesTrade trade(apiKey, apiSecret);
auto assets = trade.accountAssets();
trade.orderCreate("BTC_USDT", 1, 1, "10", 20, {{"openType", "1"}, {"price", "50000"}});  // 1=开多, 1=限价
auto orders = trade.openOrders("BTC_USDT");
```

### 现货 WebSocket（需 Boost）

现货行情推送为 **Protocol Buffers（二进制）**，控制消息（订阅确认、PING/PONG）为 **JSON 文本**。需同时设置 `setOnMessage`（收 JSON）和 `setOnBinary`（收 protobuf），并用 [websocket-proto](https://github.com/mexcdevelop/websocket-proto) 生成 C++ 反序列化代码。定期调用 `ping()` 可保活（服务端回 PONG）。

```cpp
#include "mexc/spot/websocket.hpp"

mexc::spot::SpotWebSocket ws;
ws.setOnMessage([](const std::string& msg) { /* JSON: PONG、订阅确认等 */ });
ws.setOnBinary([](const void* data, size_t size) { /* protobuf 行情，需 ParseFromArray */ });
ws.connect();
ws.subscribe({"spot@public.aggre.deals.v3.api.pb@100ms@BTCUSDT"});
ws.run();  // 阻塞；另可 runAsync() + 定时 ws.ping() 保活
```

### 合约 WebSocket

```cpp
#include "mexc/futures/websocket.hpp"

mexc::futures::FuturesWebSocket ws;
ws.setOnMessage([](const std::string& msg) { /* ... */ });
ws.connect();
ws.subTicker("BTC_USDT");
ws.run();
```

## 运行示例程序

```bash
# 现货行情（无需密钥）
./example_spot_market

# 现货交易（需传入 key/secret）
./example_spot_trade YOUR_API_KEY YOUR_API_SECRET

# 合约行情
./example_futures_market

# 合约交易
./example_futures_trade YOUR_API_KEY YOUR_API_SECRET

# WebSocket 示例（若已启用 Boost）
./example_spot_ws
./example_futures_ws
```

## API 端点

- 现货 REST: `https://api.mexc.com/api/v3`
- 现货 WebSocket: `wss://wbs-api.mexc.com/ws`
- 合约 REST: `https://api.mexc.com`（路径如 `/api/v1/contract/...`、`/api/v1/private/...`）
- 合约 WebSocket: `wss://contract.mexc.com/edge`

## License

ISC
