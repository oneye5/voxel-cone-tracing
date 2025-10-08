
#include "WaterPlane.hpp"

#include "cgra/cgra_image.hpp"
#include "cgra/cgra_mesh.hpp"
#include "cgra/cgra_shader.hpp"
#include "glm/detail/type_vec.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace Terrain;
using namespace glm;

WaterPlane::WaterPlane(GLuint texid) : plane_mesh(cgra::CREATE_PLANE(256, 256, 2.0f).build()), water_texture(texid) {
	if (!water_texture) { // Water texture not supplied
		water_texture = cgra::rgba_image(WATER_TEXTURE_PATH).uploadTexture();
	}

	if (!water_normal_texture) {
		water_normal_texture = cgra::rgba_image(WATER_NORMAL_TEX_PATH).uploadTexture();
	}

	if (!shader) {
		cgra::shader_builder sb;
		sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//terrain//water_plane.vs"));
		sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//terrain//water_plane.fs"));
		shader = sb.build();
	}

	update_transform({5.0f, 2.5f, 5.0f}, 0.0f);

	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "water_texture"), 0);
	glUniform1i(glGetUniformLocation(shader, "water_normal_texture"), 1);
}

void WaterPlane::update_transform(glm::vec3 model_scale, float sea_level) {
	mat4 m_scale = glm::scale(mat4(1.0f), model_scale);
	vec3 trans = vec3(-2.0f - (model_scale.x / 2.0f), sea_level, -2.0f - (model_scale.z / 2.0f));
	mat4 m_trans = glm::translate(mat4(1.0f), trans);

	model_transform = m_trans * m_scale;
}

GLuint WaterPlane::getShader() {
	return shader;
}

void WaterPlane::setProjViewUniforms(const glm::mat4 &view, const glm::mat4 &proj) const {
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelMatrix"), 1, false, value_ptr(model_transform));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uViewMatrix"), 1, false, value_ptr(view));
}

void WaterPlane::draw() {
	glUseProgram(shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, water_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, water_normal_texture);

	glUniform1f(glGetUniformLocation(shader, "metallic"), metallic);
	glUniform1f(glGetUniformLocation(shader, "smoothness"), smoothness);

	plane_mesh.draw();
}

glm::mat4 WaterPlane::getModelTransform() {
	return model_transform;
}
