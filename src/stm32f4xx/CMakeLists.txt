cmake_minimum_required(VERSION 3.8)

FILE(GLOB BSP_FILES bsp/src/*.c bsp/inc/*.h)
set(CPU_FILES cpu/src/startup_stm32f412zx.s cpu/src/system_stm32f4xx.c)
include_directories(bsp/inc)
include_directories(cpu/include)

add_library(bsp STATIC ${BSP_FILES} ${CPU_FILES})