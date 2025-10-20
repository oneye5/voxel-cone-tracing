#include "HydraulicErosion.hpp"
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <cmath>
#include <glm/glm.hpp>
#include <iostream>
#include <format>
#include <print>
#include <imgui.h>

#include "glm/gtc/constants.hpp"

using namespace Terrain;

HydraulicErosion::HydraulicErosion(int w, int h, unsigned int seed) : rng(seed), uniform_dist(0.0f, 1.0f) {
	width = w;
	height = h;
	heightmap.resize(width * height);
}

void HydraulicErosion::setHeightmap(const std::vector<float>& heights) {
	if (heights.size() != static_cast<size_t>(width) * height) {
		throw std::runtime_error("Size mismatch");
	}
	heightmap = heights;
}

std::vector<uint16_t> HydraulicErosion::getHeightmapAsUint16() const {
	std::vector<uint16_t> result(heightmap.size());

	for (size_t i = 0; i < heightmap.size(); i++) {
		uint16_t norm = static_cast<uint16_t>(std::clamp(heightmap[i], 0.0f, 1.0f) * UINT16_MAX);
		result[i] = norm;
	}
	return result;
}

// Helper functions for the erosion simulation
// NOTE - I think some algorithms always interpolate height so you can get float positions but i probs won't
float HydraulicErosion::getHeight(int x, int y) const {
	if (x < 0 || x >= width || y < 0 || y >= height) {
		std::println("Attempted to get height at invalid location: {},{}", x, y);
		return 0.0f;
	}
	return heightmap[y * width + x];
}

void HydraulicErosion::setHeight(int x, int y, float h) {
	if (x < 0 || x >= width || y < 0 || y >= height) {
		throw std::range_error("Attempted to set height at invalid location");
	}
	heightmap[y * width + x] = std::clamp(h, 0.0f, 1.0f);
}

// Get the height at current point using bilinear interpolation
float HydraulicErosion::getBilinearHeight(glm::vec2 pos) const {
	const int xi = static_cast<int>(pos.x);
	const int yi = static_cast<int>(pos.y);
	const float u = pos.x - std::floorf(pos.x);
	const float v = pos.y - std::floorf(pos.y);

	const float h_tl = getHeight(xi, yi); // Top left height (H00)
	const float h_tr = getHeight(xi+1, yi); // Top right height (H10)
	const float h_br = getHeight(xi+1, yi+1); // Bottom right height (H11)
	const float h_bl = getHeight(xi, yi+1); // Bottom left height (H01)

	const float h0 = h_tl * (1.0f - u) + h_tr * u;
	const float h1 = h_bl * (1.0f - u) + h_br * u;

	const float interpolated = h0 * (1.0f - v) + h1 * v;
	return interpolated;
}

// Calculate gradient from old position by doing bilinear interpolation with points around it
glm::vec2 HydraulicErosion::calculateBilinearGradient(const glm::vec2 pos) const {
	const int xi = static_cast<int>(pos.x);
	const int yi = static_cast<int>(pos.y);
	const float u = pos.x - std::floorf(pos.x);
	const float v = pos.y - std::floorf(pos.y);

	const float h_tl = getHeight(xi, yi); // Top left height (H00)
	const float h_tr = getHeight(xi+1, yi); // Top right height (H10)
	const float h_br = getHeight(xi+1, yi+1); // Bottom right height (H11)
	const float h_bl = getHeight(xi, yi+1); // Bottom left height (H01)

	const float g_x = (h_tr - h_tl) * (1.0f - v) + (h_br - h_bl) * v; // x component of gradient
	const float g_y = (h_bl - h_tl) * (1.0f - u) + (h_br - h_tr) * u; // y component of gradient
	return glm::vec2{g_x, g_y};
}

