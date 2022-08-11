//
// Created by reg on 7/29/22.
//

#ifndef CELLSIM_CELLMAP_HPP
#define CELLSIM_CELLMAP_HPP

#include "utils/vecs.hpp"
#include <shaders/shaders.hpp>
#include "texture_backed_framebuffer.hpp"

#include <memory>
#include <vector>

namespace CSIM {

using namespace utils;

struct CellMap {
	static constexpr std::int32_t INITIAL_LIFE_STATE{1};

	std::uint32_t instance_offsets_ssbo_id_{0};
	std::uint32_t state_map_ssbo_id_{0};

	std::size_t width_;
	std::size_t height_;
	TextureBackedFramebuffer fbo_;
	std::vector<Vec2<float>> cell_offsets_;
	std::vector<std::int32_t> cell_states_;

	CellMap(std::size_t width, std::size_t height);

	[[nodiscard]] utils::Vec2<std::int32_t> resolution() const noexcept {
		return {
			static_cast<std::int32_t>(width_),
			static_cast<std::int32_t>(height_)
		};
	}

	void seed(std::size_t x, std::size_t y, std::size_t range, bool round, bool clip) noexcept;
	void extend(std::size_t new_width, std::size_t new_height, bool preserve_contents);

	[[nodiscard]] const auto& textureFbo() const { return fbo_; }

	void generateOffsets() noexcept;

	void destroy() noexcept;
};

} // namespace CSIM

#endif // CELLSIM_CELLMAP_HPP
