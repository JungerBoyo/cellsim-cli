find_package(glfw3 REQUIRED)
find_package(imgui REQUIRED)
find_package(glad REQUIRED)

add_library(imgui_bindings_IMPL STATIC 
  imgui_impl_glfw.cpp
  imgui_impl_opengl3.cpp
)

target_compile_definitions(imgui_bindings_IMPL
  PUBLIC 
    IMGUI_IMPL_OPENGL_LOADER_CUSTOM
)

target_link_system_libraries(imgui_bindings_IMPL
  PUBLIC 
    imgui_bindings_INC
  PRIVATE
    glad::glad
    glfw::glfw
    imgui::imgui
)