//
// Created by reg on 8/10/22.
//
#include "rules/rule_info_window.hpp"
#include <imgui.h>

void CSIM::drawRuleInfoWindow(const std::shared_ptr<Rule> &rule,
															const std::shared_ptr<RuleConfig> &rule_config) {
	/// NOLINTBEGIN
	ImGui::Begin("Rule Info");
	ImGui::Text("Rule type :: %s", rule->ruleTypeSerialized().data());
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Text("Rule config :: %s", rule_config->ruleConfigName().data());
	ImGui::Separator();
	for (const auto &pair : rule_config->configSerialized()) {
		ImGui::Text("%s :: %s", pair.first.c_str(), pair.second.c_str());
	}
	ImGui::Separator();
	ImGui::End();
	/// NOLINTEND
}
