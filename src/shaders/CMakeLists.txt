find_package(glad REQUIRED)
find_package(fmt REQUIRED)

add_library(shaders_IMPL STATIC shaders.cpp)
target_link_libraries(shaders_IMPL
  PUBLIC
    shaders_INC
)


target_link_system_libraries(shaders_IMPL
  PRIVATE
    glad::glad
    fmt::fmt
)
