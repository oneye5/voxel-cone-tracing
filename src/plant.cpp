#include "plant.hpp"
#include "cgra/cgra_mesh.hpp"
#include "lsystem.hpp"
#include <string>
#include <glm/gtc/type_ptr.hpp>

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
	// Just for debugging atm
	mb.push_index(mb.push_vertex({{0,0, 0}}));
	mb.push_index(mb.push_vertex({{0,10, 0}}));
	mb.push_index(mb.push_vertex({{0,0, 10}}));

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
