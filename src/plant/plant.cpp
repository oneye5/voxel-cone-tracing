#include "plant.hpp"
#include "cgra/cgra_mesh.hpp"
#include "lsystem.hpp"
#include "plant/data.hpp"
#include "opengl.hpp"
#include <ostream>
#include <random>
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

using namespace plant;
using namespace glm;

Plant::Plant(lsystem::ruleset current, GLuint trunk_shader, GLuint canopy_shader, unsigned long rng_seed, int steps) :
	rng{rng_seed},
	current{current},
	size{0.5},
	trunk{trunk_shader, 0, 0},
	canopy{canopy_shader, 0, 0} {
	grow(steps);
}

Plant::Plant(data::PlantData data, int steps) :
		current{data.initial}, size{data.size},
		trunk{data.trunk_shader, data.trunk_texture_colour, data.trunk_texture_normal},
		canopy{data.canopy_shader, data.canopy_texture_colour, data.canopy_texture_normal} {
	grow(steps);
}

void Plant::grow(int steps) {
	current = lsystem::iterate(current, rng, steps);
	recalculate_mesh();
}

void Plant::recalculate_mesh() {
	cgra::mesh_builder trunk_mb;
	trunk_mb.mode = GL_LINES;
	cgra::mesh_builder canopy_mb;
	canopy_mb.mode = GL_POINTS;

	float step = 1;
	static float angle = 0.3;
	mat4 trans = mat4(1);
	std::vector<float> steps;
	std::vector<lsystem::node::node_stack> stack = {{trans, size, step, &steps}};

	for (auto &n: current) {
		n->render(stack, trunk_mb, canopy_mb);
	}

	trunk.mesh = trunk_mb.build();

	if (canopy_mb.vertices.size() <= 0) {
		canopy_mb.push_index(canopy_mb.push_vertex({{0,-10000,0}}));
	}

	canopy.mesh = canopy_mb.build();

	glGenBuffers(1, &trunk.alt_vbo);
	glBindVertexArray(trunk.mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, trunk.alt_vbo);
	glBufferData(GL_ARRAY_BUFFER, steps.size() * sizeof(float), &steps[0], GL_STATIC_DRAW);
	// 0, 1, 2 are taken
	glEnableVertexAttribArray(4);
	
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void *)0);
	glBindVertexArray(0);
}

PlantManager::PlantManager(Renderer* renderer) : renderer{renderer} {}
PlantManager::PlantManager() {}
void PlantManager::clear() {
	// Assuming that the item at the back is the one with the largest index
	for (auto it = plants.rbegin(); it != plants.rend(); it++) {
		renderer->renderables.erase(renderer->renderables.begin() + it->first + 1);
		renderer->renderables.erase(renderer->renderables.begin() + it->first);
	}
	plants.clear();
}

void PlantManager::update_plants(const std::vector<plants_manager_input>& inputs) {
	this->clear();
	std::vector<Plant> temp_plants;

	std::minstd_rand rng;

	for (auto pt : inputs) {
		data::PlantData *data;
		switch (pt.type) {
			case 0:
				if (std::generate_canonical<float, 10>(rng) < 0.5) {
					data = &data::known_plants.tree;
				} else {
					data = &data::known_plants.bush;
					pt.pos += vec3{0,0.2, 0};
				}
				break;
			case 1:
				// Tree
				data = &data::known_plants.tree;
				break;
			case 2:
				// Tree
				data = &data::known_plants.bush;
				break;
		}
		Plant p = Plant(*data);
		p.trunk.modelTransform = translate(p.trunk.modelTransform, pt.pos);
		p.canopy.modelTransform = translate(p.canopy.modelTransform, pt.pos);
		temp_plants.push_back(p);
	}

	// This is really unsafe but lol
	auto base = renderer->renderables.size();
	plants.reserve(temp_plants.size());
	for (auto plant : temp_plants) {
		plants.push_back({base, plant});
		renderer->addRenderable(&plants.back().second.canopy);
		renderer->addRenderable(&plants.back().second.trunk);
		base += 2;
	}
}

void PlantManager::grow(int step) {
	for (auto& plant : plants) {
		plant.second.grow(step);
	}
}
