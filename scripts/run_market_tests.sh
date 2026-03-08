#!/bin/bash
# Run all market API tests and then parse outputs.
# Build first: cd build && cmake .. && make
# Run from repo root: ./scripts/run_market_tests.sh

set -e
BINDIR="${BINDIR:-build}"
OUT="${MEXC_OUTPUT_DIR:-output}"
export MEXC_OUTPUT_DIR="$OUT"

echo "=== 1. REST: Spot ==="
"$BINDIR/test_spot_market_rest" || true
echo "=== 2. REST: Futures ==="
"$BINDIR/test_futures_market_rest" || true

if [ -x "$BINDIR/test_spot_market_ws" ]; then
  echo "=== 3. WebSocket Spot (each 60s) ==="
  for ch in "spot@public.aggre.deals.v3.api.pb@100ms@BTCUSDT:spot_deals" \
            "spot@public.kline.v3.api.pb@BTCUSDT@Min1:spot_kline" \
            "spot@public.aggre.depth.v3.api.pb@100ms@BTCUSDT:spot_depth" \
            "spot@public.limit.depth.v3.api.pb@BTCUSDT@5:spot_depth_limit" \
            "spot@public.aggre.bookTicker.v3.api.pb@100ms@BTCUSDT:spot_bookTicker"; do
    channel="${ch%%:*}"
    slug="${ch##*:}"
    echo "  $slug ..."
    "$BINDIR/test_spot_market_ws" "$channel" "$slug" 60 || true
  done
  echo "=== 4. WebSocket Futures (each 60s) ==="
  for ct in "ticker:ticker" "tickers:tickers" "deal:deal" "depth:depth" "kline:kline" "funding_rate:funding_rate"; do
    type="${ct%%:*}"
    slug="futures_${ct##*:}"
    echo "  $slug ..."
    "$BINDIR/test_futures_market_ws" "$type" "BTC_USDT" "$slug" 60 || true
  done
else
  echo "WebSocket tests skipped (build with Boost)"
fi

echo "=== 5. Parse REST output ==="
"$BINDIR/parse_rest_output" "$OUT/rest_spot/"*.json 2>/dev/null || true
"$BINDIR/parse_rest_output" "$OUT/rest_futures/"*.json 2>/dev/null || true
echo "=== 6. Parse WebSocket JSONL ==="
for f in "$OUT/ws_spot/"*.jsonl "$OUT/ws_futures/"*.jsonl; do
  [ -f "$f" ] && "$BINDIR/parse_ws_jsonl" "$f" || true
done
echo "Done."
