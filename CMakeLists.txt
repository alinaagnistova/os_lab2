cmake_minimum_required(VERSION 3.26)
project(os_lab2)

set(CMAKE_CXX_STANDARD 17)

add_library(os_lab2 SHARED library.cpp)

add_executable(new-ema-search-str new-ema-search-str.cpp)
add_executable(ema-search-str ema-search-str.cpp)
add_executable(generate-file generate-file.cpp)
add_executable(test test.cpp)
target_link_libraries(test os_lab2)
target_link_libraries(new-ema-search-str os_lab2)