#pragma once
#include "node.hpp"
#include "lsystem/node/node.hpp"

namespace lsystem::node::common {
	class Push : public Node {
		public:
		virtual void render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const override;
		virtual ~Push();
	};
	extern const Push *push;

	class Pop : public Node {
		public:
		virtual void render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const override;
		virtual ~Pop();
	};
	extern const Pop *pop;

	class Translate : public Node {
		glm::vec3 dist;
		public:
		Translate(glm::vec3 dist);

		virtual void render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const override;
		virtual ~Translate();
	};

	class TrunkVertex : public Node {
		public:

		virtual void render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const override;
		virtual ~TrunkVertex();
	};
	extern const TrunkVertex *trunkVertex;

	class CanopyVertex : public Node {
		public:

		virtual void render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const override;
		virtual ~CanopyVertex();
	};
	extern const CanopyVertex *canopyVertex;
}
