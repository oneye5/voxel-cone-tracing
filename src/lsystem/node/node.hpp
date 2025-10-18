#pragma once

#include "cgra/cgra_mesh.hpp"
#include <random>

namespace lsystem::node {
	struct node_stack {
		glm::mat4 trans;
		float size;
		float step;
	};
	class Node {
		public:
		virtual ~Node();

		virtual std::vector<const Node*> grow(std::minstd_rand &rng) const;

		virtual void render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const = 0;
	};
}
