cmake_minimum_required(VERSION 3.2)
project(softbeeb)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

include_directories(include)

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
        tc_bios.c
        tc_conio.c
        tc_dos.c
        tc_graphics.c
)

add_executable(softbeeb ${SOURCE_FILES})