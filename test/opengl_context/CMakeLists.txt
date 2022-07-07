find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)

add_library(opengl_context_TEST OBJECT opengl_context_test.cpp)

target_link_system_libraries(opengl_context_TEST
  PRIVATE 
    Catch2::Catch2
    glfw::glfw
    glad::glad
)