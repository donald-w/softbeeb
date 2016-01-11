cmake_minimum_required(VERSION 3.2)
project(softbeeb)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(SOURCE_FILES
        address.c
        decode.h
        decodes.c
        header.h
        io.h
        io_read.c
        io_write.c
        irq.c
        main.c
        misc.c
        mnemonic.h
        monitor.c
        opcodes.c
        reg_name.h
        screen.c
        screen.h
        scroll.c
        sound.c
        tc_conio.h
        tc_conio.c
        tc_graphics.h
        tc_graphics.c
        tc_dos.h
        tc_dos.c
        tc_bios.h
        tc_bios.c
        tc_alloc.h
        tc_alloc.c)

add_executable(softbeeb ${SOURCE_FILES})