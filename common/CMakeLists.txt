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

set(SRC
	Functional.h
	functional/Base.h
	functional/FunctionTraits.h
	functional/ContainerTraits.h
	functional/Map.h
	functional/Filter.h
	functional/Each.h
	functional/Tap.h
	functional/Collection.h
	functional/Functions.h
	functional/Eval.h

	Json.h
	Json.cpp
	FileSystem.h
	FileSystem.cpp
	Exception.h
	Exception.cpp
	BaseConfigObject.h
	BaseConfigObject.cpp
	Formatting.h

	CommandLineParser.h
	CommandLineParser_p.h
	CommandLineParser.cpp
	TermUtil.h
	TermUtil.cpp

	Optional.h
)

add_library(ralph_common STATIC ${SRC})
target_include_directories(ralph_common PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include/ralph/common>)
target_link_libraries(ralph_common PUBLIC Qt5::Core Qt5::Network)

add_executable(tst_Functional tests/Functional_Test.cpp)
target_link_libraries(tst_Functional ralph_common)
add_test(NAME tst_Functional COMMAND tst_Functional)

install(TARGETS ralph_common DESTINATION lib EXPORT RalphLib COMPONENT Development)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include/ralph COMPONENT Development FILES_MATCHING PATTERN *.h)
