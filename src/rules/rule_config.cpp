//
// Created by reg on 7/29/22.
//
#include "rules/rule_config.hpp"
#include <algorithm>
#include <cstddef>
#include <numeric>
#include <span>
#include <array>
#include <cstring>

// NOLINTBEGIN
static void convertToVec4Format(std::int32_t* dst, std::size_t dst_size,
																const std::vector<std::size_t>& src) {
	for (const auto &value : src) {
		if (value < dst_size) {
			dst[value] = 1;
		}
	}
}
// NOLINTEND

static auto generateKernelOffsets(bool moore, bool center_active, std::int32_t range) {
	std::vector<CSIM::utils::Vec2<std::int32_t>> offsets;

	if (moore) {
		/// generating moore kernel offsets (square)
		const auto size = 2 * range + 1;
		const auto offset_count = size * size - static_cast<std::int32_t>(!center_active);
		offsets.reserve(offset_count);
		for (std::int32_t y=-range; y<=range; ++y) {
			for (std::int32_t x=-range; x<=range; ++x) {
				if (x == 0 && y == 0 && !center_active) {
					continue;
				}
				offsets.push_back({x, y});
			}
		}
	} else {
		/// generating neumann kernel offsets (diamond)
		const auto size = 2 * range + 1;
		const auto offset_count = (range - 1) * (1 + (1 + (range-2) * 2)) + size -
													     static_cast<std::int32_t>(!center_active);
		offsets.reserve(offset_count);
		for (std::int32_t y=0; y<range-1; ++y) {
			const auto true_y = y - (range - 1);
			for (std::int32_t x=0; x<=y; ++x) {
				const auto true_x = x + 1;
				offsets.push_back({ true_x, true_y});
				offsets.push_back({-true_x, true_y});
				offsets.push_back({ true_x -true_y});
				offsets.push_back({-true_x,-true_y});
			}
		}
		for (std::int32_t xy=-range; xy<=range; ++xy) {
			if (xy == 0) {
				if (!center_active) {
					continue;
				} else {
					offsets.push_back({0, 0});
				}
			} else {
					offsets.push_back({0, xy});
					offsets.push_back({xy, 0});
				}
			}
		}

		return offsets;
}

CSIM::RuleConfig1DTotalistic::RuleConfig1DTotalistic( // NOLINT config is
																											// populated in the body
		std::int32_t range, bool center_active,
		const std::vector<std::size_t>& survival_conditions,
		const std::vector<std::size_t>& birth_conditions,
		std::shared_ptr<Shader> step_shader)
		: RuleConfig(std::move(step_shader)),
			config_serialized_({{"Range", std::to_string(range)},
													{"Center active", center_active ? "true" : "false"},
												  {"Survival conditions", std::accumulate(survival_conditions.cbegin(),
																					 survival_conditions.cend(),
																					 std::string{},
																					 [](std::string str, std::size_t value) {
																						return std::move(str) + std::to_string(value) + ',';
																					 }
																					)},
													{"Birth conditions", std::accumulate(birth_conditions.cbegin(),
																					 birth_conditions.cend(),
																					 std::string{},
																					 [](std::string str, std::size_t value) {
																						return std::move(str) + std::to_string(value) + ',';
																					 })}}) {
	config_.range = range;
	config_.center_active = static_cast<std::int32_t>(center_active);

	// NOLINTBEGIN
	std::fill(config_.survival_conditions_hashmap,
						config_.survival_conditions_hashmap + MAX_OPTIONS, 0);

	std::fill(config_.birth_conditions_hashmap,
						config_.birth_conditions_hashmap + MAX_OPTIONS, 0);

	convertToVec4Format(config_.survival_conditions_hashmap, MAX_OPTIONS, survival_conditions);
	convertToVec4Format(config_.birth_conditions_hashmap, MAX_OPTIONS, birth_conditions);
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

CSIM::RuleConfig2DCyclic::RuleConfig2DCyclic(std::int32_t range, std::int32_t threshold, bool moore,
																						 bool state_insensitive, bool center_active,
																						 std::shared_ptr<Shader> step_shader)
	: RuleConfig(std::move(step_shader)),
		config_serialized_({{"Range", std::to_string(range)},
												{"Threshold", std::to_string(threshold)},
												{"Kernel type", moore ? "Moore" : "Neumann"},
												{"State insensitive", state_insensitive ? "Yes" : "No"},
												{"Center active", center_active ? "Yes" : "No"}}) {

	config_.threshold = threshold;
	config_.state_insensitive = static_cast<std::int32_t>(state_insensitive);

	const auto offsets = generateKernelOffsets(moore, center_active, range);
	config_.offsets_count = static_cast<std::int32_t>(offsets.size());

	std::memcpy(config_.offsets, offsets.data(), offsets.size() * sizeof(utils::Vec2<std::int32_t>)); // NOLINT interfacing with gl
}
CSIM::RuleConfig2DLife::RuleConfig2DLife(bool moore, bool state_insensitive, bool center_active,
																				 const std::vector<std::size_t>& survival_conditions,
																				 const std::vector<std::size_t>& birth_conditions,
																				 std::shared_ptr<Shader> step_shader)
 : RuleConfig(std::move(step_shader)),
			config_serialized_({{"State insensitive", state_insensitive ? "true" : "false"},
													{"Center active", center_active ? "true" : "false"},
													{"Kernel type", moore ? "Moore" : "Neumann"},
													{"Survival conditions", std::accumulate(survival_conditions.cbegin(),
																																	survival_conditions.cend(),
																																	std::string{},
																																	[](std::string str, std::size_t value) {
																																		return std::move(str) + std::to_string(value) + ',';
																																	}
																																	)},
													{"Birth conditions", std::accumulate(birth_conditions.cbegin(),
																															 birth_conditions.cend(),
																															 std::string{},
																															 [](std::string str, std::size_t value) {
																																 return std::move(str) + std::to_string(value) + ',';
																															 })}}){
	config_.state_insensitive = static_cast<std::int32_t>(state_insensitive);
	// NOLINTBEGIN
	std::fill(config_.survival_conditions_hashmap,
						config_.survival_conditions_hashmap + MAX_OPTIONS, 0);

	std::fill(config_.birth_conditions_hashmap,
						config_.birth_conditions_hashmap + MAX_OPTIONS, 0);

	convertToVec4Format(config_.survival_conditions_hashmap, MAX_OPTIONS, survival_conditions);
	convertToVec4Format(config_.birth_conditions_hashmap, MAX_OPTIONS, birth_conditions);
	// NOLINTEND

	const auto offsets = generateKernelOffsets(moore, center_active, 1);
	config_.offsets_count = static_cast<std::int32_t>(offsets.size());

	std::transform(offsets.cbegin(), offsets.cend(), config_.offsets, // NOLINT
								 [](utils::Vec2<std::int32_t> offset){
									 return Vec4<std::int32_t>{ offset.x, offset.y, 0, 0 };
								 });
}
