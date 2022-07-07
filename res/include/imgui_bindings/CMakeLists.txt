add_library(imgui_bindings_INC INTERFACE 
  imgui_impl_glfw.h
  imgui_impl_opengl3.h
)

target_include_directories(imgui_bindings_INC INTERFACE ${INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})