
#include "BaseTerrain.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_mesh.hpp"
#include "cgra/cgra_shader.hpp"
#include "glm/detail/type_vec.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <imgui.h>
#include "opengl.hpp"

using namespace Terrain;
using namespace glm;

static PlaneTerrain CreateBasicPlane(int x_sub, int z_sub);

BaseTerrain::BaseTerrain() {
	t_mesh = CreateBasicPlane(512, 512);
	cgra::shader_builder sb;
	sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//terrain//basic_terrain.vs"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//terrain//basic_terrain.fs"));
	shader = sb.build();

	t_mesh.init_transform = glm::scale(mat4(1.0f), glm::vec3(5));
	loadTextures();

	// Set up the texture uniforms cuz only need to do once
	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "heightMap"), 0);
	glUniform1i(glGetUniformLocation(shader, "water_texture"), 1);
	glUniform1i(glGetUniformLocation(shader, "sand_texture"), 2);
	glUniform1i(glGetUniformLocation(shader, "grass_texture"), 3);
	glUniform1i(glGetUniformLocation(shader, "rock_texture"), 4);
	glUniform1i(glGetUniformLocation(shader, "snow_texture"), 5);
}

void BaseTerrain::setProjViewUniforms(const glm::mat4 &view, const glm::mat4 &proj) const {
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelMatrix"), 1, false, value_ptr(t_mesh.init_transform));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uViewMatrix"), 1, false, value_ptr(view));
}


void BaseTerrain::draw() {
	glUseProgram(shader);
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3{1, 1, 1}));

	glUniform1f(glGetUniformLocation(shader, "max_height"), t_settings.max_height);
	glUniform1f(glGetUniformLocation(shader, "min_height"), t_settings.min_height);
	glUniform1i(glGetUniformLocation(shader, "useTexturing"), useTexturing);
	glUniform1i(glGetUniformLocation(shader, "useFakedLighting"), useFakedLighting);
	glUniform1i(glGetUniformLocation(shader, "subdivisions"), plane_subs);
	glUniform1f(glGetUniformLocation(shader, "amplitude"), t_settings.amplitude);
	
	glActiveTexture(GL_TEXTURE0);
	// glUniform1i(glGetUniformLocation(shader, "heightMap"), 0);
	glBindTexture(GL_TEXTURE_2D, t_noise.texID);

	// Water
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, water_texture);
	// Sand
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, sand_texture);
	// Grass
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, grass_texture);
	// Rock
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, rock_texture);
	// Snow
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, snow_texture);

	t_mesh.mesh.draw();
}

void BaseTerrain::changePlaneSubdivision(int subs) {
	PlaneTerrain new_t = CreateBasicPlane(subs, subs);
	t_mesh.mesh = new_t.mesh;
}


// Creates and returns a new PlaneTerrain by just constructing a plane from 0,0,0 to 1,0,1
// with provided subdivisions
static PlaneTerrain CreateBasicPlane(int x_sub, int z_sub) {
	PlaneTerrain plane;
	cgra::mesh_builder mb;
	
	const float x_step = 1.0f / (float)x_sub;
	const float z_step = 1.0f / (float)z_sub;
	
	for (float x = 0.0; x <= 1.0; x += x_step) {
		for (float z = 0.0; z <= 1.0; z += z_step) {
			mb.push_vertex({{x, 0.0, z}, {0.0, 1.0, 0.0}, {x, z}});
		}
	}

	for (int z = 0; z < z_sub; z++) {
		for (int x = 0; x < x_sub; x++) {
			GLuint tl = z * (x_sub + 1) + x; // top left
			GLuint tr = tl + 1;
			GLuint bl = tl + (x_sub + 1);
			GLuint br = bl + 1;

			mb.push_indices({tl, tr, bl});
			mb.push_indices({bl, tr, br});
		}
	}

	plane.mesh = mb.build();
	return plane;
}

void BaseTerrain::renderUI() {
	
	ImGui::SetNextWindowPos(ImVec2(5, 200), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_Once);
	ImGui::Begin("Terrain Settings", 0);

	ImGui::SliderFloat("Max Height", &t_settings.max_height, 0.20, 2.0);
	ImGui::SliderFloat("Min Height", &t_settings.min_height, 0.0f, 1.0f);
	ImGui::SliderFloat("Amplitude", &t_settings.amplitude, 0.01f, 3.0f);
	ImGui::Checkbox("Use procedural texturing", &useTexturing);
	ImGui::Checkbox("Use faked lighting", &useFakedLighting);

	if (ImGui::SliderInt("Plane Subdivisions", &plane_subs, 64, 1024)) {
		changePlaneSubdivision(plane_subs);
	}

	t_noise.makeEditUI();

	ImGui::End();
}

void BaseTerrain::loadTextures() {
	static const std::string WATER_PATH = CGRA_SRCDIR + std::string("//res//textures//terrain//tex_Water.jpg");
	static const std::string SAND_PATH = CGRA_SRCDIR + std::string("//res//textures//terrain//sand_2.png");
	static const std::string GRASS_PATH = CGRA_SRCDIR + std::string("//res//textures//terrain//grass_2.jpg");
	static const std::string ROCK_PATH = CGRA_SRCDIR + std::string("//res//textures//terrain//rock.jpg");
	static const std::string SNOW_PATH = CGRA_SRCDIR + std::string("//res//textures//terrain//snow.jpg");

	water_texture = cgra::rgba_image(WATER_PATH).uploadTexture();
	grass_texture = cgra::rgba_image(GRASS_PATH).uploadTexture();
	sand_texture = cgra::rgba_image(SAND_PATH).uploadTexture();
	rock_texture = cgra::rgba_image(ROCK_PATH).uploadTexture();
	snow_texture = cgra::rgba_image(SNOW_PATH).uploadTexture();
}

GLuint BaseTerrain::getShader() {
	return shader;
}

mat4 BaseTerrain::getModelTransform() {
	return t_mesh.init_transform;
}
