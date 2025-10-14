// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "application.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"
#include <renderer.hpp>
#include <example_renderable.cpp>
#include <point_light_renderable.cpp>

#include "lsystem.hpp"
#include "plant.hpp"

#include "terrain/BaseTerrain.hpp"
#include "terrain/WaterPlane.hpp"
#include <cubeRenderable.cpp>


using namespace std;
using namespace cgra;
using namespace glm;


extern bool planttt ;
glm::vec3 lightPos;
glm::vec3 lightScale;
bool dirtyVoxels = true;
Renderer* renderer = nullptr;

// scene 0, (this is ugly but it works)
Terrain::BaseTerrain* t_terrain = nullptr;
ExampleRenderable* exampleRenderable = nullptr;
ExampleRenderable* exampleRenderable2 = nullptr;
Terrain::WaterPlane* t_water = nullptr;

// scene 1
CubeRenderable* floor1 = nullptr;
CubeRenderable* ceiling = nullptr;
CubeRenderable* backWall = nullptr;
CubeRenderable* leftWall = nullptr;
CubeRenderable* rightWall = nullptr;
CubeRenderable* cube = nullptr;
vector<plant::Plant> plants;
ExampleRenderable* exampleRenderable3 = nullptr;

// shared
PointLightRenderable* light = nullptr;

void resetScene() {
	//for (int i = 0; i < renderer->renderables.size(); i++)   
		//delete renderer->renderables.at(i);
	// keep stuff in memory, otherwise things break, this is bad and ugly and terrible but thats okay...
	renderer->renderables.clear();
	dirtyVoxels = true;
	renderer->lightingPass->setDefaultParams();
}

void loadScene0() {
	resetScene();

	t_terrain = new Terrain::BaseTerrain();
	t_water = new Terrain::WaterPlane();
	t_terrain->water_plane = t_water;
	light = new PointLightRenderable();
	exampleRenderable = new ExampleRenderable();
	exampleRenderable2 = new ExampleRenderable();

	// Initialise plant data
	plant::known_plants.tree.seed = "A";
	plant::known_plants.tree.rules = { {'A', "F[&[+A][--A]]???[^[+A][--A]]"} };
	{
		cgra::shader_builder sb;
		sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//plant_trunk_vert.glsl"));
		sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//plant_trunk_frag.glsl"));
		sb.set_shader(GL_GEOMETRY_SHADER, CGRA_SRCDIR + std::string("//res//shaders//plant_trunk_geom.glsl"));
		plant::known_plants.tree.trunk_shader = sb.build();
	}
	{
		cgra::shader_builder sb;
		sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//plant_canopy_vert.glsl"));
		sb.set_shader(GL_GEOMETRY_SHADER, CGRA_SRCDIR + std::string("//res//shaders//plant_canopy_geom.glsl"));
		sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//plant_canopy_frag.glsl"));
		plant::known_plants.tree.canopy_shader = sb.build();
	}
	//---

	// modifactions
	lightPos = glm::vec3(-0, 10, -0);
	lightScale = glm::vec3(2, 0.5, 2);
	light->modelTransform = glm::translate(glm::mat4(1), lightPos);
	light->modelTransform = glm::scale(light->modelTransform, lightScale);
	exampleRenderable->modelTransform = glm::translate(glm::mat4(1), glm::vec3(0.5, 4, 0.5));
	exampleRenderable->modelTransform = glm::scale(exampleRenderable->modelTransform, vec3(0.3));
	exampleRenderable2->mesh = cgra::load_wavefront_data(CGRA_SRCDIR + std::string("//res//assets//axis.obj")).build();
	exampleRenderable2->modelTransform = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
	exampleRenderable2->modelTransform = glm::scale(exampleRenderable2->modelTransform, vec3(0.2, 0.2, -0.2));

	// add renderables
	renderer->addRenderable(t_terrain);
	renderer->addRenderable(t_water);
	renderer->addRenderable(light);
	//renderer->addRenderable(exampleRenderable);
	//renderer->addRenderable(exampleRenderable2);

	// Create some debug plants
	plants = plant::create_plants({ {{0,1,0}} });
	for (auto& p : plants) {
		renderer->addRenderable(&p.trunk);
		renderer->addRenderable(&p.canopy);
	}

	// renderer tweaks based on scene size
	renderer->voxelizer->setCenter(glm::vec3(-5, 5, -5));
	renderer->voxelizer->setWorldSize(50);
	dirtyVoxels = true;
	renderer->lightingPass->params.uAmbientColor = glm::vec3(0.05);
	renderer->lightingPass->params.uDiffuseBrightnessMultiplier = 2500;

}


