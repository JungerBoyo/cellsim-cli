//
// Created by reg on 8/6/22.
//
#include "app.hpp"
#include "cli_emulator/cellsim_cli_emulator.hpp"

#ifdef GL_DEBUG
#include <glad/glad.h>
#include <spdlog/spdlog.h>
#endif

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui/imgui_utils.hpp>
#include <rules/rule_info_window.hpp>

/// TODO add clear color to cellmap

CSIM::App::App(Window &window) : window_(window) {
	renderer_.setColors({{0.f, 0.f, 0.f, 1.f}, {1.f, 1.f, 1.f, 1.f}});
	window_.setScrollCallback([this](float offset) {
		if (!is_viewport_win_focused_) {
			return;
		}
		constexpr float scale_value{1.10f};
		if (offset > .0f) {
			renderer_.updateView({0.f, 0.f}, scale_value);
		} else {
			renderer_.updateView({0.f, 0.f}, 1.f / scale_value);
		}
	});
	window_.setCursorPosCallback([this](float prev_x, float prev_y, float x, float y) {
		if (!is_viewport_win_focused_) {
			return;
		}
		if (prev_x > 0 && prev_y > 0 && window_.getButtonState(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			constexpr float mv_divisor{.1f};
			auto offset_x = mv_divisor * (x - prev_x);
			auto offset_y = mv_divisor * (prev_y - y);
			renderer_.updateView({offset_x, offset_y}, 1.f);
		}
	});

#ifdef GL_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(
			[](GLenum, GLenum, GLuint id, GLenum severity, GLsizei, const GLchar *message, const void *) {
				if (severity == GL_DEBUG_SEVERITY_HIGH) {
					spdlog::error("[glad] {}, id = {}", message, id);
				} else {
					spdlog::info("[glad] {}, id = {}", message, id);
				}
			},
			nullptr);
#endif
	ImGui::Init(window_.native());

	cli_emulator_.setCLI();
}

void CSIM::App::destroy() {
	ImGui::Uninit();
	renderer_.destroy();
	cell_map_.destroy();
	if (rule_) {
		rule_->destroy();
	}
	if (rule_config_) {
		rule_config_->destroy();
	}
}

