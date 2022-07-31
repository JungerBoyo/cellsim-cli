//
// Created by reg on 7/29/22.
//
#include "rules/rule_config.hpp"
#include <algorithm>

CSIM::RuleConfig1DTotalistic::RuleConfig1DTotalistic( // NOLINT config is populated in the body
		std::int32_t range, bool center_active,
		std::span<const std::size_t> survival_conditions,
		std::span<const std::size_t> birth_conditions,
		std::shared_ptr<Shader> step_shader)
			: RuleConfig(std::move(step_shader)) {

	config_.range = range;
	config_.center_active = static_cast<std::int32_t>(center_active);

	auto survival_conditions_hashmap = std::span(config_.survival_conditions_hashmap);
	auto birth_conditions_hashmap = std::span(config_.birth_conditions_hashmap);

	std::fill(survival_conditions_hashmap.begin(), survival_conditions_hashmap.end(), 0); // NOLINT 0/1
	std::fill(birth_conditions_hashmap.begin(), birth_conditions_hashmap.end(), 0); // NOLINT 0/1

	const auto setValues = [](std::span<std::int32_t> dst,
														std::span<const std::size_t> src) {
		for (const auto &value : src) {
			if (value < dst.size()) {
				dst[value] = 1;
			}
		}
	};
	setValues(survival_conditions_hashmap, survival_conditions);
	setValues(birth_conditions_hashmap, birth_conditions);
}


void CSIM::RuleConfig1DBinary::parsePatternMatchCode(std::string_view pattern_match_code) {
	auto wrapped_pattern_match_code = std::span(config_.pattern_match_code);
	std::fill(wrapped_pattern_match_code.begin(),
						wrapped_pattern_match_code.end(), 0);

	auto i = 0u;//static_cast<std::uint32_t>(pattern_match_code.length() - 1);
	constexpr std::uint32_t BIT_SHIFT{5ul}; // log2(sizeof(int))
	constexpr std::uint32_t MASK{0x1Ful};		// % 32
	for (const auto c : pattern_match_code) {
		if (c == '1') {
			wrapped_pattern_match_code[i >> BIT_SHIFT] |= (1u << (i & MASK));
		}
		++i;
	}
}