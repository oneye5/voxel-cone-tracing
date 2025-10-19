#include "lsystem/node/node.hpp"

namespace lsystem::node {
	std::vector<const Node *> Node::grow(std::minstd_rand &rng) const {
		(void)rng;
		return std::vector{this};
	}
	Node::Node() {}
	Node::~Node() {}
}
