#ifndef CELLSIM_GLCONFIG_HPP
#define CELLSIM_GLCONFIG_HPP

#include <cinttypes>
#include <string_view>

namespace CSIM::shconfig {
	static constexpr std::int32_t GLVERSION_MAJOR { 4 };
	static constexpr std::int32_t GLVERSION_MINOR { 5 };
	/// render
	static constexpr std::size_t MAX_COLORS = 256;
	static constexpr std::uint32_t IN_POSITION_LOCATION										{ 0 };
	static constexpr std::uint32_t VIEW_CONFIG_UBO_BINDING_LOCATION				{ 0 };
	static constexpr std::uint32_t COLORS_UBO_BINDING_LOCATION						{ 1 };
	static constexpr std::uint32_t INSTANCE_OFFSETS_SSBO_BINDING_LOCATION	{ 3 };
	/// rule + render
	static constexpr std::uint32_t STATE_MAP_SSBO_BINDING_LOCATION { 2 };
	/// rule
	static constexpr std::uint32_t BASE_CONFIG_UBO_BINDING_LOCATION { 4 };
	static constexpr std::uint32_t CONFIG_UBO_BINDING_LOCATION		  { 5 };

	static constexpr std::string_view VSH_RENDER_SHADER_PATH = "shaders/bin/render_shader/vert.spv";
	static constexpr std::string_view FSH_RENDER_SHADER_PATH = "shaders/bin/render_shader/frag.spv";

	static constexpr std::string_view VSH_GRID_SHADER_PATH = "shaders/bin/grid_shader/vert.spv";
	static constexpr std::string_view FSH_GRID_SHADER_PATH = "shaders/bin/grid_shader/frag.spv";
}

#endif
