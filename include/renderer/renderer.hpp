//
// Created by reg on 7/29/22.
//

#ifndef CELLSIM_RENDERER_HPP
#define CELLSIM_RENDERER_HPP

#include <cellmap/cellmap.hpp>
#include <shaders/shaders.hpp>
#include "utils/vecs.hpp"

#include <memory>
#include <array>
#include <span>

namespace CSIM {

using namespace utils;

struct Renderer {
	std::uint32_t vbo_id_							{ 0 };
	std::uint32_t vao_id_							{ 0 };
	std::uint32_t view_config_ubo_id_	{ 0 };
	std::uint32_t colors_ubo_id_			{ 0 };

	std::shared_ptr<const Shader> render_shader_{nullptr};
	float time_step_{0.f};

	struct ViewConfig {
		Vec2<float> offset{0.f, 0.f};
		float scale{1.f};
		float aspect_ratio{1.f};
	};

	ViewConfig view_config_{};
	std::array<Vec4<float>, shconfig::MAX_COLORS> colors_;

	static constexpr std::array<float, 8> QUAD {{
		-.5f,-.5f,
		-.5f, .5f,
		 .5f,-.5f,
		 .5f, .5f
	}};

	explicit Renderer(std::shared_ptr<const Shader> render_shader);

	void setColors(std::span<Vec4<float>> colors);

	void updateView(Vec2<float> offset_vec, float scale_vec) noexcept;

	void draw(Vec2<int> win_size, const CellMap &cell_map) noexcept;

	void destroy();
};

} // namespace CSIM
#endif // CELLSIM_RENDERER_HPP