void loadScene1() {
	resetScene();
	float roomSize = 2.0f;
	float wallThickness = roomSize / 20.0f;
	float wallLen = 0.55;
	glm::vec3 roomMat = glm::vec3(0, 0.5, 0);

	// Create renderables
	light = new PointLightRenderable();
	floor1 = new CubeRenderable();
	ceiling = new CubeRenderable();
	backWall = new CubeRenderable();
	leftWall = new CubeRenderable();
	rightWall = new CubeRenderable();
	cube = new CubeRenderable();
	exampleRenderable3 = new ExampleRenderable();

	// Floor (white)
	floor1->color = vec3(1.0f, 1.0f, 1.0f);
	floor1->mat = roomMat;
	floor1->modelTransform = glm::translate(mat4(1), vec3(0, 0, 0));
	floor1->modelTransform = glm::scale(floor1->modelTransform, vec3(roomSize * wallLen, wallThickness, roomSize * wallLen));

	// Ceiling (white)
	ceiling->color = vec3(1.0f, 1.0f, 1.0f);
	ceiling->mat = roomMat;
	ceiling->modelTransform = glm::translate(mat4(1), vec3(0, roomSize, 0));
	ceiling->modelTransform = glm::scale(ceiling->modelTransform, vec3(roomSize * wallLen, wallThickness, roomSize * wallLen));

	// Back wall (white)
	backWall->color = vec3(1.0f, 1.0f, 1.0f);
	backWall->mat = roomMat;
	backWall->modelTransform = glm::translate(mat4(1), vec3(0, roomSize / 2, -roomSize / 2));
	backWall->modelTransform = glm::scale(backWall->modelTransform, vec3(roomSize * wallLen, roomSize * wallLen, wallThickness));

	// Left wall (red)
	leftWall->color = vec3(1.0f, 0.0f, 0.0f);
	leftWall->mat = roomMat;
	leftWall->modelTransform = glm::translate(mat4(1), vec3(-roomSize / 2, roomSize / 2, 0));
	leftWall->modelTransform = glm::scale(leftWall->modelTransform, vec3(wallThickness, roomSize * wallLen, roomSize * wallLen));

	// Right wall (green)
	rightWall->color = vec3(0.0f, 1.0f, 0.0f);
	rightWall->mat = roomMat;
	rightWall->modelTransform = glm::translate(mat4(1), vec3(roomSize / 2, roomSize / 2, 0));
	rightWall->modelTransform = glm::scale(rightWall->modelTransform, vec3(wallThickness, roomSize * wallLen, roomSize * wallLen));

	// Light source (area light above center)
	lightPos = vec3(0, roomSize * 0.93, 0);
	lightScale = vec3(roomSize / 8, roomSize / 8, roomSize / 8);
	light->modelTransform = glm::translate(mat4(1), lightPos);
	light->modelTransform = glm::scale(light->modelTransform, lightScale);

	cube->color = vec3(1.0f, 1.0f, 1.0f);
	cube->mat = roomMat;
	cube->modelTransform = glm::translate(mat4(1), vec3(-roomSize / 8, roomSize / 6, 0));
	cube->modelTransform = glm::scale(cube->modelTransform, vec3(roomSize / 14, roomSize / 3, roomSize / 7));

	exampleRenderable3->modelTransform = glm::translate(glm::mat4(1), glm::vec3(roomSize / 5, roomSize / 2, roomSize / 5));
	exampleRenderable3->modelTransform = glm::scale(exampleRenderable3->modelTransform, vec3(roomSize / 15));

	// Add to renderer
	renderer->addRenderable(floor1);
	renderer->addRenderable(ceiling);
	renderer->addRenderable(backWall);
	renderer->addRenderable(leftWall);
	renderer->addRenderable(rightWall);
	renderer->addRenderable(light);
	renderer->addRenderable(cube);
	renderer->addRenderable(exampleRenderable3);

	// Configure voxelizer
	renderer->voxelizer->setCenter(vec3(0, roomSize / 2, 0));
	renderer->voxelizer->setWorldSize(15.25);
	light->brightness = 1;
	renderer->lightingPass->params.uAmbientColor = glm::vec3(0.02);
	renderer->lightingPass->params.uStepMultiplier = 1.5;
	renderer->lightingPass->params.uDiffuseBrightnessMultiplier = 1500;
	renderer->lightingPass->params.uZenithColor = glm::vec3(0);
	renderer->lightingPass->params.uHorizonColor = glm::vec3(0, 0, 0.01);
}