void CSIM::App::updateRuleConfig(std::shared_ptr<RuleConfig> config, RuleType rule_type) {
	if (rule_config_ != nullptr) {
		rule_config_->destroy();
	}
	rule_config_ = std::move(config);

	if (rule_ == nullptr || rule_->ruleType() != rule_type) {
		if (rule_ != nullptr) {
			rule_->destroy();
		}
		switch (rule_type) {
		case RuleType::BASIC_1D:
			rule_ = std::make_shared<Rule1D>(rule_config_);
			break;
		case RuleType::BASIC_2D:
			rule_ = std::make_shared<Rule2D>(rule_config_);
			break;
		default:
			break;
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
	if (cli_emulator_.config.cmd_set->parsed()) {
		if (cli_emulator_.config.subcmd_grid->parsed()) {
			const auto args = cli_emulator_.config.options_grid;
			if (!args.hex_color_option->empty()) {
				renderer_.setGridColor(hexColorToFloatColor(args.hex_color));
			}
			if (!args.toggle_option->empty()) {
				renderer_.toggleGrid();
			}
		} else if (cli_emulator_.config.subcmd_counter->parsed()) {
			step_size_ = static_cast<std::int32_t>(cli_emulator_.config.option_counter);
			frame_counter_ = 0;
		} else if (cli_emulator_.config.subcmd_cellmap->parsed()) {
			const auto args = cli_emulator_.config.options_cellmap;
			cell_map_.extend(args.width, args.height, !args.preserve_contents->empty());
		} else if (cli_emulator_.config.subcmd_colors->parsed()) {
			// zero state is always black
			const auto &arg_colors = cli_emulator_.config.option_colors;
			std::vector<Vec4<float>> colors(arg_colors.size());
			std::transform(arg_colors.cbegin(), arg_colors.cend(), colors.begin(), hexColorToFloatColor);

			renderer_.setColors(colors);
		} else if (cli_emulator_.config.subcmd_rule->parsed()) {
			if (cli_emulator_.config.subsubcmd_rule_1dtotalistic->parsed()) {
				const auto &rule_args = cli_emulator_.config.options_1d_totalistic;
				updateRuleConfig(std::make_shared<RuleConfig1DTotalistic>(
														 rule_args.range, rule_args.exclude_center->empty(),
														 rule_args.survival_conditions, rule_args.birth_conditions,
														 std::make_shared<CShader>("shaders/bin/1D_totalistic/comp.spv")),
												 RuleType::BASIC_1D);
			} else if (cli_emulator_.config.subsubcmd_rule_1dbinary->parsed()) {
				const auto &rule_args = cli_emulator_.config.options_1d_binary;
				updateRuleConfig(std::make_shared<RuleConfig1DBinary>(
														 rule_args.range, rule_args.pattern_match_code,
														 std::make_shared<CShader>("shaders/bin/1D_binary/comp.spv")),
												 RuleType::BASIC_1D);
			} else if (cli_emulator_.config.subsubcmd_rule_2dcyclic->parsed()) {
				const auto &rule_args = cli_emulator_.config.options_2d_cyclic;
				updateRuleConfig(std::make_shared<RuleConfig2DCyclic>(
														 rule_args.range, rule_args.threshold, !rule_args.moore->empty(),
														 !rule_args.state_insensitive->empty(),
														 rule_args.exclude_center->empty(),
														 std::make_shared<CShader>("shaders/bin/2D_cyclic/comp.spv")),
												 RuleType::BASIC_2D);
			} else if (cli_emulator_.config.subsubcmd_rule_2dlife->parsed()) {
				const auto &rule_args = cli_emulator_.config.options_2d_life;
				updateRuleConfig(std::make_shared<RuleConfig2DLife>(
														 !rule_args.moore->empty(), !rule_args.state_insensitive->empty(),
														 rule_args.exclude_center->empty(), rule_args.survival_conditions,
														 rule_args.birth_conditions,
														 std::make_shared<CShader>("shaders/bin/2D_life/comp.spv")),
												 RuleType::BASIC_2D);
			}
		} else if (cli_emulator_.config.subcmd_clear_color->parsed()) {
			renderer_.setClearColor(hexColorToFloatColor(cli_emulator_.config.option_clear_color));
		}
	} else if (cli_emulator_.config.cmd_clear->parsed()) {
		if (!cli_emulator_.config.options_clear.clear_cli->empty()) {
			cli_emulator_.clear();
		}
		if (!cli_emulator_.config.options_clear.clear_map->empty()) {
			cell_map_.clear();
		}
	} else if (cli_emulator_.config.cmd_seed->parsed()) {
		const auto args = cli_emulator_.config.options_seed;
		cell_map_.seed(args.x, args.y, args.range, !args.round->empty(), !args.clip->empty());
	} else if (cli_emulator_.config.cmd_stop->parsed()) {
		simulation_stopped_ = true;
	} else if (cli_emulator_.config.cmd_start->parsed()) {
		simulation_stopped_ = false;
	}
}

struct SeedingUIInfo {
	bool round{false};
	bool clip{false};
	int range{0};
};

static void HelpMarker(const char* desc) {
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.F);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

static void makeBorder(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) {
	auto *draw_list = ImGui::GetWindowDrawList();
	draw_list->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(r, g, b, a));
}

void CSIM::App::run() {
	while (!window_.shouldClose()) {
		window_.pollEvents();

		auto frame_time = window_.getTime();
		renderer_.time_step_ = frame_time - last_frame_time_;
		last_frame_time_ = frame_time;

		static ImVec2 viewport_win_size{120.F, 90.F};

		renderer_.draw(
			{
				static_cast<int>(viewport_win_size.x),
				static_cast<int>(viewport_win_size.y)
			},
			cell_map_
		);

		ImGui::BeginFrameCustom();
		if (rule_ != nullptr && rule_config_ != nullptr) {
			if (!simulation_stopped_) {
				if (++frame_counter_; frame_counter_ == step_size_) {
					rule_->step(cell_map_, static_cast<std::int32_t>(renderer_.colorCount()));
					frame_counter_ = 0;
				}
			}
			CSIM::drawRuleInfoWindow(rule_, rule_config_);
		}

		/// NOLINTBEGIN
		/// Setting up dockspace
		const auto* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		static constexpr auto docking_window_flags {
				ImGuiWindowFlags_NoMove                 |
				ImGuiWindowFlags_NoCollapse             |
				ImGuiWindowFlags_NoResize               |
				ImGuiWindowFlags_NoDecoration           |
				ImGuiWindowFlags_NoBringToFrontOnFocus  |
				ImGuiWindowFlags_NoTitleBar
		};
		ImGui::Begin("##docking-space", nullptr, docking_window_flags);

		auto& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("##Dockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(.0F, .0F), 0);
		}

		if (cli_emulator_.draw(window_.getKeyState(GLFW_KEY_ENTER) == GLFW_PRESS,
													 window_.getKeyState(GLFW_KEY_BACKSPACE) == GLFW_PRESS)) {
			parseCommand();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, .0F);
		static constexpr auto viewport_window_flags {
				ImGuiWindowFlags_NoScrollbar          	|
				ImGuiWindowFlags_NoScrollWithMouse
		};
		ImVec2 viewport_p0{0.F, 0.F};
		ImVec2 viewport_p1{0.F, 0.F};
		if (ImGui::Begin("##viewport", nullptr, viewport_window_flags)) {
			viewport_p0 = ImGui::GetWindowPos();
			const auto cursor_pos = ImGui::GetCursorPos();
			viewport_p0.x += cursor_pos.x;
			viewport_p0.y += cursor_pos.y;

			viewport_win_size = ImGui::GetWindowSize();
			viewport_win_size.y -= 40.F;
			viewport_p1 = {
					viewport_p0.x + viewport_win_size.x,
					viewport_p0.y + viewport_win_size.y
			};

			is_viewport_win_focused_ = ImGui::IsWindowFocused();

			ImGui::Image(
				(void *)(intptr_t)renderer_.main_fbo_.tex_id(),
				viewport_win_size,
				{0.F, 1.F},
				{1.F, 0.F}
			);
			ImGui::End(); // viewport
		}

		ImGui::PopStyleVar();

		ImGui::Begin("##lhs-main-window");
		ImGui::LabelText("##technical-info", "Technical info"); makeBorder(255, 255, 0, 255);
		ImGui::Text("window width :: %i", static_cast<int>(viewport_win_size.x));
		ImGui::Text("window height :: %i", static_cast<int>(viewport_win_size.y));
		ImGui::Text("FPS :: %i", static_cast<std::int32_t>(1.f / renderer_.time_step_));
		ImGui::Text("Counter :: %i", frame_counter_);

		ImGui::Separator();
		ImGui::Separator();

		static SeedingUIInfo seeding_ui_info{};
		ImGui::LabelText("##seeding-options", "Seeding options"); makeBorder(255, 255, 0, 255);
		ImGui::Checkbox("Round seed region", &seeding_ui_info.round);
		ImGui::Checkbox("Clip seed region", &seeding_ui_info.clip);
		ImGui::SliderInt("##seed-region-size", &seeding_ui_info.range, 0, 30); // NOLINT
		ImGui::SameLine();
		HelpMarker("Seed region size");
		if (ImGui::Button("Clear map to initial state")) {
			cell_map_.clear();
		}

		ImGui::Separator();
		ImGui::Separator();
		ImGui::LabelText("##simulation-options", "Simulation options"); makeBorder(255, 255, 0, 255);
		if (ImGui::Button("Toggle simulation start/stop")) {
			simulation_stopped_ = !simulation_stopped_;
		}
		if (ImGui::SliderInt("##single-simulation-step-delay", &step_size_, 0, 60)) {
			frame_counter_ = 0;
		}
		ImGui::SameLine();
		HelpMarker("Single simulation step delay");

		ImGui::Separator();
		ImGui::Separator();

		ImGui::LabelText("##cosmetic-options", "Cosmetic options"); makeBorder(255, 255, 0, 255);

		ImGui::Checkbox("Grid on/off", &renderer_.grid_on_);
		ImGui::ColorPicker4("Grid color", &renderer_.view_config_.outline_color.x);

		ImGui::Separator();

		ImGui::ColorPicker4("Clear color", &renderer_.clear_color_.x);

		ImGui::Separator();
		ImGui::Separator();
		ImGui::End(); // lhs main window

		ImGui::Begin("##rhs-main-window");
		ImGui::LabelText("##cellmap-info", "Cells map info"); makeBorder(255, 255, 0, 255);

		const auto rhs_main_window_size = ImGui::GetWindowSize();
		const auto map_res = cell_map_.resolution();
		ImGui::Text("map width :: %i", map_res.x);
		ImGui::Text("map height :: %i", map_res.y);
		ImGui::Separator();
		auto im_map_res = ImVec2{
				static_cast<float>(map_res.x),
				static_cast<float>(map_res.y)
		};
		if (im_map_res.x > rhs_main_window_size.x)	{
			const auto scale_factor = rhs_main_window_size.x / im_map_res.x;
			im_map_res.x *= scale_factor;
			im_map_res.y *= scale_factor;
		}
		ImGui::SetCursorPos({
			(rhs_main_window_size.x - im_map_res.x) * 0.5F,
			ImGui::GetCursorPosY()
		});
		ImGui::Image((void *)(intptr_t)cell_map_.textureFbo().tex_id(),
								 im_map_res, {0.f, 1.f},
								 {1.f, 0.f}, {1.f, 1.f, 1.f, 1.f}, {.2f, .2f, .411f, 1.f}); // NOLINT
		ImGui::SetCursorPos({
			(rhs_main_window_size.x - im_map_res.x) * 0.5F,
			ImGui::GetCursorPosY()
		});
		if (ImGui::Button("Save", {im_map_res.x + 2.F, 25.F})) {
			ImGui::OpenPopup("save_img_popup");
		}
		static bool open_error_save_img_popup{ false };
		if (ImGui::BeginPopup("save_img_popup")) {
			static std::string path;
			path.resize(512);

			ImGui::InputText("##file_path", path.data(), path.capacity());
			if (ImGui::Button("Ok")) {
				ImGui::CloseCurrentPopup();
				const auto fs_path = std::filesystem::path(
					path.cbegin(),
					std::next(path.cbegin(), std::strlen(path.data()))
				);
				if (!cell_map_.saveTextureToFile(fs_path)) {
					open_error_save_img_popup = true;
				}
				path.clear();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				path.clear();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		if (open_error_save_img_popup)	{
			ImGui::OpenPopup("error_save_img_popup");
			open_error_save_img_popup = false;
		}
		if (ImGui::BeginPopup("error_save_img_popup")) {
			ImGui::Text("You passed wrong path, already existing filename, or file extension wasn't .png");
			if (ImGui::Button("Ok")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}


		ImGui::Separator();
		ImGui::Separator();
		ImGui::LabelText("##cell-state-colors", "Cell state colors"); makeBorder(255, 255, 0, 255);
		ImGui::LabelText("##state-table-info", "Current state table");
		ImGui::Text("state count :: %lu", renderer_.colorCount());
		if (ImGui::BeginTable("##state-colors", 1, ImGuiTableFlags_Borders, ImVec2{-1, 112.F})) {
			const auto &colors = renderer_.colors();
			for (std::size_t r = 0; r < renderer_.colorCount(); ++r) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("          ");
				const auto color = colors.at(r);
				const auto tile_color = ImGui::GetColorU32({color.x, color.y, color.z, color.w});
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tile_color);
			}
			ImGui::EndTable();
		}
		static std::vector<Vec4<float>> tmp_colors;
		if (ImGui::Button("Update current states with new states")) {
			renderer_.setColors(tmp_colors);
		}
		ImGui::Separator();
		static Vec4<float> tmp_color{ 0.F, 0.F, 0.F, 1.F };
		ImGui::LabelText("##tmp-state-table-info", "New state table");
		if (ImGui::BeginTable("##tmp-state-colors", 1, ImGuiTableFlags_Borders, ImVec2{-1, 112.F})) {
			for (std::size_t r = 0; r < tmp_colors.size(); ++r) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("          ");
				const auto color = tmp_colors.at(r);
				const auto tile_color = ImGui::GetColorU32({color.x, color.y, color.z, color.w});
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tile_color);
			}
			ImGui::EndTable();
		}
		ImGui::LabelText("##tmp-state-pick-color", "New state color picker");
		ImGui::ColorPicker4("##tmp-state-table-color-picker", &tmp_color.x);
		ImGui::Separator();
		if (ImGui::Button("Push state color")) {
			tmp_colors.push_back(tmp_color);
		}
		ImGui::SameLine();
		if (ImGui::Button("Pop state color")) {
			tmp_colors.pop_back();
		}
		ImGui::Separator();
		ImGui::Separator();

		ImGui::End(); // rhs main window

		ImGui::End(); // docking space
		/// NOLINTEND

		ImGui::EndFrameCustom();

		/// seeding with mouse button left
		if (window_.getKeyState(GLFW_KEY_S) == GLFW_PRESS) {
			if (window_.getButtonState(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
				const auto[mouse_pos_x, mouse_pos_y] = window_.getMousePos();
				if (mouse_pos_x > viewport_p0.x && mouse_pos_x < viewport_p1.x &&
						mouse_pos_y > viewport_p0.y && mouse_pos_y < viewport_p1.y) {
					// first transform from screen coordinates to viewport coordinates
					const auto mouse_pos_vp_x = mouse_pos_x - viewport_p0.x;
					const auto mouse_pos_vp_y = mouse_pos_y - viewport_p0.y;
					// next transform from viewport coordinates to grid coordinates
					const auto view_config = renderer_.view_config_;
					const auto grid_p0 = ImVec2{
						(
							(.5F * view_config.aspect_ratio * view_config.offset.x + .5F) -
							(view_config.scale * view_config.aspect_ratio * .25F)
						) * viewport_win_size.x,
						(
							.5F * -view_config.offset.y + .5F - view_config.scale * .25F
						) * viewport_win_size.y
					};
					const auto grid_cell_size = ImVec2{
							view_config.aspect_ratio * view_config.scale * .5F * viewport_win_size.x,
							view_config.scale * .5F * viewport_win_size.y,
					};
					const auto grid_p1 = ImVec2{
						grid_p0.x + grid_cell_size.x * static_cast<float>(cell_map_.width_),
						grid_p0.y + grid_cell_size.y * static_cast<float>(cell_map_.height_)
					};
					if (mouse_pos_vp_x > grid_p0.x && mouse_pos_vp_x < grid_p1.x &&
							mouse_pos_vp_y > grid_p0.y && mouse_pos_vp_y < grid_p1.y) {
						const auto mouse_pos_grid_x = mouse_pos_vp_x - grid_p0.x;
						const auto mouse_pos_grid_y = mouse_pos_vp_y - grid_p0.y;
						const auto cell_x = static_cast<std::size_t>(mouse_pos_grid_x / grid_cell_size.x);
						const auto cell_y = static_cast<std::size_t>(mouse_pos_grid_y / grid_cell_size.y);

						cell_map_.seed(
							cell_x,
							cell_y,
							static_cast<std::size_t>(seeding_ui_info.range),
							seeding_ui_info.round,
							seeding_ui_info.clip
						);
					}
				}
			}
		}

		window_.swapBuffers();
	}
}
