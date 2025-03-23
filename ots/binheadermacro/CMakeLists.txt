cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(binheadermacro)

include_directories(include)
add_library(common STATIC src/common.cpp)

add_executable(bin2hppmacro src/bin2hppmacro.cpp)
target_link_libraries(bin2hppmacro common)

add_executable(hppmacro2bin src/hppmacro2bin.cpp)
target_link_libraries(hppmacro2bin common)

add_executable(hppmacro2bin-diff src/hppmacro2bin-diff.cpp)
target_link_libraries(hppmacro2bin-diff common)
