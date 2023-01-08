find_package(imgui REQUIRED)
find_package(fmt REQUIRED)
find_package(glfw3 REQUIRED)

add_library(imgui_utils_IMPL imgui_utils.cpp)

target_link_libraries(imgui_utils_IMPL
  PUBLIC
    imgui_utils_INC
  PRIVATE
    imgui_bindings_IMPL
    SHCONFIG
)

target_link_system_libraries(imgui_utils_IMPL
  PRIVATE
    imgui::imgui
    glfw::glfw
    fmt::fmt
)