void loadScene2() {

}

Application::Application(GLFWwindow* window) : m_window(window) {
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);
	renderer = new Renderer(width, height);

	loadScene0();
}

void Application::updateCameraMovement(float deltaTime) {
	m_yaw = std::clamp(m_yaw, -pi<float>(), pi<float>());
	// Calculate forward and right directions
	vec3 forward = vec3(
		sin(m_yaw) * cos(m_pitch),
		-sin(m_pitch),
		-cos(m_yaw) * cos(m_pitch)
	);
	vec3 right = vec3(
		sin(m_yaw + pi<float>() / 2),
		0,
		-cos(m_yaw + pi<float>() / 2)
	);
	vec3 up = vec3(0, 1, 0);

	float speed = 5.0f * deltaTime; // units per second

	// WASD movement
	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) m_cameraPosition += forward * speed;
	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) m_cameraPosition -= forward * speed;
	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) m_cameraPosition += right * speed;
	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) m_cameraPosition -= right * speed;
	if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) m_cameraPosition += up * speed;
	if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) m_cameraPosition -= up * speed;
}

void Application::render() {
	// Calculate delta time
	static auto lastTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
	lastTime = currentTime;

	updateCameraMovement(deltaTime);
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);
	if (width != m_windowsize.x || height != m_windowsize.y) {
		m_windowsize = vec2(width, height); // update window size
		onWindowResize();
	}

	glViewport(0, 0, width, height); // set the viewport to draw to the entire window

	// clear the back-buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// projection matrix
	mat4 proj = perspective(1.f, float(width) / height, 0.1f, 1000.f);

	// First-person view matrix
	mat4 view = rotate(mat4(1), m_pitch, vec3(1, 0, 0))
		* rotate(mat4(1), m_yaw, vec3(0, 1, 0))
		* translate(mat4(1), -m_cameraPosition);


	if (dirtyVoxels) {
		renderer->refreshVoxels(view, proj);
		dirtyVoxels = false;
	}

	renderer->render(view, proj);
}

void Application::onWindowResize() {
	renderer->resizeWindow(m_windowsize.x, m_windowsize.y);
}

void Application::renderGUI() {

	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);
	ImGui::Begin("Rendering settings", 0);

	// display current camera parameters
	ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Separator();
	if (ImGui::Checkbox("Plant", &planttt)) {
		dirtyVoxels = true;
	}
	ImGui::Separator();

	if (ImGui::Button("Re-voxelize")) { dirtyVoxels = true; }
	if (ImGui::Button("load scene 0")) { loadScene0(); }
	if (ImGui::Button("load scene 1")) { loadScene1(); }
	if (ImGui::Button("load scene 2")) { loadScene2(); }

	ImGui::Separator();
	if (ImGui::CollapsingHeader("Light settings", ImDrawFlags_Closed)) {
		if (ImGui::SliderFloat3("Light pos", &lightPos[0], -20, 20)) { light->modelTransform = glm::translate(glm::mat4(1), lightPos); light->modelTransform = glm::scale(light->modelTransform, vec3(lightScale)); }
		if (ImGui::SliderFloat3("Light scale", &lightScale[0], 0, 4)) { light->modelTransform = glm::translate(glm::mat4(1), lightPos); light->modelTransform = glm::scale(light->modelTransform, vec3(lightScale)); }
		ImGui::SliderFloat3("Light color", &light->lightColor[0], 0, 1);
		ImGui::SliderFloat("Light brightness", &light->brightness, 1, 100000);
		ImGui::SliderFloat3("Ambient RGB", &renderer->lightingPass->params.uAmbientColor[0], 0.0, 1);
		ImGui::SliderFloat("Diffuse brightness multiplier", &renderer->lightingPass->params.uDiffuseBrightnessMultiplier, 0, 100000);
		ImGui::SliderFloat("AO multiplier", &renderer->lightingPass->params.uAO, 0, 2);
		ImGui::SliderFloat("Contrast", &renderer->lightingPass->params.uContrast, 0, 2);

	}
	if (ImGui::CollapsingHeader("Sky settings", ImDrawFlags_Closed)) {
		ImGui::SliderFloat3("Horizon color", &renderer->lightingPass->params.uHorizonColor[0], 0, 1);
		ImGui::SliderFloat3("Zenith color", &renderer->lightingPass->params.uZenithColor[0], 0, 1);
	}

	
