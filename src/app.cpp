//
// Created by reg on 8/6/22.
//
#include "app.hpp"
#include "cli_emulator/cellsim_cli_emulator.hpp"

#ifdef GL_DEBUG
#include <glad/glad.h>
#include <spdlog/spdlog.h>
#endif

#include <imgui/imgui_utils.hpp>
#include <imgui.h>
#include <rules/rule_info_window.hpp>
#include <GLFW/glfw3.h>

/// TODO add clear color to cellmap

CSIM::App::App(Window& window)
	: window_(window) {
	renderer_.setColors({{0.f, 0.f, 0.f, 1.f}, {1.f, 1.f, 1.f, 1.f}});
	window_.setScrollCallback([this](float offset){
		constexpr float scale_value{1.10f};
		if(offset > .0f) {
			renderer_.updateView({0.f, 0.f}, scale_value);
		} else {
			renderer_.updateView({0.f, 0.f}, 1.f / scale_value);
		}
	});
	window_.setCursorPosCallback([this](float prev_x, float prev_y, float x, float y){
		if(prev_x > 0 && prev_y > 0 && window_.getButtonState(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			constexpr float mv_divisor{ .1f };
			auto offset_x = mv_divisor * (x - prev_x);
			auto offset_y = mv_divisor * (prev_y - y);
			renderer_.updateView({offset_x, offset_y}, 1.f);
		}
	});

#ifdef GL_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(
			[](GLenum, GLenum, GLuint id, GLenum severity, GLsizei,
				 const GLchar *message, const void *) {
				if (severity == GL_DEBUG_SEVERITY_HIGH) {
					spdlog::error("[glad] {}, id = {}", message, id);
				} else {
					spdlog::info("[glad] {}, id = {}", message, id);
				}
			}, nullptr);
#endif
	ImGui::Init(window_.native());

	cli_emulator_.setCLI();
}

void CSIM::App::destroy() {
	ImGui::Uninit();
	renderer_.destroy();
	cell_map_.destroy();
	if(rule_) {
		rule_->destroy();
	}
	if(rule_config_) {
		rule_config_->destroy();
	}
}

void CSIM::App::updateRuleConfig(std::shared_ptr<RuleConfig> config, RuleType rule_type) {
	if(rule_config_ != nullptr)	{
		rule_config_->destroy();
	}
	rule_config_ = std::move(config);

	if(rule_ == nullptr || rule_->ruleType() != rule_type)	{
		if(rule_ != nullptr) {
			rule_->destroy();
		}
		switch(rule_type) {
		case RuleType::BASIC_1D: rule_ = std::make_shared<Rule1D>(rule_config_); break;
		case RuleType::BASIC_2D: rule_ = std::make_shared<Rule2D>(rule_config_); break;
		default: break;
		}
	} else {
		rule_->setRuleConfig(rule_config_);
	}
}

static auto hexColorToFloatColor(std::uint32_t color) {
	constexpr std::uint32_t MASK{0xFF};
	constexpr float DIVIDER{255.f};
	return CSIM::utils::Vec4<float>{
			static_cast<float>((color >> 24u)) / DIVIDER,				 // NOLINT
			static_cast<float>((color >> 16u) & MASK) / DIVIDER, // NOLINT
			static_cast<float>((color >> 8u) & MASK) / DIVIDER,	 // NOLINT
			static_cast<float>((color >> 0u) & MASK) / DIVIDER	 // NOLINT
	};
}

