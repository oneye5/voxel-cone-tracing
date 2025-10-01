
#include "WaterPlane.hpp"

#include "cgra/cgra_image.hpp"
#include "cgra/cgra_mesh.hpp"
#include "cgra/cgra_shader.hpp"
#include "glm/detail/type_vec.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace Terrain;
using namespace glm;

WaterPlane::WaterPlane(GLuint texid) : plane_mesh(cgra::CREATE_PLANE(256, 256).build()), water_texture(texid) {
	if (!water_texture) { // Water texture not supplied
		water_texture = cgra::rgba_image(WATER_TEXTURE_PATH).uploadTexture();
	}

	if (!shader) {
		cgra::shader_builder sb;
		sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//terrain//water_plane.vs"));
		sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//terrain//water_plane.fs"));
		shader = sb.build();
	}

	update_transform({5.0, 5.0, 5.0}, 1.0f);
}

void WaterPlane::update_transform(glm::vec3 model_scale, float sea_level) {
	mat4 m_scale = glm::scale(mat4(1.0f), model_scale);
	mat4 m_trans = glm::translate(mat4(1.0f), {0.0, sea_level, 0.0});

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
	plane_mesh.draw();
}

glm::mat4 WaterPlane::getModelTransform() {
	return model_transform;
}
