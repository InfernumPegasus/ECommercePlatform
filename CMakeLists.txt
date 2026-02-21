# CMakeLists.txt

cmake_minimum_required(VERSION 3.28)
project(ECommercePlatform)

# Global project options
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows API target version (Windows 7)
if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0601)
endif()

# Packages installed via Conan
find_package(libpqxx REQUIRED)
find_package(RdKafka REQUIRED)
find_package(modern-cpp-kafka REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(platform_db INTERFACE)
target_link_libraries(platform_db INTERFACE
        libpqxx::pqxx
)

add_library(platform_kafka INTERFACE)
target_link_libraries(platform_kafka INTERFACE
        RdKafka::rdkafka
        RdKafka::rdkafka++
        modern-cpp-kafka::modern-cpp-kafka
)

add_library(platform_json INTERFACE)
target_link_libraries(platform_json INTERFACE
        nlohmann_json::nlohmann_json
)

# Creating core library that contains all dependencies from Conan
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
