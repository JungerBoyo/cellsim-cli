#ifndef CELLSIM_RULES_HPP
#define CELLSIM_RULES_HPP

#include "utils/vecs.hpp"
#include <cellmap/cellmap.hpp>
#include <rules/rule_config.hpp>

#include <memory>

namespace CSIM {

using namespace utils;

enum class RuleType {
	BASIC_1D,
	BASIC_2D
};
/**
 * Interface class for defining the rule algorithm. It takes compatible rule config struct that
 * requires compatible void step(...) procedure
 */
struct Rule { // NOLINT no need for move constructor/assignment
	/**
	 * base config, contains information about map resolution, number of states and which iteration
	 * rule is on
	 */
	struct BaseConfig {
		Vec2<std::int32_t> map_resolution;
		std::int32_t state_count;
		std::int32_t iteration;
	};
private:
	std::uint32_t base_config_ubo_id_{0}; /*!< ubo handle, maps to base config data*/
	std::uint32_t config_ubo_id_{0}; /*!< ubo handle, maps to config data*/

	BaseConfig base_config_;
	std::shared_ptr<RuleConfig> rule_config_; /*!< pointer to rule config currently set on the rule*/

public:
	/**
	 * @param rule_config config that rule will be using on right after creation
	 */
	explicit Rule(std::shared_ptr<RuleConfig> rule_config);
	/**
	 * Function which change used rule config
	 * @param rule_config
	 */
	void setRuleConfig(std::shared_ptr<RuleConfig> rule_config);
	/**
	 * @return rule type which identifies the rule
	 */
	[[nodiscard]] virtual RuleType ruleType() const = 0;
	/**
   * @return serialized version of rule type to show info on the screen
 	 */
	[[nodiscard]] virtual std::string_view ruleTypeSerialized() const = 0;
	/**
	 * @return current iteration
	 */
	[[nodiscard]] auto iteration() const { return base_config_.iteration; }
	/**
	 * function should call rule algorithm
	 * @param cell_map current cell map
	 * @param state_count  current number of cell states
	 */
	virtual void step(const CellMap &cell_map, std::int32_t state_count) = 0;

	virtual void destroy();
	virtual ~Rule() = default;
protected:
	/**
   * function sets base config
   * @param config new base config struct
   * @param update_iteration if false current iteration will be preserved
   */
	void setBaseConfig(BaseConfig config, bool update_iteration = false) noexcept;
	/**
	 * function returns iteration and postincrement it
	 * @return current iteration
	 */
	[[nodiscard]] auto iterate() { return base_config_.iteration++; }
	/**
	 * Binds current config compute shader
	 */
	 void bindConfigShader() { rule_config_->getShader()->bind(); }
};

/**
 * Rule performs basic 1D cellular automata algorithm, going from top to bottom and updating
 * each row
 */
struct Rule1D : public Rule {
	explicit Rule1D(std::shared_ptr<RuleConfig> rule_config);

	[[nodiscard]] RuleType ruleType() const override { return RuleType::BASIC_1D; }
	[[nodiscard]] std::string_view ruleTypeSerialized() const override { return "Basic 1D"; }

	void step(const CellMap &cell_map, std::int32_t state_count) noexcept override;

	void destroy() override;
};

/**
 * Rule performs basic kernel based 2D cellular automata algorithm
 */
struct Rule2D : public Rule {
private:
	std::uint32_t state_map_copy_ssbo_id_{ 0 };
	Vec2<std::int32_t> previous_map_resolution_{ 0, 0 };

public:
	explicit Rule2D(std::shared_ptr<RuleConfig> rule_config);

	[[nodiscard]]	RuleType ruleType() const override { return RuleType::BASIC_2D; }
	[[nodiscard]] std::string_view ruleTypeSerialized() const override { return "Basic 2D"; }

	void step(const CellMap &cell_map, std::int32_t state_count) noexcept override;

	void destroy() override;
};

} // namespace CSIM

#endif // CELLSIM_RULES_HPP
