INCLUDE(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Generic)

#set(CMAKE_C_COMPILER_WORKS 1)
#set(CMAKE_CXX_COMPILER_WORKS 1)

if(NOT LD_SCRIPT)
    message(FATAL_ERROR "Error Linkerscript not defined")
endif()

add_definitions(-Dstm32f412zg -Dstm32f412zx -Dstm32f4xx -DSTM32F412Zx)

# specify the cross compiler and tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
SET(CMAKE_OBJCOPY "arm-none-eabi-objcopy")
SET(CMAKE_OBJDUMP "arm-none-eabi-objdump")
SET(CMAKE_SIZE "arm-none-eabi-size")
SET(CMAKE_DEBUGGER "arm-none-eabi-gdb")
SET(CMAKE_CPPFILT "arm-none-eabi-c++filt")

enable_language(ASM)

## Set target specific flags
set(COMMON_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
    -fno-common -ffunction-sections -fdata-sections")

## Enable all warnings, disable optimalisation for debugging
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    set(DEBUG_FLAGS "-pedantic -Wall \
    -Wno-missing-braces -Wextra -Wno-missing-field-initializers -Wformat=2 \
    -Wswitch-default -Wswitch-enum -Wcast-align -Wpointer-arith \
    -Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes -Winline \
    -Wundef -Wnested-externs -Wcast-qual -Wshadow -Wunreachable-code \
    -Wlogical-op -Wfloat-equal -Wstrict-aliasing=2 -Wredundant-decls \
    -Wold-style-definition \
    -ggdb3 -O0 \
    -fno-omit-frame-pointer -ffloat-store -fstrict-aliasing -lm")
else()
    set(DEBUG_FLAGS "-Os -Wall")
endif()

## Set the C compile and link flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} ${DEBUG_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-gc-sections --specs=nano.specs --specs=nosys.specs -L${CMAKE_SOURCE_DIR} -T${LD_SCRIPT} \
    ${LD_LIBS}")

## Function to create a hex file from the compiled binary
function(create_hex_target TARGET)
    set(FILENAME "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}")
    add_custom_target(${TARGET}.hex DEPENDS ${TARGET} COMMAND ${CMAKE_OBJCOPY}
            -Oihex ${FILENAME} ${FILENAME}.hex)
    print_target_size(${TARGET}.hex)
endfunction()

## Function to create a hex file from the compiled binary
function(create_bin_target TARGET)
    set(FILENAME "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}")
    add_custom_target(${TARGET}.bin DEPENDS ${TARGET} COMMAND ${CMAKE_OBJCOPY}
            -Obinary ${FILENAME} ${FILENAME}.bin)
endfunction()


## Function to create a dump file from the compiled binary with disassembly and
## symbols
function(dump_target TARGET)
    set(FILENAME "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}")
    add_custom_target(${TARGET}.dump DEPENDS ${TARGET}.hex ${TARGET}.bin COMMAND ${CMAKE_OBJDUMP}
            -x -D -S -s ${FILENAME} | ${CMAKE_CPPFILT} > ${FILENAME}.dump)
endfunction()

## Function to print the size of the binary
function(print_target_size TARGET)
    set(FILENAME "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}")
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_SIZE} ${FILENAME})
endfunction()