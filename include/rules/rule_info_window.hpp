//
// Created by reg on 8/10/22.
//

#ifndef CELLSIM_RULE_INFO_WINDOW_HPP
#define CELLSIM_RULE_INFO_WINDOW_HPP

#include <rules/rule.hpp>
#include <rules/rule_config.hpp>

namespace CSIM {
	void drawRuleInfoWindow(const std::shared_ptr<Rule>& rule,
												  const std::shared_ptr<RuleConfig>& rule_config);
}
#endif // CELLSIM_RULE_INFO_WINDOW_HPP