#pragma region renderer params
	ImGui::Separator();
	if (ImGui::CollapsingHeader("Cone settings", ImDrawFlags_Closed)) {
		ImGui::Checkbox("Filmic tone mapping", &renderer->lightingPass->params.uToneMapEnable);
		ImGui::SliderFloat("Cone Aperature", &renderer->lightingPass->params.uConeAperture, 0.01, 2);
		ImGui::SliderFloat("Cone step multiplier", &renderer->lightingPass->params.uStepMultiplier, 0.05, 2);
		ImGui::SliderInt("Number of diffuse cones", &renderer->lightingPass->params.uNumDiffuseCones, 0, 128);
		ImGui::SliderFloat("Transmittance needed for cone termination", &renderer->lightingPass->params.uTransmittanceNeededForConeTermination, 0.0, 1);
		ImGui::SliderFloat("Cone offset", &renderer->lightingPass->params.uConeOffset, 0.0, 10);
		ImGui::SliderFloat("Reflection cone aperature", &renderer->lightingPass->params.uReflectionAperture, 0, 1);
		ImGui::SliderFloat("Cone max steps", &renderer->lightingPass->params.uMaxSteps, 0, 1024);
	}
	if (ImGui::CollapsingHeader("Reflection blending settings", ImDrawFlags_Closed)) {
		ImGui::SliderFloat("Reflection blend lower bound", &renderer->lightingPass->params.uReflectionBlendLowerBound, 0, 1);
		ImGui::SliderFloat("Reflection blend upper bound", &renderer->lightingPass->params.uReflectionBlendUpperBound, 0, 1);
	}

	ImGui::Separator();
	if (ImGui::CollapsingHeader("Gbuffer debug mode", ImDrawFlags_Closed)) {
		ImGui::Checkbox("Gbuffer debug enable", &renderer->debug_params.gbuffer_debug_mode_on);
		if (ImGui::Button("Gbuffer show position as RGB")) { renderer->debug_params.debug_channel_index = 1; }
		if (ImGui::Button("Gbuffer show metalic as RGB")) { renderer->debug_params.debug_channel_index = 2; }
		if (ImGui::Button("Gbuffer show normal as RGB")) { renderer->debug_params.debug_channel_index = 3; }
		if (ImGui::Button("Gbuffer show smoothness as RGB")) { renderer->debug_params.debug_channel_index = 4; }
		if (ImGui::Button("Gbuffer show albedo as RGB")) { renderer->debug_params.debug_channel_index = 5; }
		if (ImGui::Button("Gbuffer show emissive factor as RGB")) { renderer->debug_params.debug_channel_index = 6; }
		if (ImGui::Button("Gbuffer show emissive colorf as RGB")) { renderer->debug_params.debug_channel_index = 7; }
		if (ImGui::Button("Gbuffer show 'spare channel' as RGB")) { renderer->debug_params.debug_channel_index = 8; }
		if (ImGui::Button("Gbuffer show voxel sampled position as RGB")) { renderer->debug_params.debug_channel_index = 9; }
	}

	ImGui::Separator();
	if (ImGui::CollapsingHeader("Voxel settings", ImDrawFlags_Closed)) {
		ImGui::Checkbox("Voxel conservative rasterization", &renderer->voxelizer->m_params.conservativeRaster);
		ImGui::SliderInt("Voxelization render resolution", &renderer->voxelizer->m_params.voxelizeRes, 64, 7680);
		ImGui::SliderInt("Voxel splat radius", &renderer->voxelizer->m_params.voxelSplatRadius, 0, 5);

		ImGui::Checkbox("Voxel debug enable", &renderer->debug_params.voxel_debug_mode_on);
		ImGui::SliderFloat("Voxel slice", &renderer->debug_params.voxel_slice, 0, 1);
		ImGui::SliderFloat("Voxel world size", &renderer->voxelizer->m_params.worldSize, 1, 100);
		ImGui::SliderFloat("Voxel world center X", &renderer->voxelizer->m_params.center.x, -50, 50);
		ImGui::SliderFloat("Voxel world center Y", &renderer->voxelizer->m_params.center.y, -50, 50);
		ImGui::SliderFloat("Voxel world center Z", &renderer->voxelizer->m_params.center.z, -50, 50);
		if (ImGui::Button("Voxel show position as RGB")) { renderer->debug_params.debug_channel_index = 1; }
		if (ImGui::Button("Voxel show metallic as RGB")) { renderer->debug_params.debug_channel_index = 2; }
		if (ImGui::Button("Voxel show normal as RGB")) { renderer->debug_params.debug_channel_index = 3; }
		if (ImGui::Button("Voxel show smoothness as RGB")) { renderer->debug_params.debug_channel_index = 4; }
		if (ImGui::Button("Voxel show albedo as RGB")) { renderer->debug_params.debug_channel_index = 5; }
		if (ImGui::Button("Voxel show emissive factor as RGB")) { renderer->debug_params.debug_channel_index = 6; }
	}
	ImGui::End();

	// Terrain UI stuff
	t_terrain->renderUI();

	// standalone preview of the noise texture
	static int tex_prev_size = 256;
	ImGui::SetNextWindowPos(ImVec2(500, 5), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(tex_prev_size + 32, tex_prev_size + 42), ImGuiCond_Once);
	ImGui::Begin("Texture preview", 0);
	ImGui::Image((ImTextureID)(intptr_t)t_terrain->t_noise.texID, ImVec2(tex_prev_size, tex_prev_size));
	ImGui::End();

	// {{{ LSystem stuff
	static lsystem::gui::Data ls_data;
	lsystem::gui::rules_window(ls_data);
	lsystem::gui::growth_window(ls_data);
	// }}}
}


