//
// Created by reg on 7/29/22.
//

#ifndef CELLSIM_RULE_CONFIG_HPP
#define CELLSIM_RULE_CONFIG_HPP

#include "utils/vecs.hpp"
#include <shaders/shaders.hpp>

#include <memory>
#include <string_view>
#include <utility>
#include <vector>

namespace CSIM {

using namespace utils;

/**
 * Rule config is interface class which contains info about current rule specific config and
 * compute shader it can be given to compatible rule type
 */
struct RuleConfig { // NOLINT no need for move constructor/assignment
private:
	std::shared_ptr<Shader> step_shader_; /// shader

public:
	explicit RuleConfig(std::shared_ptr<Shader> step_shader) : step_shader_(std::move(step_shader)) {
	}

	/**
	 * get rule config name to display in info window
	 * @return string view name
	 */
	[[nodiscard]] virtual std::string_view ruleConfigName() const = 0;
	/**
	 * get config serialized to show in info window.
	 * @return array of config:value pairs
	 */
	[[nodiscard]] virtual const std::vector<std::pair<std::string, std::string>> &
	configSerialized() const = 0;

	/**
	 * @return size of config data
	 */
	[[nodiscard]] virtual std::size_t size() const = 0;
	/**
	 * @return pointer to config data
	 */
	[[nodiscard]] virtual const void *data() const = 0;

	[[nodiscard]] std::shared_ptr<const Shader> getShader() const {
		return step_shader_;
	}
	virtual void destroy() {
		step_shader_->destroy();
	}

	virtual ~RuleConfig() = default;
};

/**
 * Rule config 1d totalistic defines config for 1d rule that allows to specify: the range
 * of cells taken into computation of the alive cell sum, if center cell (right above currently
 * processed cell) should be taken into account, array of survival/brith conditions with which
 * computed sum of alive cells is compered with
 */
struct RuleConfig1DTotalistic : public RuleConfig {
	static constexpr Vec2<std::uint32_t> RANGE_LIM{0, 10}; /*!< limits of range configuration*/
	static constexpr std::size_t MAX_OPTIONS{2 * RANGE_LIM.y + 1}; /*!< max number of b/s options*/

private:
	/**
	 * config defines range, center active and b/s option arrays
	 */
	struct Config {
		std::int32_t range;
		std::int32_t center_active;
		alignas(16) std::int32_t survival_conditions_hashmap[MAX_OPTIONS]; // NOLINT interfacing with gl
		alignas(16) std::int32_t birth_conditions_hashmap[MAX_OPTIONS];		 // NOLINT interfacing with gl
	};

	Config config_;
	std::vector<std::pair<std::string, std::string>> config_serialized_;

public:
	/**
	 * @param range maps to Config::range
	 * @param center_active maps to Config::center_active
	 * @param survival_conditions maps to Config::survival_conditions_hashmap
	 * @param birth_conditions maps to Config::birth_conditions_hashmap
	 * @param step_shader compute shader used by the rule
	 */
	RuleConfig1DTotalistic( // NOLINT config is populated in the body
			std::int32_t range, bool center_active, const std::vector<std::size_t> &survival_conditions,
			const std::vector<std::size_t> &birth_conditions, std::shared_ptr<Shader> step_shader);

	[[nodiscard]] std::string_view ruleConfigName() const override {
		return "1D Totalistic";
	}
	[[nodiscard]] const std::vector<std::pair<std::string, std::string>> &
	configSerialized() const override {
		return config_serialized_;
	}

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

/**
 * Rule config 1d binary defines config for 1d rule that allows to specify: the range
 * of cells taken into computation of the bit pattern that alive cells create with dead cells,
 * pattern_match_code defining which patterns can make cell alive (if 1) or dead (if 0)
 */
struct RuleConfig1DBinary : public RuleConfig {
	static constexpr Vec2<std::uint32_t> RANGE_LIM{0, 4}; /*!< limits of range configuration*/
	static constexpr std::uint32_t MAX_PATTERN_COUNT{16}; // (2^(2 * RANGE_LIM.y + 1))/32

private:
	/**
	 * config defines range and pattern_match_code
	 */
	struct Config {
		std::int32_t range;
		alignas(16) std::uint32_t pattern_match_code[MAX_PATTERN_COUNT]; // NOLINT interfacing with gl
	};

