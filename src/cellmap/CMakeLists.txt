find_package(glad REQUIRED)
find_package(lodepng REQUIRED)

add_library(cellmap_IMPL STATIC cellmap.cpp texture_backed_framebuffer.cpp)
target_link_libraries(cellmap_IMPL
  PUBLIC
    cellmap_INC
  PRIVATE
    shaders_IMPL
    SHCONFIG
)

target_link_system_libraries(cellmap_IMPL
  PRIVATE
    glad::glad
    lodepng::lodepng
)
