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
set_target_properties(ralph_client PROPERTIES OUTPUT_NAME ralph)
