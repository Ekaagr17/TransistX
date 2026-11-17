cmake_minimum_required(VERSION 3.16)
project(TransitXC VERSION 2.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SOURCES "src/*.c")

add_executable(transitx_c ${SOURCES})

target_include_directories(transitx_c PRIVATE ${CMAKE_SOURCE_DIR}/include)

if(WIN32)
    target_link_libraries(transitx_c PRIVATE ws2_32 m)
else()
    target_link_libraries(transitx_c PRIVATE pthread m)
endif()

install(TARGETS transitx_c DESTINATION bin)
