cmake_minimum_required(VERSION 3.2)
project(console C)
include("${DEVKITPRO}/wut/share/wut.cmake" REQUIRED)

set(CMAKE_C_STANDARD 11)

add_executable(console
   main.c
   console.c
   console_colour.c
   console_font.c)

wut_create_rpx(console.rpx console)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/console.rpx"
        DESTINATION "${CMAKE_INSTALL_PREFIX}")
