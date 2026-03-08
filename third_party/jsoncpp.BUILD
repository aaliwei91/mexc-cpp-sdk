# Build file for jsoncpp when using http_archive (WORKSPACE) without Bzlmod.
# With Bzlmod (MODULE.bazel), the @jsoncpp module provides the target automatically.

cc_library(
    name = "jsoncpp",
    srcs = [
        "src/lib_json/json_reader.cpp",
        "src/lib_json/json_value.cpp",
        "src/lib_json/json_writer.cpp",
    ],
    hdrs = glob(["include/json/*.h"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
