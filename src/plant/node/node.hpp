#pragma once

#include "cgra/cgra_mesh.hpp"
#include <random>

namespace plant::node {
	class Node {
		public:
		virtual ~Node();

		virtual std::vector<const Node*> grow(std::minstd_rand &rng) const;
		virtual void render(std::vector<glm::mat4> stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const = 0;
	};
}
