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
