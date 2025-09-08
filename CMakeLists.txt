cmake_minimum_required(VERSION 3.20)
project(CXXServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(cxxserver src/main.cpp src/server.cpp src/handler.cpp)

target_include_directories(cxxserver PRIVATE include)

find_package(Threads REQUIRED)
target_link_libraries(cxxserver PRIVATE Threads::Threads)