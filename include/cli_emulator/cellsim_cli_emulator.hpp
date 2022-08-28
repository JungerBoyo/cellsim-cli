//
// Created by reg on 8/5/22.
//

#ifndef CELLSIM_CELLSIM_CLI_EMULATOR_HPP
#define CELLSIM_CELLSIM_CLI_EMULATOR_HPP

#include <vector>

#include <cli_emulator/cli_emulator.hpp>

namespace CSIM {

/// cellsim app specific cli emulator
struct AppCLIEmulator : public CLIEmulator {
	struct {
		/// command
		CLI::App *cmd_set;
		/// subcommand
		CLI::App *subcmd_clear_color;
		/// options
		std::uint32_t option_clear_color;
		/// subcommand
		CLI::App *subcmd_grid;
		/// options
		struct {
			CLI::Option *hex_color_option;
			std::uint32_t hex_color;
			CLI::Option *toggle_option;
		} options_grid;
		///	subcommand
		CLI::App *subcmd_cellmap;
		/// options
		struct {
			/// initial values needed for range validators in seed command
			std::size_t width{64};	// NOLINT initial cellmap extent in x
			std::size_t height{64}; // NOLINT initial cellmap extent in y
			CLI::Option *preserve_contents;
		} options_cellmap;
		/// subcommand
		CLI::App *subcmd_colors;
		/// options
		std::vector<std::uint32_t> option_colors;
		/// subcommand
		CLI::App *subcmd_rule;
		/// subsubcommand
		CLI::App *subsubcmd_rule_1dtotalistic;
		/// options
		struct {
			std::uint32_t range;
			CLI::Option *exclude_center;
			std::vector<std::size_t> survival_conditions;
			std::vector<std::size_t> birth_conditions;
		} options_1d_totalistic;
		/// subsubcommand
		CLI::App *subsubcmd_rule_1dbinary;
		/// options
		struct {
			std::uint32_t range;
			std::string pattern_match_code;
		} options_1d_binary;
		/// subsubcommand
		CLI::App *subsubcmd_rule_2dcyclic;
		/// options
		struct {
			std::int32_t range;
			std::int32_t threshold;
			CLI::Option *moore;
			CLI::Option *state_insensitive;
			CLI::Option *exclude_center;
		} options_2d_cyclic;
		/// subsubcommand
		CLI::App *subsubcmd_rule_2dlife;
		/// options
		struct {
			CLI::Option *moore;
			CLI::Option *state_insensitive;
			CLI::Option *exclude_center;
			std::vector<std::size_t> survival_conditions;
			std::vector<std::size_t> birth_conditions;
		} options_2d_life;
		/// subcommand
		CLI::App *subcmd_counter;
		/// options
		std::uint32_t option_counter{0};
		/// command
		CLI::App *cmd_clear;
		/// options
		struct {
			CLI::Option *clear_cli;
			CLI::Option *clear_map;
		} options_clear;
		/// command
		CLI::App *cmd_seed;
		struct {
			std::size_t x;
			std::size_t y;
			std::size_t range;
			CLI::Option *round;
			CLI::Option *clip;
		} options_seed;
		/// command
		CLI::App *cmd_stop;
		/// command
		CLI::App *cmd_start;
	} config;

	/**
	 * constructor of cellsim cli emulator
	 * @param title maps to label member field, will be displayed in cli window tab add ## at the
	 * front for it to not be displayed.
	 * @param description description of CLI11 app.
	 * @param prompt text that will be prompt in a CLI. '>' char will be appended.
	 */
	AppCLIEmulator(std::string_view title, std::string_view description, std::string_view prompt);

	/**
	 * method in which the cli subcommands and options from field 'config' are set in cli parser
	 */
	void setCLI() override;
};

} // namespace CSIM

#endif // CELLSIM_CELLSIM_CLI_EMULATOR_HPP
