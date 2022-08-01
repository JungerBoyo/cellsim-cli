//
// Created by reg on 7/29/22.
//
#include "cellmap/cellmap.hpp"
#include "shconfig.hpp"
#include <array>
#include <glad/glad.h>

CSIM::CellMap::CellMap(std::shared_ptr<const Shader> render_shader,
											 std::size_t width, std::size_t height,
											 std::int32_t state_count)
		: render_shader_(std::move(render_shader)), width_(width), height_(height),
			cell_offsets_(width * height), cell_states_(width * height, 0),
			state_count_(state_count) {
	generateOffsets();

	std::array<GLuint, 2> buffers; // NOLINT
	glCreateBuffers(buffers.size(), buffers.data());
	instance_offsets_ssbo_id_ = buffers[0];
	state_map_ssbo_id_ = buffers[1];

	// set up ssbos
	render_shader_->bind();

	glNamedBufferStorage(
			instance_offsets_ssbo_id_,
			static_cast<GLsizei>(cell_offsets_.size() * sizeof(Vec2<float>)),
			cell_offsets_.data(), 0); // NOLINT no flags = readonly
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
									 shconfig::INSTANCE_OFFSETS_SSBO_BINDING_LOCATION,
									 instance_offsets_ssbo_id_);

	glNamedBufferStorage(
			state_map_ssbo_id_,
			static_cast<GLsizei>(cell_states_.size() * sizeof(std::int32_t)),
			cell_states_.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
									 shconfig::STATE_MAP_SSBO_BINDING_LOCATION,
									 state_map_ssbo_id_);

	render_shader_->unbind();
}

void CSIM::CellMap::seed(std::size_t x, std::size_t y) noexcept {
	const auto index = x + y * width_;
	cell_states_.at(index) = INITIAL_LIFE_STATE;

	glNamedBufferSubData(state_map_ssbo_id_,
											 static_cast<GLintptr>(index * sizeof(std::int32_t)),
											 sizeof(std::int32_t), &INITIAL_LIFE_STATE);
}

void CSIM::CellMap::generateOffsets() noexcept {
	for (auto y = 0ul; y < height_; ++y) {
		for (auto x = 0ul; x < width_; ++x) {
			cell_offsets_.at(x + y * width_) = {static_cast<float>(x),
																					-static_cast<float>(y)};
		}
	}
}

void CSIM::CellMap::destroy() noexcept {
	std::array<GLuint, 2> buffers; // NOLINT
	buffers[0] = instance_offsets_ssbo_id_;
	buffers[1] = state_map_ssbo_id_;

	glDeleteBuffers(buffers.size(), buffers.data());
};