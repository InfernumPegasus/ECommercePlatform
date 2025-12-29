# services/OrderService/CMakeLists.txt

project(OrderService)

add_executable(OrderService
    src/OrderService.cpp
    main.cpp
)

target_include_directories(OrderService
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(OrderService
    PUBLIC
        dto
        platform_deps
)
