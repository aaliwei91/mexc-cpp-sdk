# Third party

## nlohmann/json（可选，用于离线构建）

若无法使用 FetchContent 拉取或系统未安装 nlohmann-json，可手动放置单头文件：

1. 从 https://github.com/nlohmann/json/releases 下载发布包，或
2. 从 https://raw.githubusercontent.com/nlohmann/json/v3.11.3/single_include/nlohmann/json.hpp 保存为  
   `third_party/nlohmann/json.hpp`

存在该文件时，CMake 将优先使用本地路径，不再拉取或查找系统包。
