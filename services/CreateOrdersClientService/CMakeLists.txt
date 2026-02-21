# services/CreateOrdersClientService/CMakeLists.txt

project(CreateOrdersClientService)

find_package(Boost REQUIRED)

add_executable(CreateOrdersClientService
    src/OrdersClient.cpp
    main.cpp
)

target_include_directories(CreateOrdersClientService
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(CreateOrdersClientService
    PUBLIC
        platform_json
        Boost::boost
)
