cmake_minimum_required(VERSION 3.14)
project(mexc-cpp-api VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Dependencies: CURL, OpenSSL (for HMAC-SHA256)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# nlohmann/json (header-only): 1) local third_party 2) system 3) FetchContent
set(JSON_TP "${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann/json.hpp")
if(EXISTS "${JSON_TP}")
  add_library(nlohmann_json INTERFACE)
  target_include_directories(nlohmann_json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
  set(JSON_LIB nlohmann_json)
else()
  find_package(nlohmann_json 3.2 QUIET)
  if(nlohmann_json_FOUND)
    set(JSON_LIB nlohmann_json::nlohmann_json)
  else()
    include(FetchContent)
    FetchContent_Declare(nlohmann_json
      GIT_REPOSITORY https://github.com/nlohmann/json.git
      GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(nlohmann_json)
    set(JSON_LIB nlohmann_json::nlohmann_json)
  endif()
endif()
if(NOT DEFINED JSON_LIB)
  set(JSON_LIB nlohmann_json::nlohmann_json)
endif()

# Optional: Boost.Beast for WebSocket (spot & futures)
find_package(Boost 1.70 QUIET COMPONENTS system)
set(MEXC_USE_WEBSOCKET OFF)
if(Boost_FOUND)
  set(MEXC_USE_WEBSOCKET ON)
  add_compile_definitions(MEXC_USE_WEBSOCKET=1)
endif()

add_library(mexc_cpp_api
  src/http_client.cpp
  src/signature.cpp
  src/spot_market.cpp
  src/spot_trade.cpp
  src/spot_websocket.cpp
  src/spot_trade_websocket.cpp
  src/futures_market.cpp
  src/futures_trade.cpp
  src/futures_websocket.cpp
)
target_include_directories(mexc_cpp_api PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mexc_cpp_api PUBLIC
  CURL::libcurl
  OpenSSL::SSL
  OpenSSL::Crypto
  Threads::Threads
  ${JSON_LIB}
)
if(MEXC_USE_WEBSOCKET)
  target_link_libraries(mexc_cpp_api PUBLIC Boost::system)
  target_compile_definitions(mexc_cpp_api PUBLIC MEXC_USE_WEBSOCKET=1)
endif()

# Examples
add_executable(example_spot_market examples/spot_market_rest.cpp)
target_link_libraries(example_spot_market PRIVATE mexc_cpp_api)

add_executable(example_spot_trade examples/spot_trade.cpp)
target_link_libraries(example_spot_trade PRIVATE mexc_cpp_api)

add_executable(example_futures_market examples/futures_market.cpp)
target_link_libraries(example_futures_market PRIVATE mexc_cpp_api)

add_executable(example_futures_trade examples/futures_trade.cpp)
target_link_libraries(example_futures_trade PRIVATE mexc_cpp_api)

# Market API tests (REST: write each response to output/rest_spot|rest_futures/*.json)
add_executable(test_spot_market_rest examples/test_spot_market_rest.cpp)
target_link_libraries(test_spot_market_rest PRIVATE mexc_cpp_api)
add_executable(test_futures_market_rest examples/test_futures_market_rest.cpp)
target_link_libraries(test_futures_market_rest PRIVATE mexc_cpp_api)

# Parsers for test output
add_executable(parse_rest_output tools/parse_rest_output.cpp)
target_link_libraries(parse_rest_output PRIVATE mexc_cpp_api)
add_executable(parse_ws_jsonl tools/parse_ws_jsonl.cpp)
target_link_libraries(parse_ws_jsonl PRIVATE mexc_cpp_api)

if(MEXC_USE_WEBSOCKET)
  add_executable(example_spot_ws examples/spot_websocket.cpp)
  target_link_libraries(example_spot_ws PRIVATE mexc_cpp_api)
  add_executable(example_spot_trade_ws examples/spot_trade_websocket.cpp)
  target_link_libraries(example_spot_trade_ws PRIVATE mexc_cpp_api)
  add_executable(example_futures_ws examples/futures_websocket.cpp)
  target_link_libraries(example_futures_ws PRIVATE mexc_cpp_api)
  # Market WS tests: run 1 min, write JSONL with local_ts_ms + channel + payload
  add_executable(test_spot_market_ws examples/test_spot_market_ws.cpp)
  target_link_libraries(test_spot_market_ws PRIVATE mexc_cpp_api)
  add_executable(test_futures_market_ws examples/test_futures_market_ws.cpp)
  target_link_libraries(test_futures_market_ws PRIVATE mexc_cpp_api)
endif()

install(TARGETS mexc_cpp_api
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
install(DIRECTORY include/mexc DESTINATION include)
