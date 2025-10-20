
#include "BaseTerrain.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_mesh.hpp"
#include "cgra/cgra_shader.hpp"
#include "glm/detail/type_vec.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <imgui.h>
#include <print>
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

	t_mesh.updateTransformCentered(t_settings.model_scale);
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
		stepErosion();
	}

	glUseProgram(shader);
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3{1, 1, 1}));

	glUniform1f(glGetUniformLocation(shader, "max_height"), t_settings.max_height);
	glUniform1i(glGetUniformLocation(shader, "useTexturing"), useTexturing);
	glUniform1i(glGetUniformLocation(shader, "useFakedLighting"), useFakedLighting);
	glUniform1i(glGetUniformLocation(shader, "subdivisions"), plane_subs);
	glUniform1f(glGetUniformLocation(shader, "amplitude"), t_settings.amplitude);
	glUniform1i(glGetUniformLocation(shader, "draw_from_min"), draw_from_min);
	glUniform1f(glGetUniformLocation(shader, "min_height"), t_noise.min_height);

	glUniform1f(glGetUniformLocation(shader, "min_rock_slope"), t_settings.min_rock_slope);
	glUniform1f(glGetUniformLocation(shader, "max_grass_slope"), t_settings.max_grass_slope);

	glUniform1f(glGetUniformLocation(shader, "terrain_size_scalar"), t_settings.model_scale.x);
	glUniform1i(glGetUniformLocation(shader, "use_triplanar_mapping"), t_settings.use_triplanar_mapping);
	glUniform1f(glGetUniformLocation(shader, "tex_base_scalar"), t_settings.tex_base_scalar);
	glUniform1f(glGetUniformLocation(shader, "triplanar_sharpness"), t_settings.triplanar_sharpness);
	
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
	glBindTexture(GL_TEXTURE_2D, dirt_texture);
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
	cgra::mesh_builder mb = cgra::CREATE_PLANE(x_sub, z_sub, 2.0f);
	
	plane.mesh = mb.build();
	return plane;
}

void PlaneTerrain::updateTransformCentered(vec3 scale) {
	constexpr vec3 original_center = vec3(1.0f, 0.0f, 1.0f);

	vec3 scaled_center = original_center * scale;

	mat4 m_translation = glm::translate(mat4(1.0f), -scaled_center);
	mat4 m_scale = glm::scale(mat4(1.0f), scale);

	init_transform = m_translation * m_scale;
}

