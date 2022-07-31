find_package(glad REQUIRED)

add_library(renderer_IMPL STATIC renderer.cpp)
target_link_libraries(renderer_IMPL
  PUBLIC
    renderer_INC
  PRIVATE
    shaders_IMPL
    cellmap_IMPL
)

target_link_system_libraries(renderer_IMPL
  PRIVATE
    glad::glad
)
