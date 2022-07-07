find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)
find_package(imgui REQUIRED)

add_library(imgui_TEST OBJECT imgui_test.cpp)

target_link_system_libraries(imgui_TEST
  PRIVATE 
    Catch2::Catch2
    imgui::imgui
    glfw::glfw
    glad::glad
    imgui_bindings_INC
    imgui_bindings_IMPL
)