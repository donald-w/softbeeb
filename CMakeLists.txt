cmake_minimum_required(VERSION 3.5)
project(softbeeb C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable debug symbols by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Add debug flags
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")

# Find SDL2
find_package(SDL2 REQUIRED)

include_directories(include ${SDL2_INCLUDE_DIRS})

set(SOURCE_FILES
        address.c
        decodes.c
        include/decode.h
        include/header.h
        include/mnemonic.h
        include/reg_name.h
        include/screen.h
        include/tc_alloc.h
        include/tc_bios.h
        include/tc_conio.h
        include/tc_dos.h
        include/tc_graphics.h
        io.h
        io_read.c
        io_write.c
        irq.c
        main.c
        misc.c
        monitor.c
        opcodes.c
        screen.c
        scroll.c
        sound.c
        tc_alloc.c
        # Use SDL2 implementation instead of empty stubs
        sdltest/sdl_shim.c
)

add_executable(softbeeb ${SOURCE_FILES})
target_link_libraries(softbeeb ${SDL2_LIBRARIES})

# Suppress linker warning about section alignment on macOS
if(APPLE)
    target_link_options(softbeeb PRIVATE "-Wl,-w")
endif()