cmake_minimum_required(VERSION 3.28)
project(ECommercePlatform)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PostgreSQL REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_package(RdKafka CONFIG REQUIRED)
find_package(unofficial-modern-cpp-kafka CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(platform_deps INTERFACE)

target_link_libraries(platform_deps
    INTERFACE
        PostgreSQL::PostgreSQL
        libpqxx::pqxx
        RdKafka::rdkafka
        RdKafka::rdkafka++
        unofficial::modern-cpp-kafka::modern-cpp-kafka
        nlohmann_json::nlohmann_json
)

add_subdirectory(common)
add_subdirectory(services)
