# services/CreateOrdersService/CMakeLists.txt

project(CreateOrdersService)

find_package(Threads REQUIRED)

add_executable(CreateOrdersService
    src/CreateOrdersService.cpp
    main.cpp
)

target_include_directories(CreateOrdersService
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(CreateOrdersService
    PUBLIC
        dto
        platform_deps
        Threads::Threads
)
