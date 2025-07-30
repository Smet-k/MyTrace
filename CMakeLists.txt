cmake_minimum_required(VERSION 3.14.0)
project(myTrace LANGUAGES C)

add_compile_options(-Werror)
set(CMAKE_C_STANDARD 11)

file(GLOB myTrace_src "src/*.c")
add_executable(myTrace ${myTrace_src})
target_include_directories(myTrace PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)