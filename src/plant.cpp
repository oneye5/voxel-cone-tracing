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

Plant::Plant(std::string seed, lsystem::ruleset ruleset, int steps, GLuint shader)
	: mesh{}, shader{shader}, ruleset{ruleset}, seed{seed}, current{seed} {
	grow(steps);
}

void Plant::grow(int steps) {
	current = lsystem::iterate(current, ruleset, steps);
	recalculate_mesh();
}

void Plant::recalculate_mesh() {
	cgra::mesh_builder mb;
	static float angle = 0.3;
	float size = 1;
	mb.mode = GL_LINES;
	mat4 trans = mat4(1);
	std::vector<std::pair<mat4, float>> stack = {};
	std::cout <<"Calculating for " << current << "\n";
	for (const auto &c: current) {
		switch (c) {
			case 'A': // To-grow
				mb.push_index(mb.push_vertex({{trans * vec4{0,0,0,1}}}));
				trans = translate(trans, {0, size/2.0, 0});
				mb.push_index(mb.push_vertex({{trans * vec4{0,0,0,1}}}));
				size *= 0.8;

				break;
			case 'F': // Permanent growth
				mb.push_index(mb.push_vertex({{trans * vec4{0,0,0,1}}}));
				trans = translate(trans, {0, size, 0});
				mb.push_index(mb.push_vertex({{trans * vec4{0,0,0,1}}}));
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

	mesh = mb.build();
}

void Plant::draw(const glm::mat4 &modelTransform, const glm::mat4 &view, const glm::mat4 proj) {
	mat4 modelview = view * modelTransform;
	
	glUseProgram(shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3 {1.0, 1.0, 1.0}));

	mesh.draw();
}
