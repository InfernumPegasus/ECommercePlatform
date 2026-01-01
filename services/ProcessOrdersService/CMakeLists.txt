# services/ProcessOrdersService/CMakeLists.txt

project(ProcessOrdersService)

find_package(Threads REQUIRED)

add_executable(ProcessOrdersService
    src/ProcessOrdersService.cpp
    main.cpp
)

target_include_directories(ProcessOrdersService
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ProcessOrdersService
    PUBLIC
        dto
        platform_deps
        Threads::Threads
)
