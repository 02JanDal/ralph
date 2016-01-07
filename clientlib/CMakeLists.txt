find_package(Qt5 REQUIRED COMPONENTS Core Network)
find_package(LibGit2 REQUIRED)
find_package(LibCURL REQUIRED)
find_package(KF5Archive REQUIRED)

set(CMAKE_AUTOMOC ON)

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

	git/GitRepo.h
	git/GitRepo.cpp
	git/GitException.h
	git/GitException.cpp
)

add_library(ralph_clientlib SHARED ${SRC})
target_include_directories(ralph_clientlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ralph_clientlib PRIVATE ralph_common)
target_link_libraries(ralph_clientlib PUBLIC Qt5::Core Qt5::Network KF5::Archive libgit2 libcurl)

set_source_files_properties(git/GitRepo.cpp PROPERTIES COMPILE_FLAGS -Wno-missing-field-initializers)
