cmake_minimum_required(VERSION 3.12)

project(myleptjson LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(myleptjson STATIC
    src/myleptjson.c
)

target_include_directories(myleptjson PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(myleptjson_test
    src/test.cpp
)

target_link_libraries(myleptjson_test PRIVATE myleptjson)