	Config config_;
	std::vector<std::pair<std::string, std::string>> config_serialized_;

public:
	/**
	 * @param range maps to Config::range
	 * @param pattern_match_code maps to Config::pattern_match_code
	 * @param step_shader
	 */
	RuleConfig1DBinary( // NOLINT init inside the body
			std::int32_t range, std::string_view pattern_match_code, std::shared_ptr<Shader> step_shader)
			: RuleConfig(std::move(step_shader)),
				config_serialized_({{"Range", std::to_string(range)},
														{"Pattern match code", std::string(pattern_match_code)}}) {
		config_.range = range;
		parsePatternMatchCode(pattern_match_code);
	}

	[[nodiscard]] std::string_view ruleConfigName() const override {
		return "1D Binary";
	}
	[[nodiscard]] const std::vector<std::pair<std::string, std::string>> &
	configSerialized() const override {
		return config_serialized_;
	}

	[[nodiscard]] std::size_t size() const override {
		return sizeof(Config);
	}
	[[nodiscard]] const void *data() const override {
		return &config_;
	}

	void destroy() override {
		RuleConfig::destroy();
	}

private:
	/**
	 * Transforms pattern match code in string form to pattern match code in
	 * Config::pattern_match_code array
	 * @param pattern_match_code
	 */
	void parsePatternMatchCode(std::string_view pattern_match_code);
};

/**
 * Rule config 2D cyclic defines config for 2d rule, with: range of cells taken into account
 * in sum computation, threshold signifying which sums results in birth/death, moore flag
 * if set kernel is moore kernel otherwise neumann kernel, state insensitive flag if set
 * then all cells with state > 0 are accumulated into sum otherwise only cells with state
 * next after the processed cell's state are accumulated into sum
 */
struct RuleConfig2DCyclic : public RuleConfig {
	static constexpr Vec2<std::uint32_t> RANGE_LIM{0, 10};

private:
	static constexpr std::uint32_t offsets_capacity{((2 * RANGE_LIM.y + 1) * (2 * RANGE_LIM.y + 1))};

	struct Config {
		std::int32_t threshold;
		std::int32_t state_insensitive;
		std::int32_t offsets_count;
		alignas(16) Vec2<std::int32_t> offsets[offsets_capacity]; // NOLINT interfacing with gl
	};

	Config config_;
	std::vector<std::pair<std::string, std::string>> config_serialized_;

public:
	static constexpr Vec2<std::uint32_t> SUM_LIM{0, offsets_capacity};

	/**
	 * @param range info needed for offsets creation
	 * @param threshold  maps to Config::threshold
	 * @param moore info needed for offsets creation
	 * @param state_insensitive  maps to Config::state_insensitive
	 * @param center_active info needed for offsets creation
	 * @param step_shader
	 */
	RuleConfig2DCyclic(std::int32_t range, std::int32_t threshold, bool moore, bool state_insensitive,
										 bool center_active, std::shared_ptr<Shader> step_shader);

	[[nodiscard]] std::string_view ruleConfigName() const override {
		return "2D Cyclic";
	}
	[[nodiscard]] const std::vector<std::pair<std::string, std::string>> &
	configSerialized() const override {
		return config_serialized_;
	}

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

struct RuleConfig2DLife : public RuleConfig {
	static constexpr std::size_t MAX_OPTIONS{256}; // 2^8 / 4
private:
	struct Config {
		std::int32_t state_insensitive;
		std::int32_t offsets_count;
		alignas(16) utils::Vec4<std::int32_t> offsets[9];									 // NOLINT interfacing with gl
		alignas(16) std::int32_t survival_conditions_hashmap[MAX_OPTIONS]; // NOLINT interfacing with gl
		alignas(16) std::int32_t birth_conditions_hashmap[MAX_OPTIONS];		 // NOLINT interfacing with gl
	};

	Config config_;
	std::vector<std::pair<std::string, std::string>> config_serialized_;

public:
	RuleConfig2DLife(bool moore, bool state_insensitive, bool center_active,
									 const std::vector<std::size_t> &survival_conditions,
									 const std::vector<std::size_t> &birth_conditions,
									 std::shared_ptr<Shader> step_shader);

	[[nodiscard]] std::string_view ruleConfigName() const override {
		return "2D Cyclic";
	}
	[[nodiscard]] const std::vector<std::pair<std::string, std::string>> &
	configSerialized() const override {
		return config_serialized_;
	}

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