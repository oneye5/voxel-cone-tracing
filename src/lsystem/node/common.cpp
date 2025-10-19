#include "lsystem/node/node.hpp"
#include "lsystem/node/common.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;

namespace lsystem::node::common {
	void Push::render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
		(void)trunk, (void)canopy;
		stack.push_back(stack.back());
	}
	Push::~Push() {}
	const Push *push = new Push();

	void Pop::render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
		(void)trunk, (void)canopy;
		stack.pop_back();
	}
	Pop::~Pop() {}
	const Pop *pop = new Pop();

	template<glm::vec3 axis>
	Rotate<axis>::Rotate(float angle) : angle{angle} {}

	template<glm::vec3 axis>
	void Rotate<axis>::render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
		(void)trunk, (void)canopy;
		stack.back().trans = rotate(stack.back().trans, this->angle, axis);
	}

	Translate::Translate(vec3 dist) : dist{dist} {}
	void Translate::render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
		(void)trunk, (void)canopy;
		stack.back().trans = translate(stack.back().trans, dist);
	}
	Translate::~Translate() {}

	void TrunkVertex::render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
		(void)trunk, (void)canopy;
		// TODO: Normals: translate by one then see where that leaves us as the normal?
		trunk.push_index(trunk.push_vertex({vec3{stack.back().trans * vec4{0,0,0,1}} }));
	}
	TrunkVertex::~TrunkVertex() {}
	const TrunkVertex *trunkVertex = new TrunkVertex();

	void CanopyVertex::render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
		(void)trunk, (void)canopy;
		// TODO: Normals: translate by one then see where that leaves us as the normal?
		canopy.push_index(trunk.push_vertex({vec3{stack.back().trans * vec4{0,0,0,1}}}));
	}
	CanopyVertex::~CanopyVertex() {}
	const CanopyVertex *canopyVertex = new CanopyVertex();
}
