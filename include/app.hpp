//
// Created by reg on 8/6/22.
//

#ifndef CELLSIM_APP_HPP
#define CELLSIM_APP_HPP

#include <cli_emulator/cellsim_cli_emulator.hpp>
#include <renderer/renderer.hpp>
#include <rules/rule.hpp>
#include <rules/rule_config.hpp>
#include <shaders/shaders.hpp>
#include <window/window.hpp>

namespace CSIM {

struct App {
	Window &window_;

	std::shared_ptr<Shader> render_shader_{};

	Renderer renderer_{
			{std::get<0>(window_.getWindowSize()), std::get<1>(window_.getWindowSize())},
			std::make_shared<VFShader>("shaders/bin/render_shader/vert.spv", "shaders/bin/render_shader/frag.spv"),
			std::make_shared<VFShader>("shaders/bin/grid_shader/vert.spv", "shaders/bin/grid_shader/frag.spv")};
	CellMap cell_map_{64, 64}; // NOLINT
	AppCLIEmulator cli_emulator_{"cellsim cli", "cellular automata cli based simulation app",
															 "cellsim"};
	std::shared_ptr<Rule> rule_{nullptr};
	std::shared_ptr<RuleConfig> rule_config_{nullptr};

	std::int32_t step_size_{30}; // NOLINT
	std::int32_t frame_counter_{0};
	float last_frame_time_{0.f};
	bool simulation_stopped_{false};
	bool is_viewport_win_focused_{false};

	explicit App(Window &window);

	void updateRuleConfig(std::shared_ptr<RuleConfig> config, RuleType rule_type);
	void parseCommand();
	void run();

	void destroy();
};

} // namespace CSIM

#endif // CELLSIM_APP_HPP