void CSIM::App::parseCommand() {
	if(cli_emulator_.config.cmd_set->parsed()) {
		if(cli_emulator_.config.subcmd_grid->parsed()) {
			const auto args = cli_emulator_.config.options_grid;
			if (!args.hex_color_option->empty()) {
				renderer_.setGridColor(hexColorToFloatColor(args.hex_color));
			} if (!args.toggle_option->empty()) {
				renderer_.toggleGrid();
			}
		} else if(cli_emulator_.config.subcmd_counter->parsed()) {
			step_size_ = static_cast<std::int32_t>(cli_emulator_.config.option_counter);
			frame_counter_ = 0;
		} else if(cli_emulator_.config.subcmd_cellmap->parsed()) {
			if(cli_emulator_.config.subsubcmd_cellmap_clear->parsed()) {
				cell_map_.clear();
			} else {
				const auto args = cli_emulator_.config.options_cellmap;
				cell_map_.extend(args.width, args.height, !args.preserve_contents->empty());
			}
		} else if(cli_emulator_.config.subcmd_colors->parsed()) {
			// zero state is always black
			const auto& arg_colors = cli_emulator_.config.option_colors;
			std::vector<Vec4<float>> colors(arg_colors.size());
			std::transform(arg_colors.cbegin(), arg_colors.cend(), colors.begin(), hexColorToFloatColor);

			renderer_.setColors(colors);
		} else if(cli_emulator_.config.subcmd_rule->parsed()) {
			if(cli_emulator_.config.subsubcmd_rule_1dtotalistic->parsed()) {
				auto& rule_args = cli_emulator_.config.options_1d_totalistic;
				updateRuleConfig(std::make_shared<RuleConfig1DTotalistic>(
						rule_args.range,
						!rule_args.center_active->empty(),
						rule_args.survival_conditions,
						rule_args.birth_conditions,
						std::make_shared<CShader>("shaders/bin/1D_totalistic/comp.spv")
				), RuleType::BASIC_1D);
			} else if(cli_emulator_.config.subsubcmd_rule_1dbinary->parsed()) {
				const auto& rule_args = cli_emulator_.config.options_1d_binary;
				updateRuleConfig(std::make_shared<RuleConfig1DBinary>(
						rule_args.range,
						rule_args.pattern_match_code,
						std::make_shared<CShader>("shaders/bin/1D_binary/comp.spv")
				), RuleType::BASIC_1D);
			} else if(cli_emulator_.config.subsubcmd_rule_2dcyclic->parsed()) {
				const auto& rule_args = cli_emulator_.config.options_2d_cyclic;
				updateRuleConfig(std::make_shared<RuleConfig2DCyclic>(
					rule_args.range,
					rule_args.threshold,
					!rule_args.moore->empty(),
					!rule_args.state_insensitive->empty(),
					std::make_shared<CShader>("shaders/bin/2D_cyclic/comp.spv")
				), RuleType::BASIC_2D);
			}
		} else if(cli_emulator_.config.subcmd_clear_color->parsed()) {
			renderer_.setClearColor(hexColorToFloatColor(cli_emulator_.config.option_clear_color));
		}
	} else if(cli_emulator_.config.cmd_clear->parsed()) {
		cli_emulator_.clear();
	} else if(cli_emulator_.config.cmd_seed->parsed()) {
		const auto args = cli_emulator_.config.options_seed;
		cell_map_.seed(args.x, args.y, args.range, !args.round->empty(), !args.clip->empty());
	}
}

void CSIM::App::run() {
	while(!window_.shouldClose()) {
		window_.pollEvents();

		auto frame_time = window_.getTime();
		renderer_.time_step_ = frame_time - last_frame_time_;
		last_frame_time_ = frame_time;

		const auto[width, height] = window_.getWindowSize();
		renderer_.draw({width, height}, cell_map_);

		ImGui::BeginFrameCustom();
		if(rule_ != nullptr && rule_config_ != nullptr)	{
			if(++frame_counter_; frame_counter_ == step_size_) {
				rule_->step(cell_map_, static_cast<std::int32_t>(renderer_.colorCount()));
				frame_counter_ = 0;
			}
			CSIM::drawRuleInfoWindow(rule_, rule_config_);
		}

		if (cli_emulator_.draw(window_.getKeyState(GLFW_KEY_ENTER) == GLFW_PRESS,
													 window_.getKeyState(GLFW_KEY_BACKSPACE) == GLFW_PRESS)) {
			parseCommand();
		}

		/// NOLINTBEGIN
		ImGui::Begin("Technical Info");
		ImGui::Text("window width :: %i", width);
		ImGui::Text("window height :: %i", height);
		ImGui::Text("FPS :: %f", 1.f / renderer_.time_step_);
		ImGui::End();

		ImGui::Begin("States");
		ImGui::Text("state count :: %lu", renderer_.colorCount());
		if(ImGui::BeginTable("##state_colors", 1, ImGuiTableFlags_Borders, ImVec2{-1, -1})) {
			const auto& colors = renderer_.colors();
			for(std::size_t r=0; r<renderer_.colorCount(); ++r) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("          ");
				const auto color = colors.at(r);
				const auto tile_color = ImGui::GetColorU32({color.x, color.y, color.z, color.w});
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tile_color);
			}
			ImGui::EndTable();
		}
		ImGui::End();

		ImGui::Begin("Map info");
		const auto map_res = cell_map_.resolution();
		ImGui::Text("map width :: %i", map_res.x);
		ImGui::Text("map height :: %i", map_res.y);
		ImGui::Separator();
		ImGui::Image((void*)(intptr_t)cell_map_.textureFbo().tex_id(),
								 {static_cast<float>(map_res.x), static_cast<float>(map_res.y)},
								 {0.f, 1.f}, {1.f, 0.f}, {1.f, 1.f, 1.f, 1.f}, {.2f, .2f, .411f, 1.f}); // NOLINT

		ImGui::End();
		/// NOLINTEND

		ImGui::EndFrameCustom();

		window_.swapBuffers();
	}

}
