#pragma once
#include <istream>
#include <optional>
#include <string>
#include <unordered_map>

namespace lsystem {
	typedef std::unordered_map<char, std::string> ruleset;
	std::string iterate(const std::string& seed, const ruleset& rules, int count = 1);
}
