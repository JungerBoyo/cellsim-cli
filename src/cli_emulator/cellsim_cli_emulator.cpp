//
// Created by reg on 8/5/22.
//
#include "cli_emulator/cellsim_cli_emulator.hpp"

#include <project_config/config.hpp>
#include <rules/rule_config.hpp>

#include <fmt/format.h>

CSIM::AppCLIEmulator::AppCLIEmulator(std::string_view title, std::string_view description,
																		 std::string_view prompt)
		: CLIEmulator(title, description, prompt) {
	this->parser.set_version_flag("-v,--version",
																fmt::format("{}, {}", title, cmake::config::project_version));
}

namespace CLI {
/**
 * checks if size of the vector is in range [min_size, max_size]

struct VecSizeValidator : public CLI::Validator {
	explicit VecSizeValidator(int max_size) {
		name_ = "vec size validator";
		func_ = [this, max_size]([[maybe_unused]] const std::string& value) {
			if(this->get_application_index() > max_size) {

			}

			const auto splitted = split_func(vec, token);
			if(splitted.size() < min_size || splitted.size() > max_size) {
				return fmt::format("vector size {} isn't contained in {}, {}", splitted.size(),
													 min_size, max_size);
			} else {
				return std::string();
			}
		};
	}
};
*/
/**
 * checks if passed argument have only 0/1 and is contained in range [min_size, max_size]
 */
struct BinaryNumberValidator : public CLI::Validator {
	BinaryNumberValidator(std::size_t min_length, std::size_t max_length) {
		name_ = "binary number validator";
		func_ = [min_length, max_length](const std::string &num) {
			if (num.length() >= min_length && num.length() <= max_length) {
				for (const char c : num) {
					if (c != '0' && c != '1') {
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
} // namespace CLI

void CSIM::AppCLIEmulator::setCLI() {
	config.cmd_set = this->parser.add_subcommand("set", "command which sets values");
	config.subcmd_clear_color = config.cmd_set->add_subcommand("clearcolor", "change color of the "
																																					 "background");
	config.subcmd_clear_color
			->add_option("-c,--color", config.option_clear_color, "new background color in hex")
			->check(CLI::TypeValidator<std::uint32_t>())
			->required();
	config.subcmd_grid = config.cmd_set->add_subcommand("grid", "turn on/off grid and modify grid "
																															"color");
	config.options_grid.toggle_option = config.subcmd_grid->add_flag("-t,--toggle", "toggle grid");
	config.options_grid.hex_color_option =
			config.subcmd_grid->add_option("-c,--color", config.options_grid.hex_color, "set grid color")
					->check(CLI::TypeValidator<std::uint32_t>());
	config.subcmd_cellmap = config.cmd_set->add_subcommand("cellmap", "modify cellmap extent");
	config.subcmd_cellmap
			->add_option("-x,--width", config.options_cellmap.width, "width of the cell map")
			->check(CLI::TypeValidator<std::size_t>())
			->required();
	config.subcmd_cellmap
			->add_option("-y,--height", config.options_cellmap.height, "height of the cell map")
			->check(CLI::TypeValidator<std::size_t>())
			->required();
	config.options_cellmap.preserve_contents = config.subcmd_cellmap->add_flag(
			"-p,--preserve-contents", "if set contents of previous cell map will be preserved"
																" starting from the upper left corner");
	config.subcmd_colors =
			config.cmd_set->add_subcommand("colors", "modify number of states through new color set");
	config.subcmd_colors
			->add_option("-c,--colors", config.option_colors,
									 "add history colors to cells in array of hex values (max 256)")
			->required();
	config.subcmd_rule =
			config.cmd_set->add_subcommand("rule", "modify current rule or switch to another rule");
	config.subsubcmd_rule_1dbinary =
			config.subcmd_rule->add_subcommand("1dbinary", "1d binary is one dimensional ca rule");
	config.subsubcmd_rule_1dbinary
			->add_option("-r,--range", config.options_1d_binary.range,
									 "describes range of the neighbourhood (0 ..<= 4)")
			->check(CLI::Range(RuleConfig1DBinary::RANGE_LIM.x, RuleConfig1DBinary::RANGE_LIM.y))
			->required();
	config.subsubcmd_rule_1dbinary
			->add_option("-p,--patern-match-code", config.options_1d_binary.pattern_match_code,
									 "defines which patterns of size 2*<range>+1 qualify cell for "
									 "birth/survival, 1 = qualifed, 0 = unqualified, goes from min to max")
			->check(CLI::BinaryNumberValidator(RuleConfig1DBinary::RANGE_LIM.x + 1,
																				 2 * RuleConfig1DBinary::RANGE_LIM.y + 1))
			->required();
	config.subsubcmd_rule_1dtotalistic = config.subcmd_rule->add_subcommand(
			"1dtotalistic", "1d totalistic is one dimensional ca rule");
	config.subsubcmd_rule_1dtotalistic
			->add_option("-r,--range", config.options_1d_totalistic.range,
									 "describes range of the neighbourhood (0 ..<= 10)")
			->check(CLI::Range(RuleConfig1DTotalistic::RANGE_LIM.x, RuleConfig1DTotalistic::RANGE_LIM.y))
			->required();
	config.options_1d_totalistic.exclude_center = config.subsubcmd_rule_1dtotalistic->add_flag(
			"-e,--exclude-center", "if set then center cell won't be taken into account");
	config.subsubcmd_rule_1dtotalistic->add_option("-s,--survive-conditions",
																								 config.options_1d_totalistic.survival_conditions,
																								 "array of sums which qualify cell for survival");
	config.subsubcmd_rule_1dtotalistic->add_option("-b,--birth-conditions",
																								 config.options_1d_totalistic.birth_conditions,
																								 "array of sums which qualify cell for birth");
	config.subsubcmd_rule_2dcyclic =
			config.subcmd_rule->add_subcommand("2dcyclic", "2d cyclic is"
																										 " 2 dimensional ca rule");
	config.subsubcmd_rule_2dcyclic
			->add_option("-r,--range", config.options_2d_cyclic.range,
									 "describes range of the neighbourhood (0 ..<= 10)")
			->check(CLI::Range(RuleConfig2DCyclic::RANGE_LIM.x, RuleConfig2DCyclic::RANGE_LIM.y))
			->required();
	config.subsubcmd_rule_2dcyclic
			->add_option("-t,--threshold", config.options_2d_cyclic.threshold,
									 "threshold which divide sums on cells qualifying to"
									 " be born/survive and to die")
			->check(CLI::Range(RuleConfig2DCyclic::SUM_LIM.x, RuleConfig2DCyclic::SUM_LIM.y))
			->required();
	config.options_2d_cyclic.moore = config.subsubcmd_rule_2dcyclic->add_flag(
			"-m,--moore", "if set then kernel will be moore type otherwise neumann type");
	config.options_2d_cyclic.state_insensitive = config.subsubcmd_rule_2dcyclic->add_flag(
			"-s,--state-insensitive",
			"if set then all cells with state higher than 0 will be accumulated "
			"otherwise only cells with next state after processed cell's state "
			"will be accumulated");
	config.options_2d_cyclic.exclude_center = config.subsubcmd_rule_2dcyclic->add_flag(
			"-e, --exclude-center", "if set than center/processed cell's state won't be"
															" included in computation");

	config.subsubcmd_rule_2dlife = config.subcmd_rule->add_subcommand("2dlife", "2d life is 2 "
																																							"dimensional CA "
																																							"rule");
	config.options_2d_life.moore = config.subsubcmd_rule_2dlife->add_flag(
			"-m,--moore", "if set then kernel will be moore type otherwise neumann type");
	config.options_2d_life.state_insensitive = config.subsubcmd_rule_2dlife->add_flag(
			"-S,--state-insensitive",
			"if set then all cells with state higher than 0 will be accumulated "
			"otherwise only cells with next state after processed cell's state "
			"will be accumulated");
	config.options_2d_life.exclude_center = config.subsubcmd_rule_2dlife->add_flag(
			"-e, --exclude-center", "if set than center/processed cell's state won't be"
															" included in computation");
	config.subsubcmd_rule_2dlife->add_option("-s,--survive-conditions",
																					 config.options_2d_life.survival_conditions,
																					 "array of sums which qualify cell for survival");
	config.subsubcmd_rule_2dlife->add_option("-b,--birth-conditions",
																					 config.options_2d_life.birth_conditions,
																					 "array of sums which qualify cell for birth");
	config.subcmd_counter =
			config.cmd_set->add_subcommand("counter", "set value of FPS step counter");
	config.subcmd_counter
			->add_option("-c,--counter", config.option_counter, "value to which to count")
			->check(CLI::TypeValidator<std::uint32_t>())
			->required();
	config.cmd_clear = this->parser.add_subcommand("clear", "clears CLI");
	config.options_clear.clear_cli = config.cmd_clear->add_flag("-c,--cli", "clears CLI");
	config.options_clear.clear_map = config.cmd_clear->add_flag("-m,--cellmap", "clears cellmap");
	config.cmd_seed = this->parser.add_subcommand("seed", "seed cellmap with initial state");
	config.cmd_seed
			->add_option("-x,--column", config.options_seed.x, "x coordinate of the seed center")
			->check([min = 0, &max = this->config.options_cellmap.width](const std::string &num) {
				return CLI::DynamicRangeValidatorUtil(std::stoul(num), min, max);
			})
			->required();

	config.cmd_seed->add_option("-y,--row", config.options_seed.y, "y coordinate of the seed center")
			->check([min = 0, &max = this->config.options_cellmap.height](const std::string &num) {
				return CLI::DynamicRangeValidatorUtil(std::stoul(num), min, max);
			})
			->required();
	config.cmd_seed
			->add_option("-r,--range", config.options_seed.range, "range of seed area from the center")
			->check([min = 0, &height = this->config.options_cellmap.height,
							 &width = this->config.options_cellmap.width](const std::string &num) {
				return CLI::DynamicRangeValidatorUtil(std::stoul(num), min,
																							2 * std::min(width, height) + 1);
			})
			->required();
	config.options_seed.round =
			config.cmd_seed->add_flag("-c,--circle", "switch from the square area to circle area");
	config.options_seed.clip = config.cmd_seed->add_flag(
			"-l,--clip", "clip the seed area if out of bounds if not specified out of bounds"
									 " area will behave as if map has torus topology");
	config.cmd_start = this->parser.add_subcommand("start", "starts stopped simulation");
	config.cmd_stop =
			this->parser.add_subcommand("stop", "stops simulation until next start command");
}
