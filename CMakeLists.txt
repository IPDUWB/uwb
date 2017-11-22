cmake_minimum_required(VERSION 3.8)
project(uwb)

set(CMAKE_C_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/")
set(LD_SCRIPT ${CMAKE_SOURCE_DIR}/f412zg.ld)
include(cmake/ToolChain.cmake)
set(DEVICE STM32F412ZG)

add_subdirectory(src/stm32f4xx/)

include_directories(include)
include_directories(include/rtt)

set(RTT_SOURCES
        src/rtt/SEGGER_RTT_Syscalls_GCC.c
        src/rtt/SEGGER_RTT.c)

set(SOURCE_FILES
        src/deca_device.c
        src/deca_params_init.c
        src/platform/stm32_port.c
        src/sys_timer.c
        src/platform/stm32_init.c)

file(GLOB_RECURSE HEADER_FILES include/*.h
        include/platform/*.h
        include/deca/*.h
        include/rtt/*.h)

include_directories(src/stm32f4xx/bsp/inc)
include_directories(src/stm32f4xx/cpu/include)

add_executable(uwb main.c ${SOURCE_FILES} ${RTT_SOURCES} ${HEADER_FILES})
target_link_libraries(uwb bsp)
create_bin_target(uwb)

add_executable(uwb_send main_send.c ${SOURCE_FILES} ${RTT_SOURCES} ${HEADER_FILES})
target_link_libraries(uwb_send bsp)
create_bin_target(uwb_send)

include(cmake/jlink.cmake)