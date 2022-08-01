//
// Created by reg on 7/29/22.
//
#include "rules/rule.hpp"
#include "shconfig.hpp"
#include <glad/glad.h>
#include <array>

using namespace CSIM::utils;

/// RULE impl ///

CSIM::Rule::Rule(std::shared_ptr<RuleConfig> rule_config)
		: base_config_{{}, 0, 0}, rule_config_(std::move(rule_config)) {

	std::array<GLuint, 2> buffers; // NOLINT initialization through ptr
	glCreateBuffers(buffers.size(), buffers.data());
	base_config_ubo_id_ = buffers[0];
	config_ubo_id_ = buffers[1];

	glNamedBufferStorage(base_config_ubo_id_, sizeof(BaseConfig), &base_config_,
											 GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferStorage(config_ubo_id_,
											 static_cast<GLsizeiptr>(rule_config_->size()),
											 rule_config_->data(), GL_DYNAMIC_STORAGE_BIT);

	rule_config_->getShader()->bind();
	glBindBufferBase(GL_UNIFORM_BUFFER,
									 shconfig::BASE_CONFIG_UBO_BINDING_LOCATION,
									 base_config_ubo_id_);
	glBindBufferBase(GL_UNIFORM_BUFFER, shconfig::CONFIG_UBO_BINDING_LOCATION,
									 config_ubo_id_);

	// NOLINTBEGIN
	GLint active_resources{0};
	glGetProgramInterfaceiv(rule_config_->getShader()->program_id_,
													GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES,
													&active_resources);
	for (auto i = 0; i < active_resources; ++i) {
		GLsizei args[256];
		constexpr GLsizei buf_size = 256;
		std::fill(args, args + buf_size, 0);
		GLsizei len{0};
		GLenum prop[] = {GL_BUFFER_DATA_SIZE};

		glGetProgramResourceiv(rule_config_->getShader()->program_id_,
													 GL_UNIFORM_BLOCK, i, 1, prop, buf_size, &len, args);

		int a = 5;
	}

	// NOLINTEND

	rule_config_->getShader()->unbind();
}
void CSIM::Rule::setBaseConfig(BaseConfig config,
															 bool update_iteration) noexcept {
	if (update_iteration) {
		base_config_ = config;
	} else {
		base_config_.map_resolution = config.map_resolution;
		base_config_.state_count = config.state_count;
	}
	glNamedBufferSubData(base_config_ubo_id_, 0, sizeof(BaseConfig), &config);
}
void CSIM::Rule::destroy() {
	std::array<GLuint, 2> buffers; // NOLINT
	buffers[0] = base_config_ubo_id_;
	buffers[1] = config_ubo_id_;

	glDeleteBuffers(buffers.size(), buffers.data());
}

/// Rule1D impl ///

CSIM::Rule1D::Rule1D(std::shared_ptr<RuleConfig> rule_config)
		: Rule(std::move(rule_config)) {
}

void CSIM::Rule1D::step(const CellMap &cell_map) noexcept { // NOLINT
	this->rule_config_->getShader()->bind();

	BaseConfig config;
	config.map_resolution = {static_cast<std::int32_t>(cell_map.width_),
													 static_cast<std::int32_t>(cell_map.height_)};
	config.state_count = cell_map.state_count_;
	config.iteration = this->iteration() % config.map_resolution.y;

	this->setBaseConfig(config);

	glDispatchCompute(static_cast<GLuint>(config.map_resolution.x), 1, 1);

	this->rule_config_->getShader()->unbind();
}

void CSIM::Rule1D::destroy() {
	Rule::destroy();
}
