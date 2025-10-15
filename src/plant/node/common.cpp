#include "plant/node/node.hpp"
#include "plant/node/common.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;

namespace plant::node::common {
	void Push::render(std::vector<mat4> stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
		stack.push_back(stack.back());
	}
	Push::~Push() {}
	const Push *push = new Push();

	void Pop::render(std::vector<mat4> stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
		stack.pop_back();
	}
	Pop::~Pop() {}
	const Pop *pop = new Pop();

	Translate::Translate(vec3 dist) : dist{dist} {}
	void Translate::render(std::vector<mat4> stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
		stack.back() = translate(stack.back(), dist);
	}
	Translate::~Translate() {}

	void TrunkVertex::render(std::vector<mat4> stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
		// TODO: Normals
		trunk.push_index(trunk.push_vertex({stack.back() * vec4{0,0,0,1}}));
	}
	TrunkVertex::~TrunkVertex() {}
	const TrunkVertex *trunkVertex = new TrunkVertex();

	void CanopyVertex::render(std::vector<mat4> stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
		// TODO: Normals
		canopy.push_index(trunk.push_vertex({stack.back() * vec4{0,0,0,1}}));
	}
	CanopyVertex::~CanopyVertex() {}
	const CanopyVertex *canopyVertex = new CanopyVertex();
}
