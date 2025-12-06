cmake_minimum_required(VERSION 3.10)
project(sdltest C)

set(CMAKE_C_STANDARD 99)

find_package(SDL2 REQUIRED)

include_directories(
        ${SDL2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

add_executable(sdltest
        main.c
        sdl_shim.c
)

target_link_libraries(sdltest ${SDL2_LIBRARIES})