void HydraulicErosion::applyErosion(glm::vec2 pos, float amount, int radius) {
	const int xi = static_cast<int>(pos.x);
	const int yi = static_cast<int>(pos.y);
	const glm::vec2 i_pos = glm::vec2(xi, yi);

	const float f_radius = static_cast<float>(radius);
	std::vector<float> weights;
	float total_weight = 0.0f;

	// First pass: calculate the weights for each point in radius around erosion point
	for (int dy = -radius; dy <= radius; dy++) {
		for (int dx = -radius; dx <= radius; dx++) {
			const glm::vec2 cell_pos = i_pos + glm::vec2(dx, dy);
			if (cell_pos.x < 0 || cell_pos.x >= width || cell_pos.y < 0 || cell_pos.y >= height) {
				continue;
			}

			const float weight = std::max(0.0f, f_radius - glm::length(i_pos - cell_pos));
			total_weight += weight;
			weights.push_back(weight);
		}
	}

	if (total_weight <= 0.0f) {std::print("Total weight shouldn't be 0");}

	// Second pass: do the actual erosion
	int i = 0;
	for (int dy = -radius; dy <= radius; dy++) {
		for (int dx = -radius; dx <= radius; dx++) {
			const int cx = xi + dx;
			const int cy = yi + dy;
			if (cx < 0 || cx >= width || cy < 0 || cy >= height) {
				continue;
			}

			const float weight = weights.at(i++);
			if (weight <= 0.0f) {continue;}

			const float cell_erosion_amount = amount * (weight / total_weight);
			const float cell_height = getHeight(cx, cy);
			setHeight(cx, cy, cell_height - cell_erosion_amount);
		}
	}
}

// Apply deposition
void HydraulicErosion::applyDeposition(glm::vec2 pos, float amount) {
	const int x0 = static_cast<int>(floor(pos.x));
	const int y0 = static_cast<int>(floor(pos.y));
	const int x1 = x0 + 1;
	const int y1 = y0 + 1;

	const float fx = pos.x - static_cast<float>(x0);
	const float fy = pos.y - static_cast<float>(y0);

	float w00 = (1.0f - fx) * (1.0f - fy);
	float w10 = fx * (1.0f - fy);
	float w01 = (1.0f - fx) * fy;
	float w11 = fx * fy;

	setHeight(x0, y0, getHeight(x0, y0) + w00 * amount);
	setHeight(x1, y0, getHeight(x1, y0) + w10 * amount);
	setHeight(x0, y1, getHeight(x0, y1) + w01 * amount);
	setHeight(x1, y1, getHeight(x1, y1) + w11 * amount);
}

void HydraulicErosion::simulateDroplet(int steps) {
	for (int lifetime = 0; lifetime < steps && c_particle.valid; lifetime++) {
		glm::vec2 pos_old = c_particle.c_pos;
		glm::vec2 dir_old = c_particle.c_dir;
		
		glm::vec2 gradient = calculateBilinearGradient(pos_old);
		glm::vec2 dir_new = dir_old * settings.inertia - gradient * (1.0f - settings.inertia);

		float dir_length = glm::length(dir_new);
		if (dir_length == 0) { // randomize dir when length is zero
			// TODO - kinda gross, tweak later
			static std::uniform_real_distribution<float> dist(0.0f, 2.0f * glm::pi<float>());

			float angle = dist(rng);
			dir_new = glm::normalize(glm::vec2(std::cos(angle), std::sin(angle)));
		} else { // normalize the new direction
			dir_new = glm::normalize(dir_new);
			// dir_new = glm::vec2(dir_new.x / dir_length, dir_new.y / dir_length); // might as well reuse the length
		}
		c_particle.c_dir = dir_new;
		glm::vec2 pos_new = pos_old + dir_new;
		c_particle.c_pos = pos_new;

		if (pos_new.x < 0 || pos_new.x >= width-1 || pos_new.y < 0 || pos_new.y >= height-1) {
			// Stop if out of bounds
			c_particle.valid = false;
			break;
		}

		// calculate difference in old height and new height
		const float h_old = getBilinearHeight(pos_old);
		const float h_new = getBilinearHeight(pos_new);
		const float h_dif = h_new - h_old;

		float capacity = std::max(-h_dif, settings.min_slope) * c_particle.c_vel * c_particle.c_water * settings.capacity_s;
		if (c_particle.c_sediment > capacity || h_dif > 0.0) {
			// Deposit if carrying too much or going uphill
			const float amount_to_deposit = (h_dif > 0.0) ? std::min(h_dif, c_particle.c_sediment) : (c_particle.c_sediment - capacity) * settings.deposit_speed;
			c_particle.c_sediment -= amount_to_deposit;
			
			applyDeposition(pos_old, amount_to_deposit);
			
		} else { // Steal some sediment from position and add it to particle sediment
			const float amount_to_erode = std::min((capacity - c_particle.c_sediment) * settings.erode_speed, -h_dif);
			applyErosion(pos_old, amount_to_erode, settings.erosion_radius);
			c_particle.c_sediment += amount_to_erode;
		}

		// Update the speed and evaporate water
		c_particle.c_vel = sqrtf(std::max(((c_particle.c_vel * c_particle.c_vel) + h_dif * settings.gravity), 0.0f));
		c_particle.c_water = c_particle.c_water * (1.0f - settings.evaporate_speed);

		// Stop if no water left
		if (c_particle.c_water < 0.01f) {
			c_particle.valid = false;
			break;
		}

		c_particle.c_steps += 1;
		if (c_particle.c_steps >= settings.max_lifetime) {
			// TODO - could give cells their own lifetime idk
			c_particle.valid = false;
			break;
		}
	}
}

