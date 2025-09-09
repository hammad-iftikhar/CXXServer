cmake_minimum_required(VERSION 3.20)
project(CXXServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Grab all files in the src directory except main.cpp
file(GLOB_RECURSE CXXSERVER_LIB_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
list(REMOVE_ITEM CXXSERVER_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# Create a library from all the files in the src directory except main.cpp
add_library(cxxserver_lib
    ${CXXSERVER_LIB_SOURCES}
)

# Add the include directory to the library
target_include_directories(cxxserver_lib PUBLIC include)

# Add the main executable
add_executable(cxxserver src/main.cpp)

# Link the threads library to the executable
find_package(Threads REQUIRED)
target_link_libraries(cxxserver PRIVATE cxxserver_lib Threads::Threads)