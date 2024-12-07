cmake_minimum_required(VERSION 3.26)
project(os_lab2)

set(CMAKE_CXX_STANDARD 17)

add_library(os_lab2 SHARED library.cpp)

add_executable(ema-search-str ema-search-str.cpp)
add_executable(generate-file generate-file.cpp)
