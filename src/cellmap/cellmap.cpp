//
// Created by reg on 7/29/22.
//
#include "cellmap/cellmap.hpp"
#include "shconfig.hpp"
#include <array>
#include <algorithm>
#include <glad/glad.h>

CSIM::CellMap::CellMap(std::size_t width, std::size_t height)
		: width_(width), height_(height), fbo_(static_cast<std::int32_t>(width),
																					 static_cast<std::int32_t>(height)),
			cell_offsets_(width * height), cell_states_(width * height, 0) {
	generateOffsets();

	std::array<GLuint, 2> buffers; // NOLINT
	glCreateBuffers(buffers.size(), buffers.data());
	instance_offsets_ssbo_id_ = buffers[0];
	state_map_ssbo_id_ = buffers[1];

	// set up ssbos
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
}

void CSIM::CellMap::seed(std::size_t x, std::size_t y, std::size_t range, bool round,
												 bool clip) noexcept {
	std::fill(cell_states_.begin(), cell_states_.end(), 0);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glGetNamedBufferSubData(state_map_ssbo_id_, 0, static_cast<GLsizei>(width_ * height_),
													cell_states_.data());
	if (!round) {
		std::int64_t begin_x{ 0 };
		std::int64_t end_x{ 0 };
		std::int64_t begin_y{ 0 };
		std::int64_t end_y{ 0 };
		if (clip)	{
			begin_x = std::clamp(static_cast<std::int64_t>(x) - static_cast<std::int64_t>(range),
													 static_cast<std::int64_t>(0),
													 static_cast<std::int64_t>(width_));
			end_x = std::clamp(static_cast<std::int64_t>(x) + static_cast<std::int64_t>(range) + 1,
												 static_cast<std::int64_t>(0),
												 static_cast<std::int64_t>(width_));
			begin_y = std::clamp(static_cast<std::int64_t>(y) - static_cast<std::int64_t>(range),
													 static_cast<std::int64_t>(0),
													 static_cast<std::int64_t>(height_));
			end_y = std::clamp(static_cast<std::int64_t>(y) + static_cast<std::int64_t>(range) + 1,
												 static_cast<std::int64_t>(0),
												 static_cast<std::int64_t>(height_));
		} else {
			begin_x = (static_cast<std::int64_t>(x) - static_cast<std::int64_t>(range)) %
								static_cast<std::int64_t>(width_);
			end_x = (static_cast<std::int64_t>(x) + static_cast<std::int64_t>(range) + 1) %
							static_cast<std::int64_t>(width_);
			begin_y = (static_cast<std::int64_t>(y) - static_cast<std::int64_t>(range)) %
								static_cast<std::int64_t>(height_);
			end_y = (static_cast<std::int64_t>(y) + static_cast<std::int64_t>(range) + 1) %
							static_cast<std::int64_t>(height_);
		}
		for (y=begin_y; y != end_y; y = (y + 1) % (height_ + 1)) {
			for (x=begin_x; x != end_x; x = (x + 1) % (width_ + 1))	{
				cell_states_.at(x + y * width_) = INITIAL_LIFE_STATE;
			}
		}
	} else {
		/// Bresenham's circle algorithm
		for (auto range_i=static_cast<std::int64_t>(range); range_i != 0; --range_i) {
			std::int64_t p_x{ 0 };
			std::int64_t p_y{ range_i };
			std::int64_t diff{ 3 - 2 * range_i };
			while (p_x <= p_y)	{
				std::int64_t x0{ 0 };
				std::int64_t x1{ 0 };
				std::int64_t y0{ 0 };
				std::int64_t y1{ 0 };
				if(clip) {
					x0 = std::clamp(( p_x + static_cast<std::int64_t>(x)),
													  static_cast<std::int64_t>(0),
												  	static_cast<std::int64_t>(width_));
					x1 = std::clamp((-p_x + static_cast<std::int64_t>(x)),
													static_cast<std::int64_t>(0),
													static_cast<std::int64_t>(width_));
					y0 = std::clamp(( p_y + static_cast<std::int64_t>(y)),
													static_cast<std::int64_t>(0),
													static_cast<std::int64_t>(height_));
					y1 = std::clamp((-p_y + static_cast<std::int64_t>(y)),
													static_cast<std::int64_t>(0),
													static_cast<std::int64_t>(height_));
				} else {
					x0 = ( p_x + static_cast<std::int64_t>(x)) % static_cast<std::int64_t>(width_);
					x1 = (-p_x + static_cast<std::int64_t>(x)) % static_cast<std::int64_t>(width_);
					y0 = ( p_y + static_cast<std::int64_t>(y)) % static_cast<std::int64_t>(height_);
					y1 = (-p_y + static_cast<std::int64_t>(y)) % static_cast<std::int64_t>(height_);
				}
				cell_states_.at(x0 + y0 * width_) = INITIAL_LIFE_STATE;
				cell_states_.at(x1 + y0 * width_) = INITIAL_LIFE_STATE;
				cell_states_.at(x0 + y1 * width_) = INITIAL_LIFE_STATE;
				cell_states_.at(x1 + y1 * width_) = INITIAL_LIFE_STATE;
				cell_states_.at(y0 + x0 * width_) = INITIAL_LIFE_STATE;
				cell_states_.at(y1 + x0 * width_) = INITIAL_LIFE_STATE;
				cell_states_.at(y0 + x1 * width_) = INITIAL_LIFE_STATE;
				cell_states_.at(y1 + x1 * width_) = INITIAL_LIFE_STATE;

				if (diff < 0) {
					++p_x;
					diff += 4 * p_x + 6l; // NOLINT algorithm defined coefficient
				} else {
					++p_x;
					--p_y;
					diff += 4 * (p_x - p_y) + 10l; // NOLINT algorithm defined coefficient
				}
			}
		}
	}

	glNamedBufferSubData(state_map_ssbo_id_, 0,
											 static_cast<GLsizei>(cell_states_.size() * sizeof(std::int32_t)),
											 cell_states_.data());
}

