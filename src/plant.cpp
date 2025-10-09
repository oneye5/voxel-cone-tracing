#include "plant.hpp"
#include "cgra/cgra_mesh.hpp"
#include "lsystem.hpp"
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

using namespace plant;
using namespace glm;

KnownPlants plant::known_plants;

Plant::Plant(std::string seed, GLuint trunk_shader, GLuint canopy_shader, lsystem::ruleset ruleset, int steps)
	: trunk{trunk_shader}, canopy{canopy_shader}, ruleset{ruleset}, seed{seed}, current{seed} {
	grow(steps);
}

Plant::Plant(PlantData data, int steps)
	: trunk{data.trunk_shader}, canopy{data.canopy_shader}, ruleset{data.rules}, seed{data.seed}, current{seed} {
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
	canopy_mb.mode = GL_LINES;

	static float angle = 0.3;
	float size = 1;
	mat4 trans = mat4(1);
	std::vector<std::pair<mat4, float>> stack = {};
	std::cout <<"Calculating for " << current << "\n";
	for (const auto &c: current) {
		switch (c) {
			case 'A': // To-grow
				canopy_mb.push_index(canopy_mb.push_vertex({trans * vec4{0,0,0,1}, {0,0,1}}));
				trans = translate(trans, {0, size/2.0, 0});
				canopy_mb.push_index(canopy_mb.push_vertex({{trans * vec4{0,0,0,1}}, {0,0,1}}));
				size *= 0.8;

				break;
			case 'F': // Permanent growth
				trunk_mb.push_index(trunk_mb.push_vertex({{trans * vec4{0,0,0,1}}, {0,0,1}}));
				trans = translate(trans, {0, size, 0});
				trunk_mb.push_index(trunk_mb.push_vertex({{trans * vec4{0,0,0,1}}, {0,0,1}}));
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
				stack.push_back({trans, size});
				break;
			case ']': // Pop matrix
				const auto &last = stack.back();
				size = last.second;
				trans = last.first;
				stack.pop_back();
				break;
		}
	}

	trunk.mesh = trunk_mb.build();
	canopy.mesh = canopy_mb.build();
}

std::vector<Plant> plant::create_plants(std::vector<create_plants_input> inputs) {
	std::vector<Plant> ret;

	for (auto pt : inputs) {
		Plant p = Plant(known_plants.tree);
		p.trunk.modelTransform = translate(p.trunk.modelTransform, pt.pos);
		p.canopy.modelTransform = translate(p.canopy.modelTransform, pt.pos);
		ret.push_back(p);
	}

	return ret;
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
	mesh.draw();
}
