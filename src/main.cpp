#include <stdexcept>
#include <string_view>

#include <spdlog/spdlog.h>

#include <app.hpp>
#include <window/window.hpp>

int main() {
	try {
		CSIM::Window::initWindowingSystem();

		constexpr std::string_view title = "cellsim";
		constexpr int width{940};
		constexpr int height{680};

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
	} catch (const std::exception &ex) {
		spdlog::critical("[cellsim] {}", ex.what());
	}
}