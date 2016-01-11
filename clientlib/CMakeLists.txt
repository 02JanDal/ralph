find_package(Qt5 REQUIRED COMPONENTS Core Network Test)
find_package(LibGit2 REQUIRED)
find_package(LibCURL REQUIRED)
find_package(KF5Archive REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(SRC
	package/Package.h
	package/Package.cpp
	package/PackageDatabase.h
	package/PackageDatabase.cpp
	package/PackageSource.h
	package/PackageSource.cpp
	package/PackageInstallationCandidate.h
	package/PackageInstallationCandidate.cpp

	project/Project.h
	project/Project.cpp
	project/ProjectGenerator.h
	project/ProjectGenerator.cpp

	Requirement.h
	Requirement.cpp
	Version.h
	Version.cpp

	task/Task.h
	task/Task.cpp
	task/Network.h
	task/Network.cpp
	task/Archive.h
	task/Archive.cpp
	task/Future.h
	task/Future.cpp
	task/FutureWatcher.h
	task/FutureWatcher.cpp
	task/FutureOperators.h
	task/Promise.h
	task/Promise.cpp
	task/FutureData_p.h
	task/WrappedException.h
	task/WrappedException.cpp

	git/GitRepo.h
	git/GitRepo.cpp
	git/GitException.h
	git/GitException.cpp

	resources.qrc
)

add_library(ralph_clientlib SHARED ${SRC})
target_include_directories(ralph_clientlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ralph_clientlib PUBLIC ralph_common Qt5::Core Qt5::Network KF5::Archive libgit2 libcurl)

set_source_files_properties(git/GitRepo.cpp PROPERTIES COMPILE_FLAGS -Wno-missing-field-initializers)

add_executable(tst_Future tests/Future_Test.cpp)
target_link_libraries(tst_Future PRIVATE ralph_clientlib Qt5::Test)
add_test(NAME tst_Future COMMAND tst_Future)
