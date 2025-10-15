#include <algorithm>
#include <vector>
#include <string>
#include "lsystem.hpp"

using std::vector;

lsystem::ruleset lsystem::iterate(const lsystem::ruleset &current, std::minstd_rand &rng, const int count) {
	lsystem::ruleset prev = current;
	lsystem::ruleset cur;
	for (int i = 0; i < count; i++) {
		for (const auto& n : prev) {
			auto vec = n.grow();
			cur.insert(cur.end(), vec.begin(), vec.end());
			cur.push_back();
		}
	}

	return cur;
}
