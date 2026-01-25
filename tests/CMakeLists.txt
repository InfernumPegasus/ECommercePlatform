# tests/CMakeLists.txt

include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# On Windows, explicitly set option to use msvcrt.dll
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_subdirectory(unit)
add_subdirectory(integration)