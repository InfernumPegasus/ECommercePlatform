# common/dto/CMakeLists.txt

add_library(dto STATIC)

target_sources(dto
    PRIVATE
        src/OrderCreatedEvent.cpp
)

target_include_directories(dto
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(dto
    PUBLIC
        platform_deps
)
