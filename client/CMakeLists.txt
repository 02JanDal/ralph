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

find_package(Qt5 REQUIRED COMPONENTS Core Network)

set(CMAKE_AUTOMOC ON)

set(SRC
	main.cpp

	Functions.h
	Functions.cpp
)

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

add_executable(ralph_client ${SRC})
target_include_directories(ralph_client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ralph_client PRIVATE ralph_common ralph_clientlib)
target_link_libraries(ralph_client PRIVATE pthread) # wat? why do I need this?
set_target_properties(ralph_client PROPERTIES OUTPUT_NAME ralph)
