//
// Created by reg on 8/5/22.
//
#include "cli_emulator/cellsim_cli_emulator.hpp"

#include "shconfig.hpp"
#include <rules/rule_config.hpp>

#include <fmt/format.h>

CSIM::AppCLIEmulator::AppCLIEmulator(std::string_view title, std::string_view description,
																		 std::string_view prompt)
	: CLIEmulator(title, description, prompt) {}

namespace CLI {
/**
 * checks if size of the vector is in range [min_size, max_size]
 */
struct VecSizeValidator : public CLI::Validator {
	VecSizeValidator(std::size_t min_size, std::size_t max_size) {
		name_ = "vec size validator";
		func_ = [min_size, max_size](const std::string &vec) {
			auto size =
					static_cast<std::size_t>(std::count(vec.begin(), vec.end(), ' '));
			if (vec.back() == ' ' || (size == 0 && !vec.empty())) {
				++size;
			}
			if (size < min_size || size > max_size) {
				return fmt::format("vector size {} isn't contained in {}, {}", size,
													 min_size, max_size);
			} else {
				return std::string();
			}
		};
	}
};

/**
 * checks if passed argument have only 0/1 and is contained in range [min_size, max_size]
 */
struct BinaryNumberValidator : public CLI::Validator {
	BinaryNumberValidator(std::size_t min_length, std::size_t max_length) {
		name_ = "binary number validator";
		func_ = [min_length, max_length](const std::string& num) {
			if (num.length() >= min_length && num.length() <= max_length) {
				for(const char c : num)	{
					if(c != '0' && c != '1') {
						return fmt::format("string {} contains char that isn't 0 or 1", num);
					}
				}
				return std::string();
			} else {
				return fmt::format("binary num length {} isn't between {} and {}", num, min_length,
													 max_length);
			}
		};
	}
};

}

void CSIM::AppCLIEmulator::setCLI() {
	config.cmd_set = this->parser.add_subcommand("set", "command which sets values");
		config.subcmd_cellmap = config.cmd_set->add_subcommand("cellmap");
			config.subcmd_cellmap->add_option("--width", config.options_cellmap.width,
																			  "width of the cell map")
																			  ->check(CLI::TypeValidator<std::size_t>());
			config.subcmd_cellmap->add_option("--height", config.options_cellmap.height,
																				"height of the cell map")
																			  ->check(CLI::TypeValidator<std::size_t>());
			config.subcmd_cellmap->add_option("--colors", config.options_cellmap.colors,
																				"add history colors to cells in array of hex values (max 255)")
																				->check(CLI::VecSizeValidator(1, 255)); // NOLINT
		config.subcmd_rule = config.cmd_set->add_subcommand("rule");
			config.subsubcmd_rule_1dbinary = config
																		   .subcmd_rule
																			 ->add_subcommand("1dbinary",
																												"h1d binary is one dimensional ca rule");
				config.subsubcmd_rule_1dbinary
						 ->add_option("-r,--range", config.options_1d_binary.range,
									 				"describes range of the neighbourhood (0 ..<= 4)")
													 ->check(CLI::Range(RuleConfig1DBinary::RANGE_LIM.x,
												 										  RuleConfig1DBinary::RANGE_LIM.y));
				config.subsubcmd_rule_1dbinary
						 ->add_option("-p,--patern-match-code", config.options_1d_binary.pattern_match_code,
												  "defines which patterns of size 2*<range>+1 qualify cell for "
													"birth/survival, 1 = qualifed, 0 = unqualified, goes from min to max")
													->check(CLI::BinaryNumberValidator(RuleConfig1DBinary::RANGE_LIM.x + 1,
																														 2*RuleConfig1DBinary::RANGE_LIM.y+1));
			config.subsubcmd_rule_1dtotalistic = config
																				   .subcmd_rule
																						->add_subcommand("1dtotalistic",
																													   "1d totalistic is one dimensional ca rule");
				config.subsubcmd_rule_1dtotalistic
							 ->add_option("-r,--range", config.options_1d_totalistic.range,
									 		      "describes range of the neighbourhood (0 ..<= 10)")
														->check(CLI::Range(RuleConfig1DTotalistic::RANGE_LIM.x,
												 										   RuleConfig1DTotalistic::RANGE_LIM.y));
				config.subsubcmd_rule_1dtotalistic
						   ->add_flag("-e,--exclude-center", config.options_1d_totalistic.center_active,
													"if set then center cell won't be taken into account");
				config.subsubcmd_rule_1dtotalistic
						   ->add_option("-s,--survive-conditions",
												 	  config.options_1d_totalistic.survival_conditions,
														"array of sums which qualify cell for survival")
													  ->check(CLI::VecSizeValidator(RuleConfig1DTotalistic::RANGE_LIM.x + 1,
																													2*RuleConfig1DTotalistic::RANGE_LIM.y+1));
				config.subsubcmd_rule_1dtotalistic
						   ->add_option("-b,--birth-conditions",
												    config.options_1d_totalistic.birth_conditions,
														"array of sums which qualify cell for birth")
														->check(CLI::VecSizeValidator(RuleConfig1DTotalistic::RANGE_LIM.x + 1,
																												  2*RuleConfig1DTotalistic::RANGE_LIM.y+1));
}
