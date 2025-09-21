#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <istream>
#include <sstream>
#include <string>
#include <unordered_map>
#include "imgui.h"
#include "lsystem.hpp"

using std::vector;

lsystem::ruleset lsystem::parse_rules(std::istream& rules) {
	std::unordered_map<char, std::string> map;
	
	while(rules.peek() && !rules.eof()) {
		char k = rules.get();
		if (rules.get() != ':') {
			throw "Bad rules format, missing :";
		}
		std::string v;
		std::getline(rules, v);
		map[k] = v;
	}

	return map;
}

std::string lsystem::iterate(const std::string &seed, const ruleset &rules, int count) {
	std::string cur = seed;
	for (int i = 0; i < count; i++) {
		std::stringstream res{};
		for (const auto& c : cur) {
			if (rules.find(c) != rules.end()) {
				res << rules.at(c);
			} else {
				res << c;
			}
		}
		cur = res.str();
	}
	return cur;
}

namespace lsystem::gui {
	void rules_window(struct Data &data) {
		ImGui::SetNextWindowPos(ImVec2(310, 5), ImGuiSetCond_Once);
		ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiSetCond_Once);
		ImGui::Begin("LSystem Rules", nullptr);

		static int rule_idx = 0;
		static std::string rule_text;
		bool refresh = ImGui::Button("Refresh");
		ImGui::SameLine();
		refresh |= ImGui::Combo("Rule", &rule_idx, available_rule_names, sizeof(available_rule_names)/sizeof(*available_rule_names));
		if (refresh) {
			if (rule_idx) {
				std::ifstream file{available_rule_paths[rule_idx]};
				data.rules = parse_rules(file);
				data.regrow = true;

				const auto& rules = data.rules.value();

				// Sort our keys
				vector<char> cs;
				for (const auto& n: rules) {
					cs.push_back(n.first);
				}
				std::sort(cs.begin(), cs.end());

				std::stringstream ss;
				for(const auto c: cs) {
					ss << c << ": " << rules.at(c) << "\n";
				}

				rule_text = ss.str();
			} else {
				data.rules.reset();
			}
		} else {
			data.regrow = false;
		}

		if (data.rules) {
			ImGui::Separator();
			ImGui::Text("%s", rule_text.c_str());
		}

		ImGui::End();
	}

	void growth_window(struct Data &data) {
		ImGui::SetNextWindowPos(ImVec2(310, 210), ImGuiSetCond_Once);
		ImGui::SetNextWindowSize(ImVec2(350, 120), ImGuiSetCond_Once);
		ImGui::Begin("LSystem Growth", nullptr);

		if (!data.rules.has_value()) {
			ImGui::Text("No rules loaded.");
			ImGui::End();
			return;
		}

		static char input_buf[512];
		bool regrow = ImGui::InputText("Seed", input_buf, sizeof(input_buf));

		ImGui::SameLine();
		static bool every_step = false;
		ImGui::Checkbox("Debug", &every_step);

		static int iterations = 3;
		regrow |= ImGui::InputInt("Iterations", &iterations);

		if (iterations < 0) iterations = 0;

		static vector<std::string> growth;
		if (regrow || data.regrow) {
			if (!every_step) {
				growth = {iterate(std::string{input_buf}, data.rules.value(), iterations)};
			} else {

			}
		}

		for (const auto& s : growth) {
			ImGui::Separator();
			ImGui::TextWrapped("%s", s.c_str());
		}

		ImGui::End();
	}
}
