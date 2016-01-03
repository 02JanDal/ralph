find_package(Qt5 REQUIRED COMPONENTS Core Network)
find_package(LibGit2 REQUIRED)

set(CMAKE_AUTOMOC ON)

set(SRC
    Package.h
    Package.cpp
    PackageDatabase.h
    PackageDatabase.cpp
    PackageSource.h
    PackageSource.cpp

    VersionRequirement.h
    VersionRequirement.cpp

    Task.h
    Task.cpp

    git/GitRepo.h
    git/GitRepo.cpp
    git/GitException.h
    git/GitException.cpp
)

add_library(ralph_clientlib SHARED ${SRC})
target_include_directories(ralph_clientlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ralph_clientlib PRIVATE ralph_common)
target_link_libraries(ralph_clientlib PUBLIC Qt5::Core Qt5::Network libgit2)

set_source_files_properties(git/GitRepo.cpp PROPERTIES COMPILE_FLAGS -Wno-missing-field-initializers)
