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
		CLI::App* cmd_set;
			///	subcommand
			CLI::App* subcmd_cellmap;
				/// options
				struct {
					std::size_t width;
					std::size_t height;
					std::vector<std::uint32_t> colors;
				} options_cellmap;
			/// subcommand
			CLI::App* subcmd_rule;
				/// subsubcommand
				CLI::App* subsubcmd_rule_1dtotalistic;
					/// options
					struct {
						std::uint32_t range;
						bool center_active;
						std::vector<std::size_t> survival_conditions;
						std::vector<std::size_t> birth_conditions;
					} options_1d_totalistic;
				/// subsubcommand
				CLI::App* subsubcmd_rule_1dbinary;
					/// options
					struct {
						std::uint32_t range;
						std::string pattern_match_code;
					} options_1d_binary;
			/// subcommand
			CLI::App* subcmd_counter;
			/// options
				std::size_t arg_counter{0};
		/// command
		CLI::App* cmd_clear;
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

}


#endif // CELLSIM_CELLSIM_CLI_EMULATOR_HPP
