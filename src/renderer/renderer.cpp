//
// Created by reg on 7/29/22.
//

#include "renderer/renderer.hpp"

#include <algorithm>
#include <glad/glad.h>

CSIM::Renderer::Renderer(
		Vec2<int> win_size,
		std::shared_ptr<Shader> render_shader,
		std::shared_ptr<Shader> grid_shader) :
	main_fbo_(win_size.x, win_size.y),
	render_shader_(std::move(render_shader)), 
	grid_shader_(std::move(grid_shader)) {

	std::array<GLuint, 4> buffers; // NOLINT initialization through ptr
	glCreateBuffers(buffers.size(), buffers.data());
	vbo_id_ = buffers[0];
	outline_ibo_od_ = buffers[1];
	view_config_ubo_id_ = buffers[2];
	colors_ubo_id_ = buffers[3];

	// set up vertex data
	glNamedBufferStorage(vbo_id_, QUAD.size() * sizeof(float), QUAD.data(),
											 0); // NOLINT no flags = readonly data
	// set up outline index data
	glNamedBufferStorage(outline_ibo_od_, OUTLINE_INDICES.size() * sizeof(std::uint32_t),
											 OUTLINE_INDICES.data(), 0); // NOLINT no flags = readonly data

	// specify attrib layout
	glCreateVertexArrays(1, &vao_id_); // NOLINT single vao
	glBindVertexArray(vao_id_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id_);

	glVertexAttribPointer(IN_POSITION_LOCATION, 2, GL_FLOAT, GL_FALSE, 0,
												nullptr); // NOLINT stride=0, type=fvec2
	glEnableVertexAttribArray(IN_POSITION_LOCATION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// set up ubos for ViewConfig and Colors
	glNamedBufferStorage(view_config_ubo_id_, sizeof(ViewConfig), &view_config_,
											 GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, VIEW_CONFIG_UBO_BINDING_LOCATION,
									 view_config_ubo_id_);
	glNamedBufferStorage(colors_ubo_id_, static_cast<GLsizei>(colors_.size() * sizeof(Vec4<float>)),
											 colors_.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, COLORS_UBO_BINDING_LOCATION, colors_ubo_id_);
}

void CSIM::Renderer::setColors(const std::vector<Vec4<float>> &colors) {
	color_count_ = std::clamp(colors.size(), static_cast<std::size_t>(0), MAX_COLORS);
	std::copy_n(colors.begin(), color_count_, colors_.begin());

	glNamedBufferSubData(colors_ubo_id_, 0,
											 static_cast<GLsizeiptr>(sizeof(Vec4<float>) * color_count_), colors_.data());
}

void CSIM::Renderer::updateView(Vec2<float> offset_vec, float scale_vec) noexcept {
	view_config_.offset.x += time_step_ * offset_vec.x;
	view_config_.offset.y += time_step_ * offset_vec.y;

	view_config_.scale *= scale_vec;
}

void CSIM::Renderer::draw(Vec2<int> win_size, const CellMap &cellmap) noexcept {
	if (win_size.x <= 0 || win_size.y <= 0)	 {
		return;
	}

	if (main_fbo_.width() != win_size.x || main_fbo_.height() != win_size.y) {
		main_fbo_.resize(win_size.x, win_size.y);
	}
	main_fbo_.bind_framebuffer();
	glClearColor(clear_color_.x, clear_color_.y, clear_color_.z, clear_color_.w);
	glClear(GL_COLOR_BUFFER_BIT);

	render_shader_->bind();
	glBindVertexArray(vao_id_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id_);

	view_config_.aspect_ratio = static_cast<float>(win_size.y) / static_cast<float>(win_size.x);

	glNamedBufferSubData(view_config_ubo_id_, 0, sizeof(ViewConfig), &view_config_);
	glViewport(0, 0, win_size.x, win_size.y);

	// inserting memory barrier for a shader storages because of the state_map
	// which is modified in during the step
	glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
	const auto instance_count = static_cast<GLsizei>(cellmap.cell_offsets_.size());
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, QUAD.size() / 2, instance_count);
	render_shader_->unbind();

	if (grid_on_) {
		grid_shader_->bind();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, outline_ibo_od_);
		glDrawElementsInstanced(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, nullptr, instance_count);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		grid_shader_->unbind();
	}
	main_fbo_.unbind_framebuffer();

	auto cellmap_fbo = cellmap.textureFbo();
	cellmap_fbo.bind_framebuffer();
	glClearColor(clear_color_.x, clear_color_.y, clear_color_.z, clear_color_.w);
	glClear(GL_COLOR_BUFFER_BIT);
	render_shader_->bind();
	const auto scale = 2.f / static_cast<float>(cellmap_fbo.height());
	ViewConfig cellmap_view_config{
			.offset = {-1.f - scale * (-.5f),
						1.f - scale * .5f}, // NOLINT (-1.f, 1.f) = left upper corner
											// (-.5f, .5f) = left upper corner
											// of cell map
			.scale = scale,
			.aspect_ratio =
					static_cast<float>(cellmap_fbo.width()) / static_cast<float>(cellmap_fbo.height())};
	glNamedBufferSubData(view_config_ubo_id_, 0, sizeof(ViewConfig), &cellmap_view_config);
	glViewport(0, 0, cellmap_fbo.width(), cellmap_fbo.height());

	glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, QUAD.size() / 2, instance_count);
	render_shader_->unbind();
	cellmap_fbo.unbind_framebuffer();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void CSIM::Renderer::destroy() {
	std::vector<GLuint> buffers;
	buffers.reserve(4); // NOLINT

	buffers.push_back(vbo_id_);
	buffers.push_back(outline_ibo_od_);
	buffers.push_back(view_config_ubo_id_);
	buffers.push_back(colors_ubo_id_);

	glDeleteBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
}
