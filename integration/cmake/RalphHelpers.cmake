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

if(RALPHHELPERSCMAKE_INCLUDED)
	return()
endif()
set(RALPHHELPERSCMAKE_INCLUDED 1)

if(NOT RALPH_EXECUTABLE)
	find_program(RALPH_EXECUTABLE ralph DOC "Location of the ralph executable")
	if(NOT RALPH_EXECUTABLE)
		message(FATAL_ERROR "Unable to find ralph. Make sure it is installed.")
	endif()
endif()

add_executable(ralph IMPORTED)
set_target_properties(ralph PROPERTIES IMPORTED_LOCATION "${RALPH_EXECUTABLE}")

# options
set(RALPH_DO_INSTALL OFF CACHE BOOL "Installed packages as part of running CMake")
