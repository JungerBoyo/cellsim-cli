#include <string_view>
#include <stdexcept>

#include <spdlog/spdlog.h>

#include <window/window.hpp>
#include <app.hpp>

int main() {
	try {
		CSIM::Window::initWindowingSystem();

		constexpr std::string_view title = "cellsim";
		constexpr int width{640};
		constexpr int height{480};

#ifdef GLFW_DEBUG
		CSIM::Window window(width, height, title,
										    [](int, const char *message) { spdlog::error("[glfw] {}", message); });
#else
		CSIM::Window window(width, height, title, nullptr);
#endif

		window.loadGL();

		CSIM::App app(window);

		app.run();

		app.destroy();
		window.destroy();
		CSIM::Window::uninitWindowingSystem();
	} catch (const std::exception& ex) {
		spdlog::critical("[cellsim] {}", ex.what());
	}
}