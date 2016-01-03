find_package(Qt5 REQUIRED COMPONENTS Core Network)

#set(CMAKE_AUTOMOC ON)

set(SRC
    Functional.h
    functional/Base.h
    functional/FunctionTraits.h
    functional/ContainerTraits.h
    functional/Map.h
    functional/Filter.h
    functional/Collection.h
    functional/Functions.h

    Json.h
    Json.cpp
    FileSystem.h
    FileSystem.cpp
    Exception.h
    Exception.cpp
    BaseConfigObject.h
    BaseConfigObject.cpp

    Optional.h
)

add_library(ralph_common STATIC ${SRC})
target_include_directories(ralph_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ralph_common PUBLIC Qt5::Core Qt5::Network)

add_executable(tst_Functional tests/Functional_Test.cpp)
target_link_libraries(tst_Functional ralph_common)
add_test(tst_Functional tst_Functional)
