#include <stdexcept>
#include <vector>
#include <memory>
#include <array>
#include <span>
#include <algorithm>

#include <fmt/core.h>
#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shconfig.hpp"
#include "utils/vecs.hpp"
#include <renderer/renderer.hpp>
#include <cellmap/cellmap.hpp>
#include <rules/rule.hpp>
#include <rules/rule_config.hpp>
#include <shaders/shaders.hpp>

// NOLINTBEGIN
void glDebugCallback(
  GLenum, 
  GLenum, 
  GLuint id, 
  GLenum severity, 
  GLsizei, 
  const GLchar* message,
  const void*  
) {
  if(severity == GL_DEBUG_SEVERITY_HIGH) {
    spdlog::error("[glad] {}, id = {}", message, id);
  } else {
    spdlog::info("[glad] {}, id = {}", message, id);
  }
}
// NOLINTEND

namespace CLI {
// checks size of the vector in range [min_size, max_size]
struct VecSizeValidator : public CLI::Validator {
	explicit VecSizeValidator(std::size_t min_size, std::size_t max_size) {
		name_ = "VECSIZE";
		func_ = [min_size, max_size](const std::string& vec) {
			auto size = static_cast<std::size_t>(std::count(vec.begin(), vec.end(), ' '));
			if(vec.back() == ' ' || (size == 0 && !vec.empty())) {
				++size;
			}
			if (size < min_size || size > max_size) {
				return fmt::format("vector size {} isn't contained in {}, {}",
													 size, min_size, max_size);
			} else {
				return std::string();
			}
		};
	}
};
} // namespace CLI

using namespace CSIM::utils;

