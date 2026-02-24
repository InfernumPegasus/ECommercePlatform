# tests/unit/parser/CMakeLists.txt

# ValueParser tests
add_executable(ValueParserTest ValueParserTest.cpp)
target_include_directories(ValueParserTest
        PRIVATE
        ${CMAKE_SOURCE_DIR}/common/parser/include
)
target_link_libraries(ValueParserTest
        PRIVATE
        gtest
        gtest_main
)

# TypedParams tests
add_executable(TypedParamsTest TypedParamsTest.cpp)
target_include_directories(TypedParamsTest
        PRIVATE
        ${CMAKE_SOURCE_DIR}/common/parser/include
)
target_link_libraries(TypedParamsTest
        PRIVATE
        gtest
        gtest_main
)

# PathParamType tests
add_executable(PathParamTypeTest PathParamTypeTest.cpp)
target_include_directories(PathParamTypeTest
        PRIVATE
        ${CMAKE_SOURCE_DIR}/common/parser/include
        ${CMAKE_SOURCE_DIR}/common/http/include
)
target_link_libraries(PathParamTypeTest
        PRIVATE
        gtest
        gtest_main
        http
)

include(GoogleTest)
gtest_discover_tests(ValueParserTest)
gtest_discover_tests(TypedParamsTest)
gtest_discover_tests(PathParamTypeTest)
