#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "lsystem/node/node.hpp"
#include "lsystem/node/common.hpp"
#include "lsystem/node/tree.hpp"

using namespace lsystem::node::common;
using namespace glm;

namespace lsystem::node::tree {
	std::shared_ptr<const Branch> branch = std::make_shared<Branch>();
	std::shared_ptr<const Leaf> leaf = std::make_shared<Leaf>();

	Branch::~Branch() {}

	Leaf::~Leaf() {}
	std::vector<std::shared_ptr<const Node>> Leaf::grow(std::shared_ptr<const Node> self, std::minstd_rand& rng) const {
		std::vector<std::shared_ptr<const Node>> ret;
		ret.push_back(branch);                                      // F
		ret.push_back(push);
		ret.push_back(std::make_shared<RotateY>(-0.20, 0.1));              // ???
		ret.push_back(leaf);
		ret.push_back(pop);

		ret.push_back(push);                                        // [
		ret.push_back(std::make_shared<RotateX>(0.3));              // +

		ret.push_back(push);                                        // [
		ret.push_back(std::make_shared<RotateZ>(0.3));              // +
		ret.push_back(leaf);                                        // A
		ret.push_back(pop);                                         // ]

		ret.push_back(push);                                        // [
		ret.push_back(std::make_shared<RotateZ>(-0.6));             // --
		ret.push_back(leaf);                                        // A
		ret.push_back(pop);                                         // ]

		ret.push_back(pop);                                         // ]

		ret.push_back(std::make_shared<RotateY>(0.9));              // ???

		ret.push_back(push);                                        // [
		ret.push_back(std::make_shared<RotateX>(-0.3));             // -

		ret.push_back(push);                                        // [
		ret.push_back(std::make_shared<RotateZ>(0.3));              // +
		ret.push_back(leaf);                                        // A
		ret.push_back(pop);                                         // ]

		ret.push_back(push);                                        // [
		ret.push_back(std::make_shared<RotateZ>(-0.6));             // --
		ret.push_back(leaf);                                        // A
		ret.push_back(pop);                                         // ]

		ret.push_back(pop);                                         // ]

		return ret;
	}

	void Leaf::render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
		(void)trunk;
		vec4 a = stack.back().trans * vec4{0,0,0,1};
		stack.back().trans = translate(stack.back().trans, vec3{0, stack.back().size/2.0, 0});
		vec4 b = stack.back().trans * vec4{0,0,0,1};
		vec4 norm = normalize(b-a);
		canopy.push_index(canopy.push_vertex({a, vec3{norm}}));
		stack.back().size *= 0.8;
	}

	void Branch::render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
		(void)canopy;
		trunk.push_index(trunk.push_vertex({{stack.back().trans * vec4{0,0,0,1}}, {0,0,1}}));
		stack.back().trans = translate(stack.back().trans, vec3{0, stack.back().size, 0});
		trunk.push_index(trunk.push_vertex({{stack.back().trans * vec4{0,0,0,1}}, {0,0,1}}));
		stack.back().steps->push_back(stack.back().step);
		stack.back().step += 1;
		stack.back().steps->push_back(stack.back().step);

		stack.back().size *= 0.8;
	}
}
