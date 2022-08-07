//
// Created by reg on 8/5/22.
//
#include "window/window.hpp"
#include <shconfig.hpp>

#include <stdexcept>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void CSIM::Window::loadGL() {
	if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) { // NOLINT
		throw std::runtime_error("glad loader failed");
	}
}

void CSIM::Window::initWindowingSystem() {
	if (glfwInit() != GLFW_TRUE) {
		throw std::runtime_error("glfw init failed");
	}
}

void CSIM::Window::uninitWindowingSystem() {
	glfwTerminate();
}

struct CSIM::Window::UserData {
	std::function<void(float)> scroll_callback{ nullptr };
	std::function<void(float, float, float, float)> cursor_callback{ nullptr };
	float prev_cursor_x_pos{ 0.f };
	float prev_cursor_y_pos{ 0.f };
};

struct CSIM::Window::WinNative {
	GLFWwindow* value{ nullptr };
};

CSIM::Window::Window(std::int32_t width, std::int32_t height, std::string_view title,
										 void(*error_callback)(int, const char*))
	: win_handle_{ std::make_shared<WinNative>() }, user_data_(new UserData, [](UserData* ptr) { delete ptr; })  // NOLINT
	{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, CSIM::shconfig::GLVERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, CSIM::shconfig::GLVERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	win_handle_->value = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
	if (win_handle_->value == nullptr) {
		throw std::runtime_error("window creation failed");
	}
	glfwMakeContextCurrent(win_handle_->value);

	if (error_callback != nullptr) {
		glfwSetErrorCallback(error_callback);
	}

	glfwSetWindowUserPointer(win_handle_->value, user_data_.get());
}

void CSIM::Window::destroy() {
	glfwDestroyWindow(win_handle_->value);
}

void* CSIM::Window::native(){
	return win_handle_->value;
}

int CSIM::Window::getKeyState(int key_code) {
	return glfwGetKey(win_handle_->value, key_code);
}
int CSIM::Window::getButtonState(int button_code) {
	return glfwGetMouseButton(win_handle_->value, button_code);
}
float CSIM::Window::getTime() {
	return static_cast<float>(glfwGetTime());
}
std::tuple<int, int> CSIM::Window::getWindowSize() {
	int x{ 0 };
	int y{ 0 };
	glfwGetWindowSize(win_handle_->value, &x, &y);
	return {x, y};
}

bool CSIM::Window::shouldClose() {
	return glfwWindowShouldClose(win_handle_->value) != 0;
}

void CSIM::Window::pollEvents() {
	glfwPollEvents();
}

void CSIM::Window::swapBuffers() {
	glfwSwapBuffers(win_handle_->value);
}

void CSIM::Window::setScrollCallback(const std::function<void(float)>& callback) {
	user_data_->scroll_callback = callback;
	glfwSetScrollCallback(win_handle_->value, [](GLFWwindow* win, double, double y_offset) {
		auto* user_ptr = static_cast<UserData*>(glfwGetWindowUserPointer(win));
		user_ptr->scroll_callback(static_cast<float>(y_offset));
	});
}

void CSIM::Window::setCursorPosCallback(
		const std::function<void(float, float, float, float)>& callback) {
	user_data_->cursor_callback = callback;
	glfwSetCursorPosCallback(win_handle_->value, [](GLFWwindow* win, double x, double y) {
		auto* user_ptr = static_cast<UserData*>(glfwGetWindowUserPointer(win));
		user_ptr->cursor_callback(user_ptr->prev_cursor_x_pos, user_ptr->prev_cursor_y_pos,
															static_cast<float>(x), static_cast<float>(y));
		user_ptr->prev_cursor_x_pos = static_cast<float>(x);
		user_ptr->prev_cursor_y_pos = static_cast<float>(y);
	});
}
