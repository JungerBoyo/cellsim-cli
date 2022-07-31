//
// Created by reg on 7/29/22.
//

#ifndef CELLSIM_SHADERS_HPP
#define CELLSIM_SHADERS_HPP

#include <filesystem>
#include <vector>

namespace CSIM {

struct Shader { // NOLINT no need for move constructors
	std::uint32_t program_id_;

	Shader();

	void bind() const noexcept;

	static void unbind();

	static std::vector<char> parseShader(const std::filesystem::path &path);

	virtual void destroy() noexcept;

	virtual ~Shader() = default;
};

struct VFShader : public Shader {
	std::uint32_t vsh_id_;
	std::uint32_t fsh_id_;

	VFShader(const std::filesystem::path &vsh_path,
					 const std::filesystem::path &fsh_path);

	void destroy() noexcept override;
};

struct CShader : public Shader {
	std::uint32_t csh_id_;

	explicit CShader(const std::filesystem::path &csh_path);

	void destroy() noexcept override;
};

}

#endif // CELLSIM_SHADERS_HPP