void CSIM::CellMap::extend(std::size_t new_width, std::size_t new_height, bool preserve_contents) {
	if (new_width == width_ && new_height == height_) { return; }

	{
		std::vector<std::int32_t> new_cell_states(new_width * new_height, 0);
		if (preserve_contents) {
			std::fill(cell_states_.begin(), cell_states_.end(), 0);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			glGetNamedBufferSubData(state_map_ssbo_id_, 0, static_cast<GLsizei>(width_ * height_),
															cell_states_.data());
			for (std::size_t y=0; y<std::clamp(height_, static_cast<std::size_t>(0), new_height); ++y) {
				for (std::size_t x=0; x<std::clamp(width_, static_cast<std::size_t>(0), new_width); ++x) {
					new_cell_states.at(x + y * new_width) = cell_states_.at(x + y * width_);
				}
			}
		}
		cell_states_ = std::move(new_cell_states);
	}
	width_ = new_width;
	height_ = new_height;

	cell_offsets_.resize(new_width * new_height);
	generateOffsets();

	fbo_.resize(static_cast<std::int32_t>(new_width), static_cast<std::int32_t>(new_height));
	std::array<GLuint, 2> buffers; // NOLINT
	buffers[0] = instance_offsets_ssbo_id_;
	buffers[1] = state_map_ssbo_id_;

	glDeleteBuffers(buffers.size(), buffers.data());

	glCreateBuffers(buffers.size(), buffers.data());
	instance_offsets_ssbo_id_ = buffers[0];
	state_map_ssbo_id_ = buffers[1];

	// set up ssbos
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
}

void CSIM::CellMap::generateOffsets() noexcept {
	for (auto y = 0ul; y < height_; ++y) {
		for (auto x = 0ul; x < width_; ++x) {
			cell_offsets_.at(x + y * width_) = {static_cast<float>(x), -static_cast<float>(y)};
		}
	}
}

void CSIM::CellMap::destroy() noexcept {
	fbo_.destroy();

	std::array<GLuint, 2> buffers; // NOLINT
	buffers[0] = instance_offsets_ssbo_id_;
	buffers[1] = state_map_ssbo_id_;

	glDeleteBuffers(buffers.size(), buffers.data());
}