void HydraulicErosion::simulate(int iterations) {
	if (iterations == -1) iterations = settings.iterations;
	// rng = std::mt19937(1337); // DEBUG - for if i want the same rng

	for (int i = 0; i < iterations; i++) {
		c_particle = createParticle();
		simulateDroplet(settings.max_lifetime);
	}
}

void HydraulicErosion::newSimulation(const std::vector<float> &init_heights, int w, int h) {
	heightmap = init_heights;
	width = w;
	height = h;

	c_particle = createParticle();
	iterations_ran = 0;
}

// Step the simulation by simulating [particles_per_frame] amount of iterations
void HydraulicErosion::stepSimulation() {
	if (iterations_ran > settings.iterations) {
		// TODO - set some values as invalid here or smth
		return;
	}

	for (int step_iterations = 0; step_iterations < settings.particles_per_frame; step_iterations++) {
		// Particle is invalid, create a new one and increment iterations
		c_particle = createParticle();
		simulateDroplet(settings.max_lifetime);
		iterations_ran += 1;
	}
}


// Make basic imgui controls for erosion settings
// TODO - Could add a like MIN_SETTINGS and MAX_SETTINGS for the controls idk
void HydraulicErosion::renderUI(bool use_own_window) {
	if (use_own_window) {
		ImGui::SetNextWindowPos(ImVec2(400, 5), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);
		ImGui::Begin("Noise", 0);
	} else {ImGui::Separator();}

	ImGui::Text("Erosion Settings");

	ErosionSettings& erosionSettings = settings;
    
	bool settingsChanged = false;
    
	if (ImGui::CollapsingHeader("Basic Erosion Settings")) {
		settingsChanged |= ImGui::SliderInt("Iterations", &erosionSettings.iterations, 1000, 500000);
		ImGui::SliderInt("Iterations per frame (real-time)", &erosionSettings.particles_per_frame, 1, erosionSettings.iterations-1);
		settingsChanged |= ImGui::SliderInt("Max Particle Lifetime", &erosionSettings.max_lifetime, 10, 100);
		settingsChanged |= ImGui::SliderFloat("Inertia", &erosionSettings.inertia, 0.0f, 1.0f);
		settingsChanged |= ImGui::SliderInt("Erosion Radius", &erosionSettings.erosion_radius, 1, 10);
	}
    
	if (ImGui::CollapsingHeader("Water Properties")) {
		settingsChanged |= ImGui::SliderFloat("Evaporate Speed", &erosionSettings.evaporate_speed, 0.001f, 0.1f);
		settingsChanged |= ImGui::SliderFloat("Start Water", &erosionSettings.start_water, 0.1f, 5.0f);
		settingsChanged |= ImGui::SliderFloat("Start Speed", &erosionSettings.start_velocity, 0.1f, 5.0f);
		settingsChanged |= ImGui::SliderFloat("Gravity", &erosionSettings.gravity, 1.0f, 20.0f);
	}
    
	if (ImGui::CollapsingHeader("Sediment Properties")) {
		settingsChanged |= ImGui::SliderFloat("Sediment Capacity", &erosionSettings.capacity_s, 1.0f, 20.0f);
		settingsChanged |= ImGui::SliderFloat("Min Slope", &erosionSettings.min_slope, 0.001f, 0.1f);
		settingsChanged |= ImGui::SliderFloat("Deposit Speed", &erosionSettings.deposit_speed, 0.01f, 1.0f);
		settingsChanged |= ImGui::SliderFloat("Erosion Speed", &erosionSettings.erode_speed, 0.01f, 1.0f);
	}

	if (use_own_window) {ImGui::End();}
}

void HydraulicErosion::setSimulationDims(int w, int h) {
	width = w;
	height = h;
}


