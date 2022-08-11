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

static auto DynamicRangeValidatorUtil(std::size_t value, std::size_t min, std::size_t max) {
	if (value >= min && value < max) {
		return std::string();
	} else {
		return fmt::format("number {} isn't in [{}, {})", value, min, max);
	}
}
}

void CSIM::AppCLIEmulator::setCLI() {
	config.cmd_set = this->parser.add_subcommand("set", "command which sets values");
		config.subcmd_grid = config.cmd_set->add_subcommand("grid", "turn on/off grid and modify grid "
																																"color");
			config.options_grid.toggle_option = config.subcmd_grid->add_flag("-t,--toggle",
																																		   config.options_grid.toggle,
																																		   "toggle grid")
																																			 ->default_val(false);
			config.options_grid.hex_color_option = config.subcmd_grid
																								    ->add_option("-c,--color",
																															   config.options_grid.hex_color,
																															   "set grid color")
																		 								->check(CLI::TypeValidator<std::uint32_t>());
		config.subcmd_cellmap = config.cmd_set->add_subcommand("cellmap", "modify cellmap extent");
			config.subcmd_cellmap->add_option("-x,--width", config.options_cellmap.width,
																			  "width of the cell map")
																			  ->check(CLI::TypeValidator<std::size_t>())
																				->required();
			config.subcmd_cellmap->add_option("-y,--height", config.options_cellmap.height,
																				"height of the cell map")
																			  ->check(CLI::TypeValidator<std::size_t>())
																				->required();
			config.subcmd_cellmap->add_flag("-p,--preserve-contents",
																			config.options_cellmap.preserve_contents,
																			"if set contents of previous cell map will be preserved"
																			" starting from the upper left corner")
																			->default_val(false);
		config.subcmd_colors = config.cmd_set
																  ->add_subcommand("colors",
																							     "modify number of states through new color set");
			config.subcmd_colors->add_option("-c,--colors", config.option_colors,
																				"add history colors to cells in array of hex values (max 255)")
																				->check(CLI::VecSizeValidator(1, 255)) // NOLINT
																				->required();
		config.subcmd_rule = config.cmd_set
															  ->add_subcommand("rule",
																  							 "modify current rule or switch to another rule");
			config.subsubcmd_rule_1dbinary = config
																		   .subcmd_rule
																			 ->add_subcommand("1dbinary",
																												"1d binary is one dimensional ca rule");
				config.subsubcmd_rule_1dbinary
						 ->add_option("-r,--range", config.options_1d_binary.range,
									 				"describes range of the neighbourhood (0 ..<= 4)")
													 ->check(CLI::Range(RuleConfig1DBinary::RANGE_LIM.x,
												 										  RuleConfig1DBinary::RANGE_LIM.y))
												   ->required();
				config.subsubcmd_rule_1dbinary
						 ->add_option("-p,--patern-match-code", config.options_1d_binary.pattern_match_code,
												  "defines which patterns of size 2*<range>+1 qualify cell for "
													"birth/survival, 1 = qualifed, 0 = unqualified, goes from min to max")
													->check(CLI::BinaryNumberValidator(RuleConfig1DBinary::RANGE_LIM.x + 1,
																														 2*RuleConfig1DBinary::RANGE_LIM.y+1))
													->required();
			config.subsubcmd_rule_1dtotalistic = config
																				   .subcmd_rule
																						->add_subcommand("1dtotalistic",
																													   "1d totalistic is one dimensional ca rule");
				config.subsubcmd_rule_1dtotalistic
							 ->add_option("-r,--range", config.options_1d_totalistic.range,
									 		      "describes range of the neighbourhood (0 ..<= 10)")
														->check(CLI::Range(RuleConfig1DTotalistic::RANGE_LIM.x,
												 										   RuleConfig1DTotalistic::RANGE_LIM.y))
														->required();
				config.subsubcmd_rule_1dtotalistic
						   ->add_flag("-e,--exclude-center", config.options_1d_totalistic.center_active,
													"if set then center cell won't be taken into account")
													->default_val(false);
				config.subsubcmd_rule_1dtotalistic
						   ->add_option("-s,--survive-conditions",
												 	  config.options_1d_totalistic.survival_conditions,
														"array of sums which qualify cell for survival")
													  ->check(CLI::VecSizeValidator(RuleConfig1DTotalistic::RANGE_LIM.x + 1,
																													2*RuleConfig1DTotalistic::RANGE_LIM.y+1))
														->required();
				config.subsubcmd_rule_1dtotalistic
						   ->add_option("-b,--birth-conditions",
												    config.options_1d_totalistic.birth_conditions,
														"array of sums which qualify cell for birth")
														->check(CLI::VecSizeValidator(RuleConfig1DTotalistic::RANGE_LIM.x + 1,
																												  2*RuleConfig1DTotalistic::RANGE_LIM.y+1))
														->required();
		config.subcmd_counter = config.cmd_set->add_subcommand("counter", "set value of FPS step counter");
			config.subcmd_counter->add_option("-c,--counter", config.option_counter,
																			  "value to which to count")
																				->check(CLI::TypeValidator<std::uint32_t>())
																				->required();
	config.cmd_clear = this->parser.add_subcommand("clear", "clears CLI");
	config.cmd_seed = this->parser.add_subcommand("seed", "seed cellmap with initial state");

		config.cmd_seed->add_option("-x,--column", config.options_seed.x,
															  "x coordinate of the seed center")
										            ->check(
																[min = 0, &max = this->config.options_cellmap.width]
																(const std::string& num) {
																	return CLI::DynamicRangeValidatorUtil(std::stoul(num), min, max);
																})->required();

		config.cmd_seed->add_option("-y,--row", config.options_seed.y,
																"y coordinate of the seed center")
																->check(
																[min = 0, &max = this->config.options_cellmap.height]
																(const std::string& num) {
																	return CLI::DynamicRangeValidatorUtil(std::stoul(num), min, max);
																})->required();
		config.cmd_seed->add_option("-r,--range", config.options_seed.range,
																"range of seed area from the center")
																->check(
																[min = 0, &height = this->config.options_cellmap.height,
																 &width = this->config.options_cellmap.width]
													 			(const std::string& num) {
																	return CLI::DynamicRangeValidatorUtil(std::stoul(num), min,
																		 														 2*std::min(width, height) + 1);
																})->required();
			config.cmd_seed->add_flag("-c,--circle", config.options_seed.round,
																"switch from the square area to circle area")
																->default_val(false);
			config.cmd_seed->add_flag("-l,--clip", config.options_seed.clip,
																"clip the seed area if out of bounds if not specified out of bounds"
																"area will behave as if map has torus topology")
																->default_val(false);
	}
