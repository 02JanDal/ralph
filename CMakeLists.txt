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

cmake_minimum_required(VERSION 3.1)

project(ralph VERSION 0.1)

include(FeatureSummary)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Weverything -Werror -Wno-documentation-unknown-command -Wno-undefined-reinterpret-cast -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-c++98-compat-bind-to-temporary-copy -Wno-exit-time-destructors -Wno-global-constructors -Wno-padded -Wno-missing-prototypes")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

enable_testing()

set(Ralph_DEFAULT_REPO "" CACHE STRING "Default repository URL")

option(WITH_INTEGRATION "Build with build system integration" ON)
add_feature_info(Integration WITH_INTEGRATION "Build with build system integrations")

add_subdirectory(common)
add_subdirectory(clientlib)
add_subdirectory(client)
if(WITH_INTEGRATION)
	add_subdirectory(integration)
endif()

set_package_properties(Qt5 PROPERTIES URL http://qt.io/ DESCRIPTION "a cross-platform application framework" TYPE REQUIRED)
set_package_properties(Qt5Core PROPERTIES URL http://qt.io/ DESCRIPTION "Core non-graphical classes used by other modules." TYPE REQUIRED)
set_package_properties(Qt5Network PROPERTIES URL http://qt.io/ DESCRIPTION "Classes to make network programming easier and more portable." TYPE REQUIRED)
set_package_properties(LibGit2 PROPERTIES URL https://libgit2.github.com/ DESCRIPTION "a portable, pure C implementation of the Git core methods" TYPE REQUIRED)
set_package_properties(LibCURL PROPERTIES URL http://curl.haxx.se/libcurl/ DESCRIPTION "a free and easy-to-use client-side URL transfer library" TYPE REQUIRED)
set_package_properties(KF5Archive PROPERTIES URL http://api.kde.org/frameworks-api/frameworks5-apidocs/karchive/html/ DESCRIPTION "provides classes for easy reading, creation and manipulation of archive formats like ZIP and TAR." TYPE REQUIRED)

feature_summary(FATAL_ON_MISSING_REQUIRED_PACKAGES WHAT ALL)
