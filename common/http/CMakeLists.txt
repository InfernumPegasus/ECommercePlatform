# common/http/CMakeLists.txt

add_library(http STATIC)

target_sources(http
    PRIVATE
        src/HttpHelpers.cpp
        src/Listener.cpp
        src/Router.cpp
        src/Session.cpp
)

target_include_directories(http
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(http
    PUBLIC
        platform_deps
)
