# Copyright (C) 2016 Jan Dalheimer <jan@dalheimer.de>
# This work is free. You can redistribute it and/or modify it under the
# terms of the Do What The Fuck You Want To Public License, Version 2,
# as published by Sam Hocevar. See http://www.wtfpl.net/ for more details.

find_program(RALPH_EXECUTABLE ralph DOC "Location of the ralph executable")
if(NOT RALPH_EXECUTABLE)
	message(FATAL_ERROR "Unable to find ralph. Make sure it is installed.")
endif()
execute_process(COMMAND ${RALPH_EXECUTABLE} integration cmake cmake_path OUTPUT_VARIABLE RALPH_CMAKE_PATH)
mark_as_advanced(RALPH_CMAKE_PATH)
include(${RALPH_CMAKE_PATH}/RalphHelpers.cmake)
include(${RALPH_CMAKE_PATH}/RalphFunctions.cmake)
