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

	/// creating uniform buffers
	std::array<GLuint, 2> buffers; // NOLINT initialization through ptr
	glCreateBuffers(buffers.size(), buffers.data());
	base_config_ubo_id_ = buffers[0];
	config_ubo_id_ = buffers[1];

	/// allocating storage
	glNamedBufferStorage(base_config_ubo_id_, sizeof(BaseConfig), &base_config_,
											 GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferStorage(config_ubo_id_,
											 static_cast<GLsizeiptr>(rule_config_->size()),
											 rule_config_->data(), GL_DYNAMIC_STORAGE_BIT);

	/// binding buffers to binding indices
	rule_config_->getShader()->bind();
	glBindBufferBase(GL_UNIFORM_BUFFER, shconfig::BASE_CONFIG_UBO_BINDING_LOCATION,
									 base_config_ubo_id_);
	glBindBufferBase(GL_UNIFORM_BUFFER, shconfig::CONFIG_UBO_BINDING_LOCATION, config_ubo_id_);
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


void CSIM::Rule::setRuleConfig(std::shared_ptr<RuleConfig> rule_config) {
	rule_config_ = std::move(rule_config); /// change config
	/// rebind buffers to binding indices with new compute shader bound
	rule_config_->getShader()->bind();
	glBindBufferBase(GL_UNIFORM_BUFFER, shconfig::BASE_CONFIG_UBO_BINDING_LOCATION,
									 base_config_ubo_id_);
	glBindBufferBase(GL_UNIFORM_BUFFER, shconfig::CONFIG_UBO_BINDING_LOCATION, config_ubo_id_);
	rule_config_->getShader()->unbind();
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

void CSIM::Rule1D::step(const CellMap &cell_map, std::int32_t state_count) noexcept { // NOLINT
	this->bindConfigShader();

	BaseConfig config;
	config.map_resolution = cell_map.resolution();
	config.state_count = state_count;
	/// iteration will be used to select currently processed row
	config.iteration = this->iterate() % config.map_resolution.y;

	this->setBaseConfig(config);

	/// dispatch with mapwidth workgroups
	glDispatchCompute(static_cast<GLuint>(config.map_resolution.x), 1, 1);

	Shader::unbind();
}

void CSIM::Rule1D::destroy() {
	Rule::destroy();
}

/// Rule2D impl ///

CSIM::Rule2D::Rule2D(std::shared_ptr<RuleConfig> rule_config)
		: Rule(std::move(rule_config)) {
}

void CSIM::Rule2D::step(const CellMap &cell_map, std::int32_t state_count) noexcept {
	this->bindConfigShader();

	BaseConfig config;
	config.map_resolution = cell_map.resolution();
	config.state_count = state_count;
	config.iteration = this->iterate();

	this->setBaseConfig(config);

	const auto size = static_cast<GLsizeiptr>(
			sizeof(std::int32_t) * config.map_resolution.x * config.map_resolution.y);
	if (config.map_resolution.x != previous_map_resolution_.x ||
			config.map_resolution.y != previous_map_resolution_.y) {
		if (state_map_copy_ssbo_id_ != 0)	{
			glDeleteBuffers(1, &state_map_copy_ssbo_id_);
		}
		glCreateBuffers(1, &state_map_copy_ssbo_id_);

		glNamedBufferStorage(state_map_copy_ssbo_id_, size, nullptr, 0);

		constexpr GLuint binding{ 6 };
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, state_map_copy_ssbo_id_);

		previous_map_resolution_ = config.map_resolution;
	}

	glCopyNamedBufferSubData(cell_map.stateMapSsboId(), state_map_copy_ssbo_id_, 0, 0, size);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	/// dispatch for every cell on the map
	glDispatchCompute(static_cast<GLuint>(config.map_resolution.x),
										static_cast<GLuint>(config.map_resolution.y), 1);

	Shader::unbind();
}

void CSIM::Rule2D::destroy() {
	Rule::destroy();
	if (state_map_copy_ssbo_id_ != 0) {
		glDeleteBuffers(1, &state_map_copy_ssbo_id_);
	}
}
