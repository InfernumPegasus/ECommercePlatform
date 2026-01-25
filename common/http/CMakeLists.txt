# common/http/CMakeLists.txt

add_library(http OBJECT)

find_package(Boost COMPONENTS url beast asio container REQUIRED)

target_precompile_headers(http PRIVATE
        <boost/beast/core.hpp>
        <boost/beast/http.hpp>
        <boost/url.hpp>
        <nlohmann/json.hpp>
        <string>
        <vector>
)

target_sources(http
    PRIVATE
        src/HttpHelpers.cpp
        src/Listener.cpp
        src/RequestContext.cpp
        src/Router.cpp
        src/RouteTrie.cpp
        src/Session.cpp
)

target_include_directories(http
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(http
    PUBLIC
        platform_json
        Boost::url
        Boost::beast
        Boost::asio
        Boost::container
        parser
)
