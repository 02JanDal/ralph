# Copyright 2016 Jan Dalheimer <jan@dalheimer.de>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

if(RALPHFUNCTIONSCMAKE_INCLUDED)
	return()
endif()
set(RALPHFUNCTIONSCMAKE_INCLUDED 1)

if(NOT RALPHHELPERSCMAKE_INCLUDED)
	message(FATAL_ERROR "You need to include RalphHelpers.cmake first")
endif()

macro(ralph_load)
	if(${ARGV0})
		set(dir "${ARGV0}")
	elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ralph.json")
		set(dir "${CMAKE_CURRENT_SOURCE_DIR}")
	elseif(EXISTS "${PROJECT_SOURCE_DIR}/ralph.json")
		set(dir "${PROJECT_SOURCE_DIR}")
	elseif(EXISTS "${CMAKE_SOURCE_DIR}/ralph.json")
		set(dir "${CMAKE_SOURCE_DIR}")
	endif()

	if(${dir}/ralph.json IS_NEWER_THAN ${dir}/.ralph.json.lock)
		if(CMAKE_BUILD_TYPE STREQUAL "")
			set(buildtype "Release")
		else()
			set(buildtype ${CMAKE_BUILD_TYPE})
		endif()

		if(RALPH_DO_INSTALL)
			execute_process(COMMAND ralph project install -c build.type:${buildtype} WORKING_DIRECTORY ${dir})
		else()
			message(FATAL_ERROR "ralph.json has been updated. Please run 'ralph project install -c build.type:${buildtype}'.")
		endif()

		unset(buildtype)
	endif()

	execute_process(COMMAND ralph integration cmake load -c build.type=${CMAKE_BUILD_TYPE} ${CMAKE_CURRENT_BINARY_DIR}/ralphcmake WORKING_DIRECTORY ${dir})
	include(${CMAKE_CURRENT_BINARY_DIR}/ralphcmake/ralph-packages.cmake)
	unset(dir)
endmacro()
