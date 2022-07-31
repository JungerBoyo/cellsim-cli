//
// Created by reg on 7/29/22.
//

#ifndef CELLSIM_RULE_CONFIG_HPP
#define CELLSIM_RULE_CONFIG_HPP

#include <shaders/shaders.hpp>
#include "utils/vecs.hpp"

#include <memory>
#include <span>
#include <string_view>

namespace CSIM {

using namespace utils;

struct RuleConfig { // NOLINT no need for move constructor/assignment
	std::shared_ptr<Shader> step_shader_;

	explicit RuleConfig(std::shared_ptr<Shader> step_shader)
			: step_shader_(std::move(step_shader)) {
	}

	[[nodiscard]] virtual std::size_t size() const = 0;
	[[nodiscard]] virtual const void *data() const = 0;

	[[nodiscard]] std::shared_ptr<const Shader> getShader() const {
		return step_shader_;
	}

	virtual void destroy() {
		step_shader_->destroy();
	}

	virtual ~RuleConfig() = default;
};

struct RuleConfig1DTotalistic : public RuleConfig {
	static constexpr Vec2<std::uint32_t> RANGE_LIM{0, 10};
	static constexpr std::size_t MAX_OPTIONS{2 * RANGE_LIM.y + 1};

	struct Config {
		std::int32_t range;
		std::int32_t center_active;
		alignas(16) std::int32_t survival_conditions_hashmap[MAX_OPTIONS]; // NOLINT interfacing with gl
		alignas(16) std::int32_t birth_conditions_hashmap[MAX_OPTIONS]; // NOLINT interfacing with gl
	};

	Config config_;

	RuleConfig1DTotalistic( // NOLINT config is populated in the body
			std::int32_t range, bool center_active,
			std::span<const std::size_t> survival_conditions,
			std::span<const std::size_t> birth_conditions,
			std::shared_ptr<Shader> step_shader);

	[[nodiscard]] std::size_t size() const override {
		return sizeof(Config);
	}
	[[nodiscard]] const void *data() const override {
		return &config_;
	}

	void destroy() override {
		RuleConfig::destroy();
	}
};

struct RuleConfig1DBinary : public RuleConfig {
	static constexpr Vec2<std::uint32_t> RANGE_LIM{ 0, 4 };
	static constexpr std::uint32_t MAX_PATTERN_COUNT{ 16 }; // (2^(2 * RANGE_LIM.y + 1))/32

	struct Config {
		std::int32_t range;
		alignas(16) std::uint32_t pattern_match_code[MAX_PATTERN_COUNT]; // NOLINT interfacing with gl
	};

	Config config_;

	RuleConfig1DBinary( // NOLINT init inside the body
			std::int32_t range,
			std::string_view pattern_match_code,
			std::shared_ptr<Shader> step_shader)
			: RuleConfig(std::move(step_shader)) {
		config_.range = range;
		parsePatternMatchCode(pattern_match_code);
	}

	void parsePatternMatchCode(std::string_view pattern_match_code);

	[[nodiscard]] std::size_t size() const override {
		return sizeof(Config);
	}
	[[nodiscard]] const void *data() const override {
		return &config_;
	}

	void destroy() override {
		RuleConfig::destroy();
	}
};

} // namespace CSIM

#endif // CELLSIM_RULE_CONFIG_HPP
