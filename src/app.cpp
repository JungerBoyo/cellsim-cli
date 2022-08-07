//
// Created by reg on 8/6/22.
//
#include "app.hpp"

#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include <imgui/imgui_utils.hpp>
#include <GLFW/glfw3.h>

/// TODO add clear color to cellmap

CSIM::App::App(Window& window)
	: window_(window) {
	renderer_.setColors({{0.f, 0.f, 0.f, 1.f}, {1.f, 1.f, 1.f, 1.f}});
	window_.setScrollCallback([this](float offset){
		constexpr float scale_value{1.10f};
		if(offset > .0f) {
			renderer_.updateView({0.f, 0.f}, scale_value);
		} else {
			renderer_.updateView({0.f, 0.f}, 1.f / scale_value);
		}
	});
	window_.setCursorPosCallback([this](float prev_x, float prev_y, float x, float y){
		if(prev_x > 0 && prev_y > 0 && window_.getButtonState(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			constexpr float mv_divisor{ .1f };
			auto offset_x = mv_divisor * (x - prev_x);
			auto offset_y = mv_divisor * (prev_y - y);
			renderer_.updateView({offset_x, offset_y}, 1.f);
		}
	});

#ifdef GL_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(
			[](GLenum, GLenum, GLuint id, GLenum severity, GLsizei,
				 const GLchar *message, const void *) {
				if (severity == GL_DEBUG_SEVERITY_HIGH) {
					spdlog::error("[glad] {}, id = {}", message, id);
				} else {
					spdlog::info("[glad] {}, id = {}", message, id);
				}
			}, nullptr);
#endif
	glClearColor(.0f, .0f, .0f, 1.f); // NOLINT
	ImGui::Init(window_.native());

	cell_map_.seed(0, 0); // NOLINT
	cli_emulator_.setCLI();
}

void CSIM::App::destroy() {
	ImGui::Uninit();
	renderer_.destroy();
	cell_map_.destroy();
	if(rule_) {
		rule_->destroy();
	}
	if(rule_config_) {
		rule_config_->destroy();
	}
}

void CSIM::App::run() {
	while(!window_.shouldClose()) {
		window_.pollEvents();

		glClear(GL_COLOR_BUFFER_BIT);

		auto frame_time = window_.getTime();
		renderer_.time_step_ = frame_time - last_frame_time_;
		last_frame_time_ = frame_time;

		const auto[width, height] = window_.getWindowSize();
		renderer_.draw({width, height}, cell_map_);

		if(rule_ && rule_config_)	{
			if(++frame_counter_; frame_counter_ == step_size_) {
				rule_->step(cell_map_);
				frame_counter_ = 0;
			}
		}

		ImGui::BeginFrameCustom();
		if (cli_emulator_.draw(window_.getKeyState(GLFW_KEY_ENTER) == GLFW_PRESS,
													 window_.getKeyState(GLFW_KEY_BACKSPACE) == GLFW_PRESS)) {
			/// TODO
		}
		ImGui::EndFrameCustom();

		window_.swapBuffers();
	}

}
