# tests/integration/CMakeLists.txt

add_executable(RouteParsingIntegrationTest
        RouteParsingIntegrationTest.cpp
)

add_executable(HttpServerBehaviorIntegrationTest
        HttpServerBehaviorIntegrationTest.cpp
)

add_executable(HttpRoutingLoadIntegrationTest
        HttpRoutingLoadIntegrationTest.cpp
)

add_executable(HttpServerLoadIntegrationTest
        HttpServerLoadIntegrationTest.cpp
)

target_link_libraries(RouteParsingIntegrationTest
        PRIVATE
        GTest::gtest_main
        parser
)

target_link_libraries(HttpServerBehaviorIntegrationTest
        PRIVATE
        GTest::gtest_main
        http
)

target_link_libraries(HttpRoutingLoadIntegrationTest
        PRIVATE
        GTest::gtest_main
        http
)

target_link_libraries(HttpServerLoadIntegrationTest
        PRIVATE
        GTest::gtest_main
        http
)

target_include_directories(RouteParsingIntegrationTest
        PRIVATE
        ${PROJECT_SOURCE_DIR}/common/parser/include
)

target_include_directories(HttpServerBehaviorIntegrationTest
        PRIVATE
        ${PROJECT_SOURCE_DIR}/common/http/include
)

target_include_directories(HttpRoutingLoadIntegrationTest
        PRIVATE
        ${PROJECT_SOURCE_DIR}/common/http/include
)

target_include_directories(HttpServerLoadIntegrationTest
        PRIVATE
        ${PROJECT_SOURCE_DIR}/common/http/include
)

include(GoogleTest)

gtest_discover_tests(RouteParsingIntegrationTest)
gtest_discover_tests(HttpServerBehaviorIntegrationTest)
gtest_discover_tests(HttpRoutingLoadIntegrationTest)
gtest_discover_tests(HttpServerLoadIntegrationTest)
