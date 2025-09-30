
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

#include "terrain/BaseTerrain.hpp"


using namespace std;
using namespace cgra;
using namespace glm;

Renderer* renderer = nullptr;

ExampleRenderable* exampleRenderable  = nullptr;
PointLightRenderable* light = nullptr;
Terrain::BaseTerrain* t_terrain = nullptr;

Application::Application(GLFWwindow *window) : m_window(window) {
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);
	renderer = new Renderer(width, height);

	// add all renderables
	// exampleRenderable = new ExampleRenderable{};
	// renderer->addRenderable(exampleRenderable);
	light = new PointLightRenderable();
	light->modelTransform = glm::translate(glm::mat4(1), glm::vec3(2,10,2));
	light->modelTransform = glm::scale(light->modelTransform, vec3(1));

	renderer->addRenderable(light);

	t_terrain = new Terrain::BaseTerrain();
	renderer->addRenderable(t_terrain);
}

bool dirtyVoxels = true;
void Application::render() {
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);  
	if (width != m_windowsize.x || height != m_windowsize.y) {
		m_windowsize = vec2(width, height); // update window size
		onWindowResize();
	}

	glViewport(0, 0, width, height); // set the viewport to draw to the entire window

	// clear the back-buffer
	glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	// enable flags for normal/forward rendering
	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LESS);

	// projection matrix
	mat4 proj = perspective(1.f, float(width) / height, 0.1f, 1000.f);

	// view matrix
	mat4 view = translate(mat4(1), vec3(0, 0, -m_distance))
		* rotate(mat4(1), m_pitch, vec3(1, 0, 0))
		* rotate(mat4(1), m_yaw,   vec3(0, 1, 0));


	// helpful draw options
	if (m_show_grid) drawGrid(view, proj);
	if (m_show_axis) drawAxis(view, proj);
	glPolygonMode(GL_FRONT_AND_BACK, (m_showWireframe) ? GL_LINE : GL_FILL);


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
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Once);
	ImGui::Begin("Options", 0);

	// display current camera parameters
	ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::SliderFloat("Pitch", &m_pitch, -pi<float>() / 2, pi<float>() / 2, "%.2f");
	ImGui::SliderFloat("Yaw", &m_yaw, -pi<float>(), pi<float>(), "%.2f");
	ImGui::SliderFloat("Distance", &m_distance, 0, 100, "%.2f");

	// helpful drawing options
	ImGui::Checkbox("Show axis", &m_show_axis);
	ImGui::SameLine();
	ImGui::Checkbox("Show grid", &m_show_grid);
	ImGui::Checkbox("Wireframe", &m_showWireframe);
	ImGui::SameLine();
	if (ImGui::Button("Screenshot")) rgba_image::screenshot(true);

#pragma region renderer params
	ImGui::Separator();
	ImGui::Checkbox("Voxel debug enable", &renderer->debug_params.voxel_debug_mode_on);
	ImGui::SliderFloat("Voxel slice", &renderer->debug_params.voxel_slice, 0, 1);
	if (ImGui::Button("Voxel show position as RGB")) { renderer->debug_params.debug_channel_index = 1; }
	if (ImGui::Button("Voxel show metallic as RGB")) { renderer->debug_params.debug_channel_index = 2; }
	if (ImGui::Button("Voxel show normal as RGB")) { renderer->debug_params.debug_channel_index = 3; }
	if (ImGui::Button("Voxel show smoothness as RGB")) { renderer->debug_params.debug_channel_index = 4; }
	if (ImGui::Button("Voxel show albedo as RGB")) { renderer->debug_params.debug_channel_index = 5; }
	if (ImGui::Button("Voxel show emissive factor as RGB")) { renderer->debug_params.debug_channel_index = 6; }

	ImGui::Separator();
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
#pragma endregion

	ImGui::End();

	// Terrain UI stuff
	t_terrain->renderUI();

	// standalone preview of the noise texture
	static int tex_prev_size = 256;
	ImGui::SetNextWindowPos(ImVec2(500, 5), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(tex_prev_size+32, tex_prev_size+42), ImGuiCond_Once);
	ImGui::Begin("Texture preview", 0);
	ImGui::Image((ImTextureID)(intptr_t)t_terrain->t_noise.texID, ImVec2(tex_prev_size, tex_prev_size));
	ImGui::End();
}


void Application::cursorPosCallback(double xpos, double ypos) {
	if (m_leftMouseDown) {
		vec2 whsize = m_windowsize / 2.0f;

		// clamp the pitch to [-pi/2, pi/2]
		m_pitch += float(acos(glm::clamp((m_mousePosition.y - whsize.y) / whsize.y, -1.0f, 1.0f))
			- acos(glm::clamp((float(ypos) - whsize.y) / whsize.y, -1.0f, 1.0f)));
		m_pitch = float(glm::clamp(m_pitch, -pi<float>() / 2, pi<float>() / 2));

		// wrap the yaw to [-pi, pi]
		m_yaw += float(acos(glm::clamp((m_mousePosition.x - whsize.x) / whsize.x, -1.0f, 1.0f))
			- acos(glm::clamp((float(xpos) - whsize.x) / whsize.x, -1.0f, 1.0f)));
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
	m_distance *= pow(1.1f, -yoffset);
}


void Application::keyCallback(int key, int scancode, int action, int mods) {
	(void)key, (void)scancode, (void)action, (void)mods; // currently un-used
}


void Application::charCallback(unsigned int c) {
	(void)c; // currently un-used
}