void BaseTerrain::renderUI() {
	
	ImGui::SetNextWindowPos(ImVec2(5, 350), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_Once);
	ImGui::Begin("Terrain Settings", 0);

	if (ImGui::SliderFloat3("Terrain Scale", value_ptr(t_settings.model_scale), 1.0f, 20.0f)) {
		t_mesh.updateTransformCentered(t_settings.model_scale);
	}

	ImGui::SliderFloat("Amplitude", &t_settings.amplitude, 0.01f, 3.0f);
	ImGui::Checkbox("Draw from min height", &draw_from_min);
	ImGui::Checkbox("Use texturing", &useTexturing);
	//ImGui::Checkbox("Use faked lighting", &useFakedLighting);

	if (ImGui::SliderInt("Plane Subdivisions", &plane_subs, 64, 1024)) {
		changePlaneSubdivision(plane_subs);
	}

	ImGui::Text("Texturing settings");
	ImGui::Checkbox("Use Triplanar Mapping", &t_settings.use_triplanar_mapping);
	ImGui::SliderFloat("Triplanar Sharpness", &t_settings.triplanar_sharpness, 0.01f, 4.0f);
	ImGui::SliderFloat("Texture coordinate scalar (non triplanar)", &t_settings.tex_base_scalar, 1.0f, 20.0f);
	ImGui::SliderFloat("Min Rock Slope", &t_settings.min_rock_slope, 0.0f, t_settings.max_grass_slope-0.001f);
	ImGui::SliderFloat("Max Grass Slope", &t_settings.max_grass_slope, 0.0f, 1.0f);

	if (water_plane) {
		ImGui::Text("Water settings");
		if (ImGui::SliderFloat("Sea Level", &t_settings.sea_level, 0.0f, 5.0f)) {
			water_plane->update_transform(vec3(t_settings.model_scale), t_settings.sea_level);
		}
		if (ImGui::DragFloat("Sea size scalar", &water_plane->size_scalar, 0.01f, 2.5f)) {
			water_plane->update_transform(vec3(t_settings.model_scale), t_settings.sea_level);
		}
		if (ImGui::Button("Make Water Reflective (FPS Heavy)")) {
			water_plane->smoothness = WaterPlane::SHINY_SMOOTHNESS;
		}
		ImGui::DragFloat("Wave Speed", &water_plane->wave_speed, 0.0001f, 0.001f, 0.5f, "%.5f");
		ImGui::SliderFloat("Water metallicness", &water_plane->metallic, 0.0f, 1.0f);
		ImGui::SliderFloat("Water smoothness (fps heavy)", &water_plane->smoothness, 0.0f, 1.0f);
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Noise settings", ImGuiTreeNodeFlags_DefaultOpen)) {
		t_noise.makeEditUI();
	}

	ImGui::Separator();

	ImGui::Text("Tree Placement Controls");
	// TODO - maybe add some kind of like "auto update flag" to auto calculate new positions and send them to the tree object
	ImGui::SliderFloat("Minimum distance apart", &tree_settings.min_distance, 0.05f, 20.0f);
	if (ImGui::InputInt("Max tree amount", &tree_settings.max_trees)) {
		// No negative numbers
		tree_settings.max_trees *= tree_settings.max_trees > 0;
	};
	ImGui::SliderInt("Max Placement Attempts", &tree_settings.placement_attempts, 1, 200);

	if (ImGui::Button("Calculate tree positions")) {
		calculateAndSendTreePlacements();
	}

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
		float fraction = static_cast<float>(t_erosion.iterations_ran) / static_cast<float>(t_erosion.settings.iterations);
		ImGui::ProgressBar(fraction, ImVec2(0.0f, 0.0f));
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

	water_texture = Textures::water;
	sand_texture = Textures::sand;
	dirt_texture = Textures::grass;
	rock_texture = Textures::rock;
	snow_texture = Textures::snow;
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

// TODO - implement the sending to the plant manager object thing
void BaseTerrain::calculateAndSendTreePlacements(const int seed) {
	std::mt19937 rng = (seed == -1) ? std::mt19937(std::random_device()()) : std::mt19937(seed);
	std::uniform_real_distribution<float> uniform_dist(0.0f, 1.0f); // Calculate between 0 and 1 and map to actual position

	std::vector<plant::plants_manager_input> positions;
	positions.reserve(tree_settings.max_trees); // Might not manage to use max trees but hopefully should get somewhere there

	for (int i = 0; i < tree_settings.placement_attempts && positions.size() < tree_settings.max_trees; i++) {
		const float n_x = uniform_dist(rng);
		const float n_z = uniform_dist(rng);

		vec3 new_pos = normalizedXZToWorldPos({n_x, n_z});

		// Check that new position doesn't collide with existing
		bool valid = true;
		for (const plant::plants_manager_input& p: positions) {
			float dist = glm::distance(new_pos, p.pos);
			if (dist < tree_settings.min_distance) {
				valid = false;
				break;
			}
		}

		if (valid) {
			positions.push_back({new_pos});
		}
	}

	sendTreePlacements(positions);
}

vec3 BaseTerrain::normalizedXZToWorldPos(const vec2 &n_pos) {
	const float scaled_x = (n_pos.x * 2.0f) * t_settings.model_scale.x;
	const float scaled_z = (n_pos.y * 2.0f) * t_settings.model_scale.z;

	const float world_x = scaled_x - t_settings.model_scale.x;
	const float world_z = scaled_z - t_settings.model_scale.z;
	
	float y_pos = approximateYAtPoint(n_pos);
	vec3 pos{world_x, y_pos, world_z};
	return pos;
}

float BaseTerrain::approximateYAtPoint(const vec2 &pos) {
	const float Y_REDUCTION = 0.1f * t_settings.amplitude; // Amount to reduce the calculated y by, to avoid trees starting above ground
	const int scaled_x = static_cast<int>(fminf(roundf(t_noise.width * pos.x), t_noise.width-1));
	const int scaled_z = static_cast<int>(fminf(roundf(t_noise.height * pos.y), t_noise.height-1));

	const float norm_height = t_noise.heightmap.at(scaled_z * t_noise.width + scaled_x);
	float height = (norm_height - Y_REDUCTION) * t_settings.model_scale.y * t_settings.amplitude;
	if (draw_from_min) {height = height - t_noise.min_height;}
	return height;
}

void BaseTerrain::sendTreePlacements(std::vector<plant::plants_manager_input> &positions) {
	for (auto& p: positions) {
		std::cout << p << std::endl;
		// std::print("{:.10f},  {:.10f},  {:.10f}\n", p.pos.x, p.pos.y, p.pos.z);
	}

	plant_manager->update_plants(positions);
}
