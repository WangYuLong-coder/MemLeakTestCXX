cmake_minimum_required(VERSION 3.4.1)

project(memtrace)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wl,--wrap=malloc,--wrap=free,--wrap=calloc,--wrap=realloc ")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=format")


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(${PROJECT_NAME} SHARED
    src/memtrace.cpp
)

target_link_libraries(${PROJECT_NAME} dl pthread )
