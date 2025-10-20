#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "lsystem/node/node.hpp"
#include "lsystem/node/common.hpp"
#include "lsystem/node/bush.hpp"

using namespace lsystem::node::common;
using namespace glm;

namespace lsystem::node::bush {
	std::shared_ptr<const Vertical> vertical = std::make_shared<Vertical>();
	std::shared_ptr<const Branch> branch = std::make_shared<Branch>();
	std::shared_ptr<const Leaf> leaf = std::make_shared<Leaf>();

	Vertical::~Vertical() {}
	Branch::~Branch() {}
	Leaf::~Leaf() {}

	const float rot = 0.5;

	std::vector<std::shared_ptr<const Node>> Vertical::grow(std::shared_ptr<const Node> self, std::minstd_rand& rng) const {
		std::vector<std::shared_ptr<const Node>> ret;

		/* V */ret.push_back(vertical);
		/* + */ret.push_back(std::make_shared<RotateZ>(0.05, 0.1));
		/* & */ret.push_back(std::make_shared<RotateY>(0.0f, 1.0));
		/* [ */ret.push_back(push);
		/* & */ret.push_back(std::make_shared<RotateY>(0.0f, 1.0));
		/* + */ret.push_back(std::make_shared<RotateZ>(rot, 0.3));
		/* L */ret.push_back(leaf);
		/* ] */ret.push_back(pop);
		/* B */ret.push_back(branch);
		/* [ */ret.push_back(push);
		/* & */ret.push_back(std::make_shared<RotateY>(0.0f, 1.0));
		/* + */ret.push_back(std::make_shared<RotateZ>(rot, 0.3));
		/* B */ret.push_back(branch);
		/* + */ret.push_back(std::make_shared<RotateZ>(rot, 0.3));
		/* L */ret.push_back(leaf);
		/* ] */ret.push_back(pop);
		/* B */ret.push_back(branch);

		return ret;
	}

	std::vector<std::shared_ptr<const Node>> Leaf::grow(std::shared_ptr<const Node> self, std::minstd_rand& rng) const {
		std::vector<std::shared_ptr<const Node>> ret;

		ret.push_back(branch);
		/* + */ret.push_back(std::make_shared<RotateZ>(rot, 0.3));
		ret.push_back(self);

		return ret;
	}

	std::vector<std::shared_ptr<const Node>> Branch::grow(std::shared_ptr<const Node> self, std::minstd_rand& rng) const {
		std::vector<std::shared_ptr<const Node>> ret;

		ret.push_back(self);

		return ret;
	}

	void Leaf::render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
		(void)trunk;
		vec4 a = stack.back().trans * vec4{0,0,0,1};
		stack.back().trans = translate(stack.back().trans, vec3{0, 0, 1.0});
		vec4 b = stack.back().trans * vec4{0,0,0,1};
		vec4 norm = normalize(b-a);
		canopy.push_index(canopy.push_vertex({a, vec3{norm}}));
	}

	void Branch::render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
		(void)canopy;
		trunk.push_index(trunk.push_vertex({{stack.back().trans * vec4{0,0,0,1}}, {0,0,1}}));
		stack.back().trans = translate(stack.back().trans, vec3{0, stack.back().size * pow(0.9, stack.back().step), 0});
		trunk.push_index(trunk.push_vertex({{stack.back().trans * vec4{0,0,0,1}}, {0,0,1}}));
		stack.back().steps->push_back(stack.back().step);
		stack.back().step += 1;
		stack.back().steps->push_back(stack.back().step);

		stack.back().size *= 1;
	}

	void Vertical::render(std::vector<node_stack> &stack, cgra::mesh_builder &trunk, cgra::mesh_builder &canopy) const {
		(void)canopy;
		trunk.push_index(trunk.push_vertex({{stack.back().trans * vec4{0,0,0,1}}, {0,0,1}}));
		stack.back().trans = translate(stack.back().trans, vec3{0, stack.back().size, 0});
		trunk.push_index(trunk.push_vertex({{stack.back().trans * vec4{0,0,0,1}}, {0,0,1}}));
		stack.back().steps->push_back(stack.back().step);
		stack.back().step += 1;
		stack.back().steps->push_back(stack.back().step);

		stack.back().size *= 1;
	}
}
