# Market API 测试说明

## 1. REST 测试

每个 REST 接口会请求一次，并把响应写入 JSON 文件。

### Spot

```bash
./build/test_spot_market_rest
```

输出目录默认 `output/rest_spot/`，可通过环境变量覆盖：

```bash
MEXC_OUTPUT_DIR=/tmp/mexc ./build/test_spot_market_rest
```

生成文件示例：`ping.json`, `time.json`, `exchangeInfo.json`, `depth.json`, `trades.json`, `historicalTrades.json`, `aggTrades.json`, `klines.json`, `avgPrice.json`, `ticker24hr.json`, `ticker24hr_all.json`, `tickerPrice.json`, `bookTicker.json` 等。

### Futures

```bash
./build/test_futures_market_rest
```

输出目录默认 `output/rest_futures/`。生成文件：`ping.json`, `contractDetail.json`, `supportCurrencies.json`, `depth.json`, `depthCommits.json`, `indexPrice.json`, `fairPrice.json`, `fundingRate.json`, `kline.json`, `deals.json`, `ticker.json`, `riskReverse.json`, `riskReverseHistory.json`, `fundingRateHistory.json` 等。

---

## 2. WebSocket 测试

每个 WebSocket 测试默认运行 **60 秒**，收到的每条消息会加上**本地时间戳**并写入 JSONL，每行一条记录，包含元数据。

### 格式

每行一个 JSON 对象：

- `local_ts_ms`: 本地时间戳（毫秒）
- `channel`: 订阅的 channel 名称
- `payload`: 服务端下发的原始消息内容

### Spot

需要先编译时启用 Boost（WebSocket）。

```bash
# 用法: test_spot_market_ws <channel> [output_slug] [duration_sec]
./build/test_spot_market_ws "spot@public.aggre.deals.v3.api.pb@100ms@BTCUSDT" spot_deals 60
./build/test_spot_market_ws "spot@public.kline.v3.api.pb@BTCUSDT@Min1" spot_kline 60
./build/test_spot_market_ws "spot@public.aggre.depth.v3.api.pb@100ms@BTCUSDT" spot_depth 60
./build/test_spot_market_ws "spot@public.limit.depth.v3.api.pb@BTCUSDT@5" spot_depth_limit 60
./build/test_spot_market_ws "spot@public.aggre.bookTicker.v3.api.pb@100ms@BTCUSDT" spot_bookTicker 60
```

输出目录默认 `output/ws_spot/`，得到 `spot_deals.jsonl`, `spot_kline.jsonl` 等。

### Futures

```bash
# 用法: test_futures_market_ws <channel_type> [symbol] [output_slug] [duration_sec]
# channel_type: ticker | tickers | deal | depth | kline | funding_rate
./build/test_futures_market_ws ticker BTC_USDT futures_ticker 60
./build/test_futures_market_ws tickers "" futures_tickers 60
./build/test_futures_market_ws deal BTC_USDT futures_deal 60
./build/test_futures_market_ws depth BTC_USDT futures_depth 60
./build/test_futures_market_ws kline BTC_USDT futures_kline 60
./build/test_futures_market_ws funding_rate BTC_USDT futures_funding_rate 60
```

输出目录默认 `output/ws_futures/`。

---

## 3. 解析输出

### REST 输出（JSON）

```bash
./build/parse_rest_output output/rest_spot/ping.json output/rest_spot/depth.json
# 或解析整个目录（需 shell 展开）
./build/parse_rest_output output/rest_spot/*.json
./build/parse_rest_output output/rest_futures/*.json
```

程序会读取每个 JSON 文件，解析并打印摘要（类型、键、数组长度、部分字段等）。

### WebSocket 输出（JSONL）

```bash
./build/parse_ws_jsonl output/ws_spot/spot_deals.jsonl
./build/parse_ws_jsonl output/ws_futures/futures_ticker.jsonl
# 多个文件
./build/parse_ws_jsonl output/ws_spot/*.jsonl output/ws_futures/*.jsonl
```

程序会按行解析，打印前几条及按间隔采样的行的 `local_ts_ms`、`channel`、`payload` 预览，并输出总行数。

---

## 4. 一键跑全量测试

```bash
chmod +x scripts/run_market_tests.sh
./scripts/run_market_tests.sh
```

会依次执行：Spot REST、Futures REST、Spot WS（各 channel 60s）、Futures WS（各 channel 60s），最后对生成的所有 REST JSON 和 WS JSONL 调用解析程序。可设置 `BINDIR=build`、`MEXC_OUTPUT_DIR=output` 覆盖路径。
