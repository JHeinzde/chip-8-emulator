cmake_minimum_required(VERSION 3.20)
project(Chip8Emulator C)

set(CMAKE_C_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)
add_executable(chip8_simulator main.c chip8.c)
target_link_libraries(chip8_simulator raylib winmm gdi32 shell32)
target_include_directories(chip8_simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})