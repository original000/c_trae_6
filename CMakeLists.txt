cmake_minimum_required(VERSION 3.10)
project(ExpressionTemplates LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 创建可执行文件
add_executable(ExpressionTemplates main.cpp)

# 优化选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|Intel")
    target_compile_options(ExpressionTemplates PRIVATE -O3 -march=native -ffast-math)
endif()

# 包含头文件目录
target_include_directories(ExpressionTemplates PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
