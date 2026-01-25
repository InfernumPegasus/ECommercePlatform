# tests/unit/routing/CMakeLists.txt

# RouteTrie tests
add_executable(RouteTrieTest RouteTrieTest.cpp)
target_include_directories(RouteTrieTest
        PRIVATE
        ${CMAKE_SOURCE_DIR}/common/http/include
        ${CMAKE_SOURCE_DIR}/common/parser/include
)
target_link_libraries(RouteTrieTest
        PRIVATE
        gtest
        gtest_main
        http
        parser
)

# Router tests
add_executable(RouterTest RouterTest.cpp)
target_include_directories(RouterTest
        PRIVATE
        ${CMAKE_SOURCE_DIR}/common/http/include
        ${CMAKE_SOURCE_DIR}/common/parser/include
        ${CMAKE_SOURCE_DIR}/services/CreateOrdersService/include
)
target_link_libraries(RouterTest
        PRIVATE
        gtest
        gtest_main
        http
        parser
)

include(GoogleTest)
gtest_discover_tests(RouteTrieTest)
gtest_discover_tests(RouterTest)