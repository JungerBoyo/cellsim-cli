#include <catch2/catch.hpp>

#include <string_view>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

TEST_CASE("Test Dear ImGui") {

  // test glfw initialization
  REQUIRE(glfwInit() == GLFW_TRUE);
  
  // test window creation
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  constexpr int width{ 640 };
  constexpr int height{ 480 };
  constexpr std::string_view glsl_version = "#version 330";

  auto* window = glfwCreateWindow(width, height, "Test Window", nullptr, nullptr); 
  REQUIRE(window != nullptr);

  glfwMakeContextCurrent(window);

  REQUIRE(gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) != 0);// NOLINT 

  IMGUI_CHECKVERSION();
  REQUIRE(ImGui::CreateContext() != nullptr);

  REQUIRE(ImGui_ImplGlfw_InitForOpenGL(window, true));  
  REQUIRE(ImGui_ImplOpenGL3_Init(glsl_version.data()));

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
}