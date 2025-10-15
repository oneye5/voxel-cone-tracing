#include "plant/node/node.hpp"

namespace plant::node {
	std::vector<const Node *> Node::grow(std::minstd_rand &rng) const {
		return std::vector{this};
	}

	Node::~Node() {}
}
