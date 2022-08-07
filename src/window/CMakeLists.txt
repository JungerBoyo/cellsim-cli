find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)

add_library(window_IMPL_glfw3 window_glfw3.cpp)

target_link_libraries(window_IMPL_glfw3
  PUBLIC
    window_INC
)

target_link_system_libraries(window_IMPL_glfw3
  PRIVATE
    glad::glad
    glfw::glfw
)