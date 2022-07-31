//
// Created by reg on 7/29/22.
//

#ifndef CELLSIM_CELLMAP_HPP
#define CELLSIM_CELLMAP_HPP

#include <shaders/shaders.hpp>
#include "utils/vecs.hpp"

#include <memory>
#include <vector>

namespace CSIM {

using namespace utils;

struct CellMap {
	std::uint32_t instance_offsets_ssbo_id_{ 0 };
	std::uint32_t state_map_ssbo_id_       { 0 };

	std::shared_ptr<const Shader> render_shader_{ nullptr };

	std::size_t width_;
	std::size_t height_;
	std::vector<Vec2<float>> cell_offsets_;
	std::vector<std::int32_t> cell_states_;
	std::int32_t state_count_;
	static constexpr std::int32_t INITIAL_LIFE_STATE{ 1 };

	CellMap(std::shared_ptr<const Shader> render_shader,
					std::size_t width,
					std::size_t height,
					std::int32_t state_count);

	void seed(std::size_t x, std::size_t y) noexcept;

	void generateOffsets() noexcept;

	void destroy() noexcept;
};

}

#endif // CELLSIM_CELLMAP_HPP
