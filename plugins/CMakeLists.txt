cmake_minimum_required(VERSION 3.10)

# 示例插件源文件
set(EXAMPLE_PLUGIN_SOURCES
    ExamplePlugin.cpp
)

# 示例插件头文件
set(EXAMPLE_PLUGIN_HEADERS
    ExamplePlugin.h
)

# 创建示例插件动态链接库
add_library(ExamplePlugin SHARED ${EXAMPLE_PLUGIN_SOURCES} ${EXAMPLE_PLUGIN_HEADERS})

# 设置输出目录，确保插件DLL生成在plugins目录下
set_target_properties(ExamplePlugin PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# 包含目录
include_directories(${PROJECT_SOURCE_DIR})
