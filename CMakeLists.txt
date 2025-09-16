cmake_minimum_required(VERSION 3.20)
project(CXXServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# JSON library (nlohmann/json) via FetchContent
# include(FetchContent)
# FetchContent_Declare(
#   nlohmann_json
#   GIT_REPOSITORY https://github.com/nlohmann/json.git
#   GIT_TAG v3.11.3
# )
# FetchContent_MakeAvailable(nlohmann_json)

# Grab all files in the src directory except main.cpp
file(GLOB_RECURSE CXXSERVER_LIB_SOURCES CONFIGURE_DEPENDS "src/libcxxserver/*.cpp")

# Create a library from all the files in the src directory except main.cpp
add_library(cxxserver
    ${CXXSERVER_LIB_SOURCES}
)

# Expose JSON library to dependents of cxxserver
# target_link_libraries(cxxserver PUBLIC nlohmann_json::nlohmann_json)

# Add the main executable
add_executable(main src/main.cpp)

# Link the threads library to the executable
find_package(Threads REQUIRED)
target_link_libraries(main PRIVATE cxxserver Threads::Threads)