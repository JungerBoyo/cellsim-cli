find_package(glad REQUIRED)

add_library(cellmap_IMPL STATIC cellmap.cpp)
target_link_libraries(cellmap_IMPL
  PUBLIC
    cellmap_INC
  PRIVATE
    shaders_IMPL
)

target_include_directories(cellmap_IMPL
  PRIVATE
    "${CMAKE_BINARY_DIR}/shaders"
)

target_link_system_libraries(cellmap_IMPL
  PRIVATE
    glad::glad
)
