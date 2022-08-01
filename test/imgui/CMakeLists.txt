find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)
find_package(imgui REQUIRED)
find_package(fmt REQUIRED)

add_library(imgui_TEST OBJECT imgui_test.cpp)

target_link_system_libraries(imgui_TEST
  PRIVATE 
    Catch2::Catch2
    imgui::imgui
    glfw::glfw
    glad::glad
    fmt::fmt
    imgui_bindings_INC
    imgui_bindings_IMPL
)

target_include_directories(imgui_TEST
  PRIVATE
    "${CMAKE_BINARY_DIR}/shaders"
)