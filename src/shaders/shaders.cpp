//
// Created by reg on 7/29/22.
//

#include "shaders/shaders.hpp"

#include <fstream>
#include <glad/glad.h>
#include <fmt/format.h>

/// Shader impl ///

CSIM::Shader::Shader() : program_id_(glCreateProgram()) {
}

void CSIM::Shader::bind() const noexcept {
	glUseProgram(program_id_);
}

void CSIM::Shader::unbind() {
	glUseProgram(0);
}

std::vector<char> CSIM::Shader::parseShader(const std::filesystem::path &path) {
	std::ifstream stream(path.c_str(), std::ios::binary | std::ios::ate);

	if (!stream.good()) {
		throw std::runtime_error(fmt::format(
				"failed to create stream from shader file {}", path.c_str()));
	}

	const auto size = static_cast<std::size_t>(stream.tellg());
	std::vector<char> code(size);

	stream.seekg(0);
	stream.read(code.data(), static_cast<std::streamsize>(size));

	stream.close();

	return code;
}

void CSIM::Shader::destroy() noexcept {
	glDeleteProgram(program_id_);
}

/// VFShader impl ///

CSIM::VFShader::VFShader(const std::filesystem::path &vsh_path,
				 const std::filesystem::path &fsh_path)
		: vsh_id_(glCreateShader(GL_VERTEX_SHADER)),
			fsh_id_(glCreateShader(GL_FRAGMENT_SHADER)) {
	const auto vsh_binary = parseShader(vsh_path);
	const auto fsh_binary = parseShader(fsh_path);

	glShaderBinary(1, &vsh_id_, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB,
								 vsh_binary.data(), static_cast<GLsizei>(vsh_binary.size()));
	glShaderBinary(1, &fsh_id_, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB,
								 fsh_binary.data(), static_cast<GLsizei>(fsh_binary.size()));

	glSpecializeShaderARB(vsh_id_, "main", 0, nullptr, nullptr);
	glSpecializeShaderARB(fsh_id_, "main", 0, nullptr, nullptr);

	glAttachShader(this->program_id_, vsh_id_);
	glAttachShader(this->program_id_, fsh_id_);

	glValidateProgram(this->program_id_);
	glLinkProgram(this->program_id_);
}

void CSIM::VFShader::destroy() noexcept {
	glDetachShader(this->program_id_, vsh_id_);
	glDetachShader(this->program_id_, fsh_id_);

	glDeleteShader(vsh_id_);
	glDeleteShader(fsh_id_);

	Shader::destroy();
}

/// CShader impl ///

CSIM::CShader::CShader(const std::filesystem::path &csh_path)
		: csh_id_(glCreateShader(GL_COMPUTE_SHADER)) {
	const auto csh_binary = parseShader(csh_path);

	glShaderBinary(1, &csh_id_, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB,
								 csh_binary.data(), static_cast<GLsizei>(csh_binary.size()));

	glSpecializeShaderARB(csh_id_, "main", 0, nullptr, nullptr);

	glAttachShader(this->program_id_, csh_id_);

	glValidateProgram(this->program_id_);
	glLinkProgram(this->program_id_);
}

void CSIM::CShader::destroy() noexcept {
	glDetachShader(this->program_id_, csh_id_);

	glDeleteShader(csh_id_);

	Shader::destroy();
}