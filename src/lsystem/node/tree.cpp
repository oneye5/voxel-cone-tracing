#include "lsystem/node/node.hpp"
#include "lsystem/node/tree.hpp"

namespace lsystem::node::tree {
	std::vector<const Node*> Leaf::grow(std::minstd_rand& rng) const {
		std::vector<const Node*> ret;

		return ret;
	}

	void Leaf::render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {

	}

	Leaf::~Leaf() {}

	// Branch::Branch() : {}
	std::vector<const Node*> Branch::grow(std::minstd_rand& rng) const {
		std::vector<const Node*> ret;

		return ret;
	}

	void Branch::render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
	}
	Branch::Branch() : Node{} {}
	Branch::~Branch() {}
}
