# common/parser/CMakeLists.txt

add_library(parser INTERFACE)

target_include_directories(parser
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)