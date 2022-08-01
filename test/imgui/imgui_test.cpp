#include <catch2/catch.hpp>

#include <string_view>

#include "shconfig.hpp"
#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

TEST_CASE("Test Dear ImGui") {
	// NOLINTBEGIN
	// test glfw initialization
	REQUIRE(glfwInit() == GLFW_TRUE);

	// due to cognitive complexity warn
	// test window creation
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, CSIM::shconfig::GLVERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, CSIM::shconfig::GLVERSION_MINOR);
	constexpr int width{640};
	constexpr int height{480};

	const auto glsl_version =
			fmt::format("#version {}{}0", CSIM::shconfig::GLVERSION_MAJOR,
									CSIM::shconfig::GLVERSION_MINOR);
	auto *window =
			glfwCreateWindow(width, height, "Test Window", nullptr, nullptr);
	REQUIRE(window != nullptr);

	glfwMakeContextCurrent(window);

	REQUIRE(gladLoadGLLoader(reinterpret_cast<GLADloadproc>(
							glfwGetProcAddress)) != 0); // NOLINT

	IMGUI_CHECKVERSION();
	REQUIRE(ImGui::CreateContext() != nullptr);

	REQUIRE(ImGui_ImplGlfw_InitForOpenGL(window, true));
	REQUIRE(ImGui_ImplOpenGL3_Init(glsl_version.data()));

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	// NOLINTEND
}