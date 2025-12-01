cmake_minimum_required(VERSION 3.10)

# 项目名称
project(PluginReloaderDemo CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# 包含目录
include_directories(${PROJECT_SOURCE_DIR})

# 主程序源文件
set(MAIN_SOURCES
    main.cpp
    PluginReloader.cpp
)

# 主程序头文件
set(MAIN_HEADERS
    IPlugin.h
    PluginReloader.h
)

# 创建主程序可执行文件
add_executable(PluginReloaderDemo ${MAIN_SOURCES} ${MAIN_HEADERS})

# 链接库
target_link_libraries(PluginReloaderDemo PRIVATE psapi)

# 插件编译配置
add_subdirectory(plugins)
