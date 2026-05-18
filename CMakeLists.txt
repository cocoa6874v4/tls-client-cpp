cmake_minimum_required(VERSION 3.20)
project(tls-client-cpp LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(tls-client-cpp
    src/main.cpp
)

target_include_directories(tls-client-cpp PRIVATE
 ${CMAKE_CURRENT_SOURCE_DIR}/include
)