int main(int argc, const char* const* argv) {
// parse command arguments //
	CLI::App app_cli_context{"CellSim cli based prototype"};

	std::size_t arg_map_width{ 0 };
	std::size_t arg_map_height{ 0 };
	std::size_t arg_counter{ 0 };
	std::vector<std::uint32_t> arg_colors;
	arg_colors.reserve(CSIM::shconfig::MAX_COLORS);

	app_cli_context.add_option(
		"--width", arg_map_width,
		"width of the cell map")
			->check(CLI::TypeValidator<std::size_t>());

	app_cli_context.add_option(
		"--height", arg_map_height,
		"height of the cell map")
			->check(CLI::TypeValidator<std::size_t>());

	app_cli_context.add_option(
		"--colors", arg_colors,
		"add history colors to cells in array of hex values (max 255)")
			->check(CLI::VecSizeValidator(1, 255));// NOLINT

	app_cli_context.add_option(
		"--step-delay", arg_counter,
		"step delay between steps in fps")
			->check(CLI::TypeValidator<std::size_t>());

	// 1D totalistic
	struct { // NOLINT initialized through ref
		std::uint32_t range;
		bool center_active;
		std::vector<std::size_t> survival_conditions;
		std::vector<std::size_t> birth_conditions;
	} cli_1dtotalistic_subcmd_data;
	auto* ca_type_1dtotalistic_subcmd = app_cli_context.add_subcommand(
			"1dtotalistic", "1d totalistic is one dimensional ca rule");
	ca_type_1dtotalistic_subcmd->add_option(
		"-r,--range", cli_1dtotalistic_subcmd_data.range,
		"describes range of the neighbourhood (0 ..<= 10)")
			->check(
					CLI::Range(CSIM::RuleConfig1DTotalistic::RANGE_LIM.x,
										         CSIM::RuleConfig1DTotalistic::RANGE_LIM.y));

	ca_type_1dtotalistic_subcmd->add_flag(
		"-e,--exclude-center",
		"if set then center cell won't be taken into account");

	ca_type_1dtotalistic_subcmd->add_option(
		"-s,--survive-conditions",
		cli_1dtotalistic_subcmd_data.survival_conditions,
		"array of sums which qualify cell for survival");

	ca_type_1dtotalistic_subcmd->add_option(
		"-b,--birth-conditions",
		cli_1dtotalistic_subcmd_data.birth_conditions,
		"array of sums which qualify cell for birth");

	// 1D binary
	struct { // NOLINT initialized through ref
		std::uint32_t range;
		std::string pattern_match_code;
	}cli_1dbinary_subcmd_data;
	auto* ca_type_1dbinary_subcmd = app_cli_context.add_subcommand(
			"1dbinary", "1d binary is one dimensional ca rule");
	ca_type_1dbinary_subcmd->add_option(
		"-r,--range",
		cli_1dbinary_subcmd_data.range,
		"describes range of the neighbourhood (0 ..<= 4)")
			->check(CLI::Range(CSIM::RuleConfig1DBinary::RANGE_LIM.x,
												 				CSIM::RuleConfig1DBinary::RANGE_LIM.y));

	ca_type_1dbinary_subcmd->add_option(
		"-p,--patern-match-code",
		cli_1dbinary_subcmd_data.pattern_match_code,
		"defines which patterns of size 2*<range>+1 qualify cell for birth/survival, 1 = qualifed, 0 = unqualified, goes from min to max");

	// parsing
	try { app_cli_context.parse(argc, argv); }
	catch(const CLI::ParseError& e) {
	  if(app_cli_context.exit(e) != static_cast<int>(CLI::ExitCodes::Success)) {
	    spdlog::error("[CLI11] {}: {}", e.get_name(), e.what());
	    return e.get_exit_code();
	  } else if(e.get_name() == "CallForHelp" 		||
							e.get_name() == "CallForAllHelp"  ||
							e.get_name() == "CallForVersion") {
			return e.get_exit_code();
		}
	}

	// zero state is always black
	constexpr std::uint32_t BLACK_NON_TRANSPARENT_HEX{ 0xFF };
	arg_colors.insert(arg_colors.begin(), BLACK_NON_TRANSPARENT_HEX);

	std::vector<Vec4<float>> colors(arg_colors.size());
	std::transform(
		arg_colors.cbegin(),arg_colors.cend(),
		colors.begin(),
		[](std::uint32_t color){
			constexpr std::uint32_t MASK{ 0xFF };
			constexpr float DIVIDER{ 255.f };
			return Vec4<float>{
			  static_cast<float>((color >> 24u))			  /DIVIDER, // NOLINT
				static_cast<float>((color >> 16u) & MASK) /DIVIDER, // NOLINT
				static_cast<float>((color >>  8u) & MASK) /DIVIDER, // NOLINT
				static_cast<float>((color >>  0u) & MASK) /DIVIDER  // NOLINT
			};
		}
	);
/////////////////////////////

// init window and gl //
	// test glfw initialization
  if(glfwInit() != GLFW_TRUE) {
    spdlog::critical("glfw init failed");
    return 1;
	}
  
  // test window creation
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, CSIM::shconfig::GLVERSION_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, CSIM::shconfig::GLVERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  constexpr int width = 640;
  constexpr int height = 480;
  auto* window = glfwCreateWindow(width, height, "CellSim", nullptr, nullptr); 
  if(window == nullptr) {
		spdlog::critical("window creation failed");
    return 1;
	}  
	
	glfwMakeContextCurrent(window);
  glfwSetErrorCallback([](int, const char* message){ spdlog::error("[glfw] {}", message); });

  if(gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) { // NOLINT 
    spdlog::critical("glad loader failed");
    return 1;
	}

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(glDebugCallback, nullptr);
////////////////////////
  try {

		std::unique_ptr<CSIM::Rule> rule{nullptr};
		std::shared_ptr<CSIM::RuleConfig> rule_config{nullptr};
		if (ca_type_1dtotalistic_subcmd->parsed()) {
			rule_config = std::make_shared<CSIM::RuleConfig1DTotalistic>(
					static_cast<std::int32_t>(cli_1dtotalistic_subcmd_data.range),
					cli_1dtotalistic_subcmd_data.center_active,
					cli_1dtotalistic_subcmd_data.survival_conditions,
					cli_1dtotalistic_subcmd_data.birth_conditions,
					std::make_shared<CSIM::CShader>("shaders/bin/1D_totalistic/comp.spv"));
			rule = std::make_unique<CSIM::Rule1D>(rule_config);
		} else if (ca_type_1dbinary_subcmd->parsed()) {
			rule_config = std::make_shared<CSIM::RuleConfig1DBinary>(
					static_cast<std::int32_t>(cli_1dbinary_subcmd_data.range),
					cli_1dbinary_subcmd_data.pattern_match_code,
					std::make_shared<CSIM::CShader>("shaders/bin/1D_binary/comp.spv"));
			rule = std::make_unique<CSIM::Rule1D>(rule_config);
		} else {
			throw std::runtime_error("passed subcommand not supported");
		}



  std::shared_ptr<CSIM::Shader> render_shader = std::make_shared<CSIM::VFShader>(
		CSIM::shconfig::VSH_RENDER_SHADER_PATH,
		CSIM::shconfig::FSH_RENDER_SHADER_PATH
	);
  CSIM::Renderer renderer(render_shader);
	renderer.setColors(colors);

	glfwSetWindowUserPointer(window, &renderer);
	glfwSetKeyCallback(window,
	[](GLFWwindow* win, int key, int, int action, int){
		auto* renderer_ptr = static_cast<CSIM::Renderer*>(glfwGetWindowUserPointer(win));
		if(action == GLFW_PRESS) {
			constexpr float offset_value{ 15.0f };
			constexpr float scale_value{ 2.01f };
			switch(key) {
			// zoom in, zoom out keys
			case GLFW_KEY_N: renderer_ptr->updateView({0.f, 0.f},     scale_value); break;
			case GLFW_KEY_M: renderer_ptr->updateView({0.f, 0.f}, 1.f/scale_value); break;
			// move keys
			case GLFW_KEY_W: renderer_ptr->updateView({ 0.f, offset_value}, 1.f); break;
			case GLFW_KEY_S: renderer_ptr->updateView({ 0.f,-offset_value}, 1.f); break;
			case GLFW_KEY_A: renderer_ptr->updateView({-offset_value, 0.f}, 1.f); break;
			case GLFW_KEY_D: renderer_ptr->updateView({ offset_value, 0.f}, 1.f); break;
			default: break;
		  }
		}
	});

	CSIM::CellMap cell_map(render_shader, arg_map_width, arg_map_height,
											 	 static_cast<std::int32_t>(colors.size()));

	cell_map.seed(0, 0); // NOLINT

  glClearColor(.0f, .0f, .0f, 1.f); // NOLINT
	auto last_frame_time{ static_cast<float>(glfwGetTime()) };

	std::size_t counter{ 0ul };
  while(glfwWindowShouldClose(window) == 0) {
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT);

		auto frame_time = static_cast<float>(glfwGetTime());
		renderer.time_step_ = frame_time - last_frame_time;
		last_frame_time = frame_time;

		Vec2<int> win_size;
		glfwGetWindowSize(window, &win_size.x, &win_size.y);

    renderer.draw(win_size, cell_map);
		if(++counter; counter == arg_counter) { // NOLINT ~2 steps per second
			rule->step(cell_map);
			counter = 0;
		}

    glfwSwapBuffers(window);
  }

	glfwDestroyWindow(window);

	renderer.destroy();
	rule->destroy();
	cell_map.destroy();

	glfwTerminate();

  } catch(const std::exception& e) {
    spdlog::critical("{}", e.what());

    glfwDestroyWindow(window);   
    glfwTerminate();

    return 1;
  }

  return 0;
}