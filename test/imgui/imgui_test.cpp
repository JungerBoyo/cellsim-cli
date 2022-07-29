#include <catch2/catch.hpp>

#include <string_view>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <project_config/config.hpp>

TEST_CASE("Test Dear ImGui") {

  // NOLINTBEGIN
  if constexpr (cellsim::cmake::remote_build == "OFF") {
    // test glfw initialization
    REQUIRE(glfwInit() == GLFW_TRUE);
  
    // due to cognitive complexity warn
    // test window creation
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    constexpr int width{ 640 };
    constexpr int height{ 480 };
    constexpr std::string_view glsl_version = "#version 320 es";

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
  // NOLINTEND

}