#ifndef CELLSIM_RULES_HPP
#define CELLSIM_RULES_HPP

#include <cellmap/cellmap.hpp>
#include <rules/rule_config.hpp>
#include "utils/vecs.hpp"

#include <memory>

namespace CSIM {

using namespace utils;

struct Rule { // NOLINT no need for move constructor/assignment
	std::uint32_t base_config_ubo_id_ { 0 };
	std::uint32_t config_ubo_id_			{ 0 };

	struct BaseConfig {
		Vec2<std::int32_t> map_resolution;
		std::int32_t state_count;
		std::int32_t iteration;
	};

	BaseConfig base_config_;
	std::shared_ptr<RuleConfig> rule_config_;

	explicit Rule(std::shared_ptr<RuleConfig> rule_config);

	void setBaseConfig(BaseConfig config, bool update_iteration = false) noexcept;
	[[nodiscard]] auto iteration() { return base_config_.iteration++; }

	virtual void destroy();

	virtual void step(const CellMap& cell_map) = 0;
	virtual ~Rule() = default;
};
/*
 * TODO
 displaying current config in imgui VS create some showconfig subcommand
 */

struct Rule1D : public Rule {
	explicit Rule1D(std::shared_ptr<RuleConfig> rule_config);

	void step(const CellMap& cell_map) noexcept override;

	void destroy() override;
};

} // namespace CSIM

#endif // CELLSIM_RULES_HPP
