//
// Created by reg on 8/6/22.
//

#ifndef CELLSIM_APP_HPP
#define CELLSIM_APP_HPP

#include <window/window.hpp>
#include <shaders/shaders.hpp>
#include <shconfig.hpp>
#include <renderer/renderer.hpp>
#include <cli_emulator/cellsim_cli_emulator.hpp>
#include <rules/rule.hpp>
#include <rules/rule_config.hpp>

namespace CSIM {

struct App {
	Window& window_;

	std::shared_ptr<Shader> render_shader_{};

	Renderer renderer_{
			std::make_shared<VFShader>(shconfig::VSH_RENDER_SHADER_PATH, shconfig::FSH_RENDER_SHADER_PATH),
			std::make_shared<VFShader>(shconfig::VSH_GRID_SHADER_PATH, shconfig::FSH_GRID_SHADER_PATH)
	};
	CellMap cell_map_{ 64, 64 }; // NOLINT
	AppCLIEmulator cli_emulator_{"cellsim cli",
															 "cellular automata cli based simulation app", "cellsim"};
	std::shared_ptr<Rule> rule_{ nullptr };
	std::shared_ptr<RuleConfig> rule_config_{ nullptr };


	std::int32_t step_size_{ 30 }; // NOLINT
	std::int32_t frame_counter_{ 0 };
	float last_frame_time_{ 0.f };

	explicit App(Window& window);

	void updateRuleConfig(std::shared_ptr<RuleConfig> config, RuleType rule_type);
	void parseCommand();
	void run();

	void destroy();
};

}

#endif // CELLSIM_APP_HPP