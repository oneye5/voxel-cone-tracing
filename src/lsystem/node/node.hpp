#pragma once

#include "cgra/cgra_mesh.hpp"
#include <random>

namespace lsystem::node {
	struct node_stack {
		glm::mat4 trans;
		float size;
		float step;
		std::vector<float> *steps;
	};
	class Node {
		public:
		Node();
		virtual ~Node();

		virtual std::vector<const Node*> grow(std::minstd_rand &rng) const;

		virtual void render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const = 0;
	};
}
