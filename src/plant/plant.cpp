#include "plant.hpp"
#include "cgra/cgra_mesh.hpp"
#include "lsystem.hpp"
#include "opengl.hpp"
#include <ostream>
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

using namespace plant;
using namespace glm;

KnownPlants plant::known_plants;
bool planttt = true;

Plant::Plant(std::string seed, GLuint trunk_shader, GLuint canopy_shader, lsystem::ruleset ruleset, int steps) :
	ruleset{ruleset},
	seed{seed},
	current{seed},
	trunk{trunk_shader},
	canopy{canopy_shader} {
	grow(steps);
}

Plant::Plant(PlantData data, int steps) :
		ruleset{data.rules},
		seed{data.seed},
		current{seed},
		trunk{data.trunk_shader},
		canopy{data.canopy_shader} {
	grow(steps);
}

void Plant::grow(int steps) {
	current = lsystem::iterate(current, ruleset, steps);
	recalculate_mesh();
}

void Plant::recalculate_mesh() {
	cgra::mesh_builder trunk_mb;
	trunk_mb.mode = GL_LINES;
	cgra::mesh_builder canopy_mb;
	canopy_mb.mode = GL_POINTS;

	float step = 1;
	static float angle = 0.3;
	float size = 1;
	mat4 trans = mat4(1);
	struct stackItem {
		mat4 trans;
		float size;
		float step;
	};
	std::vector<float> steps;
	std::vector<stackItem> stack = {};
	for (const auto &c: current) {
		switch (c) {
			case 'A': // To-grow
				{
					vec4 a = trans * vec4{0,0,0,1};
					trans = translate(trans, {0, size/2.0, 0});
					vec4 b = trans * vec4{0,0,0,1};
					vec4 norm = normalize(b-a);
					canopy_mb.push_index(canopy_mb.push_vertex({a, vec3{norm}}));
					// canopy_mb.push_index(canopy_mb.push_vertex({{trans * vec4{0,0,0,1}}, {0,0,1}}));
					size *= 0.8;
				}

				break;
			case 'F': // Permanent growth
				trunk_mb.push_index(trunk_mb.push_vertex({{trans * vec4{0,0,0,1}}, {0,0,1}}));
				trans = translate(trans, {0, size, 0});
				trunk_mb.push_index(trunk_mb.push_vertex({{trans * vec4{0,0,0,1}}, {0,0,1}}));
				steps.push_back(step);
				step += 1;
				steps.push_back(step);
				size *= 0.8;

				break;
			case '-': // Rot back Z
				trans = rotate(trans, -angle, {0,0,1});
				break;
			case '+': // Rot forward Z
				trans = rotate(trans, angle, {0,0,1});
				break;

			case '^': // Rot back X
				trans = rotate(trans, -angle, {1,0,0});
				break;
			case '&': // Rot forward X
				trans = rotate(trans, angle, {1,0,0});
				break;
			case '!': // Rot back Y
				trans = rotate(trans, -angle, {0,1,0});
				break;
			case '?': // Rot forward Y
				trans = rotate(trans, angle, {0,1,0});
				break;
			case '[': // Push matrix
				stack.push_back({trans, size, step});
				break;
			case ']': // Pop matrix
				const auto &last = stack.back();
				size = last.size;
				trans = last.trans;
				step = last.step;
				stack.pop_back();
				break;
		}
	}

	trunk.mesh = trunk_mb.build();
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

// - mesh
Mesh::Mesh() : mesh{}, shader{0}, modelTransform{1} {}

Mesh::Mesh(GLuint shader) : mesh{}, shader{shader}, modelTransform{1} {}

GLuint Mesh::getShader() {
	return shader;
}

mat4 Mesh::getModelTransform() {
	return modelTransform;
}

void Mesh::setProjViewUniforms(const glm::mat4& view, const glm::mat4& proj) const {
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uViewMatrix"), 1, false, value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelMatrix"), 1, false, value_ptr(modelTransform));
}

void Mesh::draw() {
	// TODO: textures or whatever
	if (!planttt) return;
	mesh.draw();
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

	for (auto pt : inputs) {
		Plant p = Plant(known_plants.tree);
		// Clip into the ground to avoid weird stuff
		pt.pos -= vec3{0,0.1, 0};
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
