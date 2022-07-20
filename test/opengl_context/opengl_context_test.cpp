#include <catch2/catch.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <config.hpp>

TEST_CASE("Test opengl context(GLAD + GLFW3)") {

  // test glfw initialization
  REQUIRE(glfwInit() == GLFW_TRUE);
  
  if constexpr (cellsim::cmake::remote_build == "OFF") {
    // test window creation
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    constexpr int width = 640;
    constexpr int height = 480;
    auto* window = glfwCreateWindow(width, height, "Test Window", nullptr, nullptr); 
    REQUIRE(window != nullptr);

    glfwMakeContextCurrent(window);

    glfwDestroyWindow(window);
  } else {
    REQUIRE(gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) != 0);// NOLINT 
  }

  glfwTerminate();
}