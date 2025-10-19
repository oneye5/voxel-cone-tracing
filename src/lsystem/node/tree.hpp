#pragma once
#include "lsystem/node/node.hpp"
namespace lsystem::node::tree {
	class Leaf : public Node {
		public:
		virtual std::vector<const Node*> grow(std::minstd_rand& rng) const override;
		virtual void render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const override;
		virtual ~Leaf();
	};

	class Branch : public Node {
		public:
		Branch();
		virtual std::vector<const Node*> grow(std::minstd_rand& rng) const override;
		virtual void render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const override;
		virtual ~Branch();
	};
}
