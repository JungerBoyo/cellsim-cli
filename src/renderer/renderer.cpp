//
// Created by reg on 7/29/22.
//

#include "shconfig.hpp"
#include "renderer/renderer.hpp"
#include <glad/glad.h>
#include <algorithm>

CSIM::Renderer::Renderer(std::shared_ptr<const Shader> render_shader)
	: render_shader_(std::move(render_shader)) {

	std::array<GLuint, 3> buffers; // NOLINT initialization through ptr
	glCreateBuffers(buffers.size(), buffers.data());
	vbo_id_ 						= buffers[0];
	view_config_ubo_id_ = buffers[1];
	colors_ubo_id_ 			= buffers[2];

	// set up vertex data
	glNamedBufferStorage(vbo_id_, QUAD.size() * sizeof(float), QUAD.data(),
											 0); // NOLINT no flags = readonly data

	// specify attrib layout
	glCreateVertexArrays(1, &vao_id_); // NOLINT single vao
	glBindVertexArray(vao_id_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id_);

	glVertexAttribPointer(shconfig::IN_POSITION_LOCATION, 2, GL_FLOAT, GL_FALSE, 0,
												nullptr); // NOLINT stride=0, type=fvec2
	glEnableVertexAttribArray(shconfig::IN_POSITION_LOCATION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// set up ubos for ViewConfig and Colors
	render_shader_->bind();

	glNamedBufferStorage(view_config_ubo_id_, sizeof(ViewConfig), &view_config_,
											 GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, shconfig::VIEW_CONFIG_UBO_BINDING_LOCATION,
									 view_config_ubo_id_);

	glNamedBufferStorage(
			colors_ubo_id_,
			static_cast<GLsizei>(colors_.size() * sizeof(Vec4<float>)),
			colors_.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, shconfig::COLORS_UBO_BINDING_LOCATION,
									 colors_ubo_id_);

	render_shader_->unbind();
}

void CSIM::Renderer::setColors(std::span<Vec4<float>> colors) {
	const std::size_t colors_size =
			std::clamp(colors.size(), 0ul, shconfig::MAX_COLORS - 1);
	std::copy_n(colors.begin(), colors_size, colors_.begin());

	glNamedBufferSubData(colors_ubo_id_, 0,
		static_cast<GLsizeiptr>(sizeof(Vec4<float>) * colors_size),
		colors_.data());
}

void CSIM::Renderer::updateView(Vec2<float> offset_vec, float scale_vec) noexcept {
	view_config_.offset.x += time_step_ * offset_vec.x;
	view_config_.offset.y += time_step_ * offset_vec.y;

	view_config_.scale *= scale_vec;
}

void CSIM::Renderer::draw(Vec2<int> win_size, const CellMap &cell_map) noexcept {
	render_shader_->bind();
	glBindVertexArray(vao_id_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id_);

	view_config_.aspect_ratio =
		static_cast<float>(win_size.y)/static_cast<float>(win_size.x);

	glNamedBufferSubData(view_config_ubo_id_, 0, sizeof(ViewConfig), &view_config_);

	// inserting memory barrier for a shader storages because of the state_map
	// which is modified in during the step
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glViewport(0, 0, win_size.x, win_size.y);
	glDrawArraysInstanced(
			GL_TRIANGLE_STRIP, 0, QUAD.size() / 2,
			static_cast<GLsizei>(cell_map.cell_offsets_.size()));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	render_shader_->unbind();
}

void CSIM::Renderer::destroy() {
	std::vector<GLuint> buffers;
	buffers.reserve(3); // NOLINT

	buffers.push_back(vbo_id_);
	buffers.push_back(view_config_ubo_id_);
	buffers.push_back(colors_ubo_id_);
}
