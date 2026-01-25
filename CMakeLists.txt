# CMakeLists.txt

cmake_minimum_required(VERSION 3.28)
project(ECommercePlatform)

# Global project options
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Packages installed via Vcpkg
find_package(PostgreSQL REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_package(RdKafka CONFIG REQUIRED)
find_package(unofficial-modern-cpp-kafka CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(platform_db INTERFACE)
target_link_libraries(platform_db INTERFACE
        PostgreSQL::PostgreSQL
        libpqxx::pqxx
)

add_library(platform_kafka INTERFACE)
target_link_libraries(platform_kafka INTERFACE
        RdKafka::rdkafka
        RdKafka::rdkafka++
        unofficial::modern-cpp-kafka::modern-cpp-kafka
)

add_library(platform_json INTERFACE)
target_link_libraries(platform_json INTERFACE
        nlohmann_json::nlohmann_json
)

# Creating core library that contains all dependencies from Vcpkg
add_library(platform_deps INTERFACE)

target_link_libraries(platform_deps INTERFACE
        platform_db
        platform_kafka
        platform_json
)

# Adding subfolders with libraries and executables
add_subdirectory(common)
add_subdirectory(services)

# Enabling and adding tests
enable_testing()
add_subdirectory(tests)