# tests/integration/CMakeLists.txt

add_executable(RouteParsingIntegrationTest
        RouteParsingIntegrationTest.cpp
)

add_executable(HttpServerBehaviorIntegrationTest
        HttpServerBehaviorIntegrationTest.cpp
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

target_include_directories(RouteParsingIntegrationTest
        PRIVATE
        ${PROJECT_SOURCE_DIR}/common/parser/include
)

target_include_directories(HttpServerBehaviorIntegrationTest
        PRIVATE
        ${PROJECT_SOURCE_DIR}/common/http/include
)

include(GoogleTest)

gtest_discover_tests(RouteParsingIntegrationTest)
gtest_discover_tests(HttpServerBehaviorIntegrationTest)
