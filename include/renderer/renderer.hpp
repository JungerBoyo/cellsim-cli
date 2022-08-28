//
// Created by reg on 7/29/22.
//

#ifndef CELLSIM_RENDERER_HPP
#define CELLSIM_RENDERER_HPP

#include "utils/vecs.hpp"
#include <cellmap/cellmap.hpp>
#include <shaders/shaders.hpp>
#include <shconfig.hpp>

#include <array>
#include <memory>
#include <vector>

namespace CSIM {

using namespace utils;

struct Renderer {
	std::uint32_t vbo_id_{0};
	std::uint32_t outline_ibo_od_{0};
	std::uint32_t vao_id_{0};
	std::uint32_t view_config_ubo_id_{0};
	std::uint32_t colors_ubo_id_{0};

	std::shared_ptr<Shader> render_shader_{nullptr};
	std::shared_ptr<Shader> grid_shader_{nullptr};
	float time_step_{0.f};

	struct ViewConfig {
		Vec2<float> offset{0.f, 0.f};
		float scale{1.f};
		float aspect_ratio{1.f};
		Vec4<float> outline_color{1.f, 1.f, 1.f, 1.f};
	};

	ViewConfig view_config_{};
	std::array<Vec4<float>, shconfig::MAX_COLORS> colors_;
	std::size_t color_count_{0};
	bool grid_on_{false};
	Vec4<float> clear_color_{0.f, 0.f, 0.f, 1.f};

	static constexpr std::array<float, 8> QUAD{{-.5f, -.5f, -.5f, .5f, .5f, -.5f, .5f, .5f}};
	static constexpr std::array<std::uint32_t, 4> OUTLINE_INDICES{{0, 1, 3, 2}};

	explicit Renderer(std::shared_ptr<Shader> render_shader, std::shared_ptr<Shader> grid_shader);

	void setClearColor(Vec4<float> color) {
		clear_color_ = color;
	}

	void toggleGrid() noexcept {
		grid_on_ = !grid_on_;
	}
	void setGridColor(Vec4<float> color) noexcept {
		view_config_.outline_color = color;
	}

	void setColors(const std::vector<Vec4<float>> &colors);
	[[nodiscard]] auto colorCount() const noexcept {
		return color_count_;
	}
	[[nodiscard]] const auto &colors() const noexcept {
		return colors_;
	}

	void updateView(Vec2<float> offset_vec, float scale_vec) noexcept;

	void draw(Vec2<int> win_size, const CellMap &cellmap) noexcept;

	void destroy();
};

} // namespace CSIM
#endif // CELLSIM_RENDERER_HPP
