#pragma once
#include "lsystem/node/node.hpp"
namespace lsystem::node::bush {
	class Leaf : public Node {
		public:
		virtual std::vector<std::shared_ptr<const Node>> grow(std::shared_ptr<const Node> self, std::minstd_rand& rng) const override;
		virtual void render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const override;
		virtual ~Leaf();
	};

	class Branch : public Node {
	public:
		virtual void render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const override;
		virtual ~Branch();
	};

	class Vertical : public Node {
	public:
		virtual std::vector<std::shared_ptr<const Node>> grow(std::shared_ptr<const Node> self, std::minstd_rand& rng) const override;
		virtual void render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const override;
		virtual ~Vertical();
	};

	extern std::shared_ptr<const Vertical> vertical;
	extern std::shared_ptr<const Branch> branch;
	extern std::shared_ptr<const Leaf> leaf;
}
