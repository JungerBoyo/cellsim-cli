#include <catch2/catch.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

TEST_CASE("Test opengl context(GLAD + GLFW3)") {

  // test glfw initialization
  REQUIRE(glfwInit() == GLFW_TRUE);
  
  // test window creation
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  constexpr int width = 640;
  constexpr int height = 480;
  auto* window = glfwCreateWindow(width, height, "Test Window", nullptr, nullptr); 
  REQUIRE(window != nullptr);

  glfwMakeContextCurrent(window);

  REQUIRE(gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) != 0);// NOLINT 

  glfwDestroyWindow(window);
  glfwTerminate();
}