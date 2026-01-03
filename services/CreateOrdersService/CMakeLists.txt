# services/CreateOrdersService/CMakeLists.txt

project(CreateOrdersService)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS url)

add_executable(CreateOrdersService
    src/CreateOrdersService.cpp
    src/OrdersController.cpp
    main.cpp
)

target_include_directories(CreateOrdersService
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(CreateOrdersService
    PUBLIC
        dto
        http
        platform_deps
#        Threads::Threads
        Boost::url
)