void Application::cursorPosCallback(double xpos, double ypos) {
	if (m_leftMouseDown) {
		vec2 delta = vec2(xpos, ypos) - m_mousePosition;

		// Update yaw and pitch based on mouse movement
		m_yaw += delta.x * 0.005f;
		m_pitch += delta.y * 0.005f;

		// Clamp pitch
		m_pitch = glm::clamp(m_pitch, -pi<float>() / 2, pi<float>() / 2);

		// Wrap yaw
		if (m_yaw > pi<float>()) m_yaw -= float(2 * pi<float>());
		else if (m_yaw < -pi<float>()) m_yaw += float(2 * pi<float>());
	}

	// updated mouse position
	m_mousePosition = vec2(xpos, ypos);
}


void Application::mouseButtonCallback(int button, int action, int mods) {
	(void)mods; // currently un-used

	// capture is left-mouse down
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		m_leftMouseDown = (action == GLFW_PRESS); // only other option is GLFW_RELEASE
}


void Application::scrollCallback(double xoffset, double yoffset) {
	(void)xoffset; // currently un-used

	// Calculate forward direction
	vec3 forward = vec3(
		sin(m_yaw) * cos(m_pitch),
		-sin(m_pitch),
		-cos(m_yaw) * cos(m_pitch)
	);

	// Move camera forward/backward
	m_cameraPosition += forward * float(yoffset) * 0.5f;
}


void Application::keyCallback(int key, int scancode, int action, int mods) {
	(void)scancode, (void)mods; // currently un-used

	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		// Calculate forward and right directions
		vec3 forward = vec3(
			sin(m_yaw) * cos(m_pitch),
			-sin(m_pitch),
			-cos(m_yaw) * cos(m_pitch)
		);
		vec3 right = vec3(
			sin(m_yaw + pi<float>() / 2),
			0,
			-cos(m_yaw + pi<float>() / 2)
		);
		vec3 up = vec3(0, 1, 0);

		float speed = 0.3f;

		// WASD movement
		if (key == GLFW_KEY_W) m_cameraPosition += forward * speed;
		if (key == GLFW_KEY_S) m_cameraPosition -= forward * speed;
		if (key == GLFW_KEY_D) m_cameraPosition += right * speed;
		if (key == GLFW_KEY_A) m_cameraPosition -= right * speed;
		if (key == GLFW_KEY_SPACE) m_cameraPosition += up * speed;
		if (key == GLFW_KEY_LEFT_SHIFT) m_cameraPosition -= up * speed;
	}
}


void Application::charCallback(unsigned int c) {
	(void)c; // currently un-used
}
