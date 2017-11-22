# JLink functions
# Adds targets for JLink programmers and emulators
# Copyright (c) 2016 Ryan Kurte
# This file is covered under the MIT license available at: https://opensource.org/licenses/MIT

# Configure flasher script for the project
set(BINARY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_PROJECT_NAME})
configure_file(${CMAKE_CURRENT_LIST_DIR}/flash.in ${CMAKE_CURRENT_BINARY_DIR}/flash.jlink)

if(NOT DEVICE)
	message(FATAL_ERROR "Error Linkerscript not defined")
endif()

add_custom_target(debug-server 
	COMMAND JLinkGDBServer -device ${DEVICE} -speed 4000 -if SWD)

add_custom_target(flash 
	COMMAND JLinkExe -device ${DEVICE} -speed 4000 -if SWD -CommanderScript ${CMAKE_CURRENT_LIST_DIR}/flash.in
		DEPENDS ${TARGET})

add_custom_target(erase 
	COMMAND JLinkExe -device ${DEVICE} -speed 4000 -if SWD -CommanderScript ${CMAKE_CURRENT_LIST_DIR}/erase.jlink)

add_custom_target(reset 
	COMMAND JLinkExe -device ${DEVICE} -speed 4000 -if SWD -CommanderScript ${CMAKE_CURRENT_LIST_DIR}/reset.jlink)