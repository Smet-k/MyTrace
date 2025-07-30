cmake_minimum_required(VERSION 3.14.0)
project(myTraceroute LANGUAGES C)

add_compile_options(-Werror)
set(CMAKE_C_STANDARD 11)

file(GLOB myTraceroute_src "src/*.c")
add_executable(myTraceroute ${myTraceroute_src})
target_include_directories(myTraceroute PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)