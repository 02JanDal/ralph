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

find_package(Qt5 REQUIRED COMPONENTS Core Network Test)
find_package(LibGit2 REQUIRED)
find_package(LibCURL REQUIRED)
find_package(KF5Archive REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(SRC
	package/Package.h
	package/Package.cpp
	package/PackageDependency.h
	package/PackageDependency.cpp
	package/PackageDatabase.h
	package/PackageDatabase.cpp
	package/PackageSource.h
	package/PackageSource.cpp
	package/PackageMirror.h
	package/PackageMirror.cpp
	package/PackageGroup.h
	package/PackageGroup.cpp
	package/PackageConfiguration.h
	package/PackageConfiguration.cpp

	package/steps/InstallationStep.h
	package/steps/InstallationStep.cpp
	package/steps/GitInstallationSteps.h
	package/steps/GitInstallationSteps.cpp
	package/steps/CMakeInstallationSteps.h
	package/steps/CMakeInstallationSteps.cpp

	project/Project.h
	project/Project.cpp
	project/ProjectGenerator.h
	project/ProjectGenerator.cpp
	project/ProjectLockFile.h
	project/ProjectLockFile.cpp

	Requirement.h
	Requirement.cpp
	Version.h
	Version.cpp
	ActionContext.h
	ActionContext.cpp

	task/Task.h
	task/Task.cpp
	task/Network.h
	task/Network.cpp
	task/Archive.h
	task/Archive.cpp
	task/Process.h
	task/Process.cpp

	future/Future.h
	future/Future.cpp
	future/FutureWatcher.h
	future/FutureWatcher.cpp
	future/FutureOperators.h
	future/AwaitTerminal.h
	future/Promise.h
	future/Promise.cpp
	future/FutureData_p.h
	future/WrappedException.h
	future/WrappedException.cpp

	git/GitRepo.h
	git/GitRepo.cpp
	git/GitException.h
	git/GitException.cpp

	resources.qrc
)

add_library(ralph_clientlib SHARED ${SRC})
target_include_directories(ralph_clientlib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include/ralph/clientlib>)
target_link_libraries(ralph_clientlib PUBLIC ralph_common Qt5::Core Qt5::Network KF5::Archive libgit2 libcurl)

set_source_files_properties(git/GitRepo.cpp PROPERTIES COMPILE_FLAGS -Wno-missing-field-initializers)

add_executable(tst_Future tests/Future_Test.cpp)
target_link_libraries(tst_Future PRIVATE ralph_clientlib Qt5::Test)
target_link_libraries(tst_Future PRIVATE pthread) # wat? why do I need this?
add_test(NAME tst_Future COMMAND tst_Future)

install(TARGETS ralph_clientlib DESTINATION lib EXPORT RalphLib COMPONENT Runtime)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include/ralph COMPONENT Development FILES_MATCHING PATTERN *.h)
