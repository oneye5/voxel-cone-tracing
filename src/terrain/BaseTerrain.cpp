
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

// TODO - not the nicest, maybe redo this lmao (why do i have to redefine them here...)
GLuint Textures::water = 0;
GLuint Textures::sand = 0;
GLuint Textures::grass = 0;
GLuint Textures::rock = 0;
GLuint Textures::snow = 0;

static PlaneTerrain CreateBasicPlane(int x_sub, int z_sub);

BaseTerrain::BaseTerrain() : t_erosion(t_noise.width, t_noise.height) {
	t_mesh = CreateBasicPlane(512, 512);
	cgra::shader_builder sb;
	sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//terrain//basic_terrain.vs"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//terrain//basic_terrain.fs"));
	shader = sb.build();

	t_mesh.init_transform = glm::translate(glm::mat4(1), glm::vec3(-5,0,-5));
	t_mesh.init_transform = glm::scale(t_mesh.init_transform, glm::vec3(DEFAULT_TERRAIN_SCALE, DEFAULT_TERRAIN_SCALE / 2, DEFAULT_TERRAIN_SCALE));
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
	if (erosion_running) {
		// TODO - check that running erosion sim here is fine, can do in UI render instead.
		stepErosion();
	}

	glUseProgram(shader);
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3{1, 1, 1}));

	glUniform1f(glGetUniformLocation(shader, "max_height"), t_settings.max_height);
	glUniform1f(glGetUniformLocation(shader, "min_height"), t_settings.min_height);
	glUniform1i(glGetUniformLocation(shader, "useTexturing"), useTexturing);
	glUniform1i(glGetUniformLocation(shader, "useFakedLighting"), useFakedLighting);
	glUniform1i(glGetUniformLocation(shader, "subdivisions"), plane_subs);
	glUniform1f(glGetUniformLocation(shader, "amplitude"), t_settings.amplitude);

	glUniform1f(glGetUniformLocation(shader, "min_rock_slope"), t_settings.min_rock_slope);
	glUniform1f(glGetUniformLocation(shader, "max_grass_slope"), t_settings.max_grass_slope);
	
	glActiveTexture(GL_TEXTURE0);
	// glUniform1i(glGetUniformLocation(shader, "heightMap"), 0);
	glBindTexture(GL_TEXTURE_2D, t_noise.texID);

	// Water
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// Sand
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	// Grass
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture3);
	// Rock
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, texture4);
	// Snow
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, texture5);

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
	cgra::mesh_builder mb = cgra::CREATE_PLANE(x_sub, z_sub);
	
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

	if (water_plane && ImGui::SliderFloat("Sea Level", &t_settings.sea_level, 0.0f, 5.0f)) {
		water_plane->update_transform(vec3(DEFAULT_TERRAIN_SCALE), t_settings.sea_level);
	}

	ImGui::Text("Texturing settings");
	ImGui::SliderFloat("Min Rock Slope", &t_settings.min_rock_slope, 0.0f, t_settings.max_grass_slope-0.001f);
	ImGui::SliderFloat("Max Grass Slope", &t_settings.max_grass_slope, 0.0f, 1.0f);

	t_noise.makeEditUI();

	ImGui::Separator();

	// Erosion controls
	t_erosion.renderUI();

	if (ImGui::Button("Apply Erosion")) { // stackable
		applyErosion();
	}
	ImGui::SameLine();
	if (ImGui::Button("Regenerate terrain and erode")) {
		t_noise.generateHeightmap(false);
		applyErosion();
	}

	// Real-time erosion stuff
	if (!erosion_running) {
		if (ImGui::Button("Start real-time erosion")) {
			erosion_running = true;
			t_erosion.newSimulation(t_noise.heightmap, t_noise.width, t_noise.height);
		}
	} else {
		ImGui::Text("Erosion sim running..");
		ImGui::Text("Currently on iteration %d / %d", t_erosion.iterations_ran, t_erosion.settings.iterations);
		if (ImGui::Button("Abort")) {
			erosion_running = false;
		}
	}

	ImGui::End();
}

void BaseTerrain::loadTextures() {
	static const std::string WATER_PATH = CGRA_SRCDIR + std::string("//res//textures//terrain//tex_Water.jpg");
	static const std::string SAND_PATH = CGRA_SRCDIR + std::string("//res//textures//terrain//sand_2.png");
	static const std::string GRASS_PATH = CGRA_SRCDIR + std::string("//res//textures//terrain//grass_3.png");
	static const std::string ROCK_PATH = CGRA_SRCDIR + std::string("//res//textures//terrain//rock.jpg");
	static const std::string SNOW_PATH = CGRA_SRCDIR + std::string("//res//textures//terrain//snow.jpg");

	Textures::water = cgra::rgba_image(WATER_PATH).uploadTexture();
	Textures::sand = cgra::rgba_image(SAND_PATH).uploadTexture();
	Textures::grass = cgra::rgba_image(GRASS_PATH).uploadTexture();
	Textures::rock = cgra::rgba_image(ROCK_PATH).uploadTexture();
	Textures::snow = cgra::rgba_image(SNOW_PATH).uploadTexture();

	texture1 = Textures::water;
	texture2 = Textures::sand;
	texture3 = Textures::grass;
	texture4 = Textures::rock;
	texture5 = Textures::snow;
}

GLuint BaseTerrain::getShader() {
	return shader;
}

mat4 BaseTerrain::getModelTransform() {
	return t_mesh.init_transform;
}

// Get the heightmap from noise, apply erosion and then update the heightmap texture
void BaseTerrain::applyErosion() {
	t_erosion.newSimulation(t_noise.heightmap, t_noise.width, t_noise.height);
	t_erosion.simulate();
	t_noise.setHeightmap(t_erosion.getHeightmap());
}

void BaseTerrain::stepErosion() {
	t_erosion.stepSimulation();
	t_noise.setHeightmap(t_erosion.getHeightmap());
	if (t_erosion.iterations_ran >= t_erosion.settings.iterations) {
		erosion_running = false;
	}
}

