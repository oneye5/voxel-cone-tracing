#pragma once
#include "plant/node/node.hpp"

namespace plant::node::common {
	class Push : public Node {
		public:
		virtual void render(std::vector<glm::mat4> stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const;
		virtual ~Push();
	};
	extern Push *push;

	class Pop : public Node {
		public:
		virtual void render(std::vector<glm::mat4> stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const;
		virtual ~Pop();
	};
	extern Pop *pop;

	class Translate : public Node {
		glm::vec3 dist;
		public:
		Translate(glm::vec3 dist);

		virtual void render(std::vector<glm::mat4> stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const;
		virtual ~Translate();
	};

	class TrunkVertex : public Node {
		public:

		virtual void render(std::vector<glm::mat4> stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const;
		virtual ~TrunkVertex();
	};
	extern TrunkVertex *trunkVertex;

	class CanopyVertex : public Node {
		public:

		virtual void render(std::vector<glm::mat4> stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const;
		virtual ~CanopyVertex();
	};
	extern CanopyVertex *canopyVertex;
}
