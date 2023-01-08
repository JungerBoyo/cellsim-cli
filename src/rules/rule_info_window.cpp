//
// Created by reg on 8/10/22.
//
#include "rules/rule_info_window.hpp"
#include <imgui.h>

static void makeBorder(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) {
	auto *draw_list = ImGui::GetWindowDrawList();
	draw_list->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(r, g, b, a));
}
void CSIM::drawRuleInfoWindow(const std::shared_ptr<Rule> &rule,
															const std::shared_ptr<RuleConfig> &rule_config) {
	/// NOLINTBEGIN
	ImGui::Begin("Rule Info");
	ImGui::LabelText("##base-info", "Base info"); makeBorder(255, 255, 0, 255);
	ImGui::Text("Rule type :: %s", rule->ruleTypeSerialized().data());
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Text("Rule config :: %s", rule_config->ruleConfigName().data());
	ImGui::Separator();
	ImGui::LabelText("##rule-specific-info", "Rule specific info"); makeBorder(255, 255, 0, 255);
	for (const auto &pair : rule_config->configSerialized()) {
		ImGui::Text("%s :: %s", pair.first.c_str(), pair.second.c_str());
	}
	ImGui::Separator();
	ImGui::End();
	/// NOLINTEND
}
