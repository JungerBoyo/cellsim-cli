//
// Created by reg on 7/29/22.
//
#include "rules/rule_config.hpp"
#include <algorithm>
#include <span>
#include <array>

CSIM::RuleConfig1DTotalistic::RuleConfig1DTotalistic( // NOLINT config is
																											// populated in the body
		std::int32_t range, bool center_active,
		const std::vector<std::size_t>& survival_conditions,
		const std::vector<std::size_t>& birth_conditions,
		std::shared_ptr<Shader> step_shader)
		: RuleConfig(std::move(step_shader)) {

	config_.range = range;
	config_.center_active = static_cast<std::int32_t>(center_active);

	// NOLINTBEGIN
	std::fill(config_.survival_conditions_hashmap,
						config_.survival_conditions_hashmap + MAX_OPTIONS, 0);

	std::fill(config_.birth_conditions_hashmap,
						config_.birth_conditions_hashmap + MAX_OPTIONS, 0);

	const auto setValues = [](std::int32_t* dst, std::size_t dst_size,
														const std::vector<std::size_t>& src) {
		for (const auto &value : src) {
			if (value < dst_size) {
				dst[value] = 1;
			}
		}
	};
	setValues(config_.survival_conditions_hashmap, MAX_OPTIONS, survival_conditions);
	setValues(config_.birth_conditions_hashmap, MAX_OPTIONS, birth_conditions);
	// NOLINTEND
}

void CSIM::RuleConfig1DBinary::parsePatternMatchCode(
		std::string_view pattern_match_code) {
	auto wrapped_pattern_match_code = std::span(config_.pattern_match_code);
	std::fill(wrapped_pattern_match_code.begin(),
						wrapped_pattern_match_code.end(), 0);

	auto i = 0u;
	constexpr std::uint32_t BIT_SHIFT{5ul}; // log2(sizeof(int))
	constexpr std::uint32_t MASK{0x1Ful};		// % 32
	for (const auto c : pattern_match_code) {
		if (c == '1') {
			wrapped_pattern_match_code[i >> BIT_SHIFT] |= (1u << (i & MASK));
		}
		++i;
	}
}