#include "plant/mesh.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "cgra/cgra_mesh.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

using namespace plant;
using namespace glm;
bool planttt = true;

Mesh::Mesh() : mesh{}, shader{0}, modelTransform{1} {}

Mesh::Mesh(GLuint shader, GLuint colour, GLuint normal) : mesh{}, shader{shader}, modelTransform{1}, colour_texture{colour}, normal_texture{normal} {}

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

	glUseProgram(shader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colour_texture);
	glUniform1i(glGetUniformLocation(shader, "colourTexture"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal_texture);
	glUniform1i(glGetUniformLocation(shader, "normalTexture"), 1);

	mesh.draw();
}
