#include <catch2/catch.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shconfig.hpp"

TEST_CASE("Test opengl context(GLAD + GLFW3)") {
  // test glfw initialization
  REQUIRE(glfwInit() == GLFW_TRUE);

  // test window creation
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, CSIM::shconfig::GLVERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, CSIM::shconfig::GLVERSION_MINOR);
  constexpr int width = 640;
  constexpr int height = 480;
  auto* window = glfwCreateWindow(width, height, "Test Window", nullptr, nullptr);
  REQUIRE(window != nullptr);

  glfwMakeContextCurrent(window);
  REQUIRE(gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) != 0);// NOLINT

  glfwDestroyWindow(window);
  glfwTerminate();
}