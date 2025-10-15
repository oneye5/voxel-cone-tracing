#pragma once
#include <istream>
#include <optional>
#include <string>
#include "lsystem/node/node.hpp"
#include <vector>

namespace lsystem {
	typedef std::vector<const node::Node *> ruleset;
	std::string iterate(const ruleset &current, std::minstd_rand &rng, const int count = 1);
}
