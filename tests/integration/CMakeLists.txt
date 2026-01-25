# tests/integration/CMakeLists.txt

add_executable(RouteParsingIntegrationTest
        RouteParsingIntegrationTest.cpp
)

target_link_libraries(RouteParsingIntegrationTest
        PRIVATE
        GTest::gtest_main
        parser
)

target_include_directories(RouteParsingIntegrationTest
        PRIVATE
        ${PROJECT_SOURCE_DIR}/common/parser/include
)

include(GoogleTest)

gtest_discover_tests(RouteParsingIntegrationTest)
