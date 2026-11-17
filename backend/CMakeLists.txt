cmake_minimum_required(VERSION 3.16)
project(TransitX VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(PkgConfig)

if(PkgConfig_FOUND)
    pkg_check_modules(LIBPQXX libpqxx)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

# Collect all source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Main executable
add_executable(transitx ${SOURCES})

target_include_directories(transitx PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

target_link_libraries(transitx PRIVATE
    Threads::Threads
    pqxx
    pq
    ssl
    crypto
)

if(WIN32)
    target_link_libraries(transitx PRIVATE ws2_32 mswsock)
endif()

# Install
install(TARGETS transitx DESTINATION bin)
