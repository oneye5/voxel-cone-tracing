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

#include "BaseTerrain.hpp"
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
		return;
	}
	heightmap[y * width + x] = std::clamp(h, 0.0f, 1.0f);
}

// Get
float HydraulicErosion::getHeightInterpolated(float x, float y) const {
	const int xi = static_cast<int>(x);
	const int yi = static_cast<int>(y);
	const float xf = x - std::floorf(x); // This is u
	const float yf = y - std::floorf(y); // this is v

	float h00 = getHeight(xi, yi);
	float h10 = getHeight(xi + 1, yi);
	float h01 = getHeight(xi, yi + 1);
	float h11 = getHeight(xi + 1, yi + 1);

	float h0 = h00 * (1.0f - xf) + h10 * xf;
	float h1 = h01 * (1.0f - xf) + h11 * xf;

	return h0 * (1 - yf) + h1 * yf;
}

// Calculate gradient from old position by doing bilinear interpolation with points around it
glm::vec2 HydraulicErosion::calculateBilinearGradient(const glm::vec2 pos) const {
	const int xi = static_cast<int>(pos.x);
	const int yi = static_cast<int>(pos.y);
	const float u = pos.x - std::floorf(pos.x);
	const float v = pos.y - std::floorf(pos.y);

	const float h_c = getHeight(xi, yi); // Center height
	const float h_cr = getHeight(xi+1, yi); // Right height
	const float h_br = getHeight(xi+1, yi+1); // Bottom right
	const float h_bc = getHeight(xi, yi+1); // Bottom

	const float g_x = (h_cr - h_c) * (1.0f - v) + (h_br - h_bc) * v; // x component of gradient
	const float g_y = (h_bc - h_c) * (1.0f - u) + (h_br - h_cr) * u; // y component of gradient
	return glm::vec2{g_x, g_y};
}

void HydraulicErosion::applyErosion(float x, float y, float amount, int radius) {
	const int xi = static_cast<int>(x);
	const int yi = static_cast<int>(y);
	const float f_radius = static_cast<float>(radius);

	for (int dy = -radius; dy <= radius; dy++) {
		for (int dx = -radius; dx <= radius; dx++) {
			float distance = sqrt(dx * dx + dy * dy);
			if (distance > f_radius) continue;

			// Weight decreases with distance
			float weight = 1.0f - (distance / f_radius);
			weight = weight * weight; // Square for smoother falloff

			int px = xi + dx;
			int py = yi + dy;

			if (px >= 0 && px < width && py >= 0 && py < height) {
				// TODO - something about setting height by interpolating between points
				float currentHeight = getHeight(px, py);
				float deltaHeight = amount * weight;
				setHeight(px, py, currentHeight + deltaHeight);
			}
		}
	}
}

void HydraulicErosion::simulateDroplet() {
	// Start at random position
	float x = uniform_dist(rng) * static_cast<float>(width - 1);
	float y = uniform_dist(rng) * static_cast<float>(height - 1);
	glm::vec2 pos(x, y);
	
	glm::vec2 dir(0.0f, 0.0f);
	
	float vel = settings.start_velocity;
	float water = settings.start_water;
	float sediment = 0;

	for (int lifetime = 0; lifetime < settings.max_lifetime; lifetime++) {
		glm::vec2 pos_old = pos;
		glm::vec2 dir_old = dir;
		
		glm::vec2 gradient = calculateBilinearGradient(pos_old);
		glm::vec2 dir_new = dir_old * settings.inertia - gradient * (1.0f - settings.inertia);

		float dir_length = glm::length(dir_new);
		if (dir_length == 0) { // randomize dir when length is zero
			// TODO - kinda gross, tweak later
			static std::uniform_real_distribution<float> dist(0.0f, 2.0f * glm::pi<float>());

			float angle = dist(rng);
			dir_new = glm::vec2(std::cos(angle), std::sin(angle));
		} else { // normalize the new direction
			dir_new = glm::vec2(dir_new.x / dir_length, dir_new.y / dir_length); // might as well reuse the length
		}
		dir = dir_new;

		glm::vec2 pos_new = pos_old + dir_new;
		pos = pos_new;

		if (pos_new.x < 0 || pos_new.x >= width-1 || pos_new.y < 0 || pos_new.y >= height-1) {
			// Stop if out of bounds
			break;
		}

		// calculate difference in old height and new height
		float h_old = getHeight(static_cast<int>(pos_old.x), static_cast<int>(pos_old.y));
		float h_new = getHeight(static_cast<int>(pos_new.x), static_cast<int>(pos_new.y));
		float h_dif = h_new - h_old;

		// Calculate capacity
		float capacity = std::max(-h_dif, settings.min_slope) * vel * water * settings.capacity_s;
		

		if (sediment > capacity) {
			// Deposit some sediment at point as above carry capacity
			const float amount_to_deposit = (sediment - capacity) * settings.deposition;
			sediment -= amount_to_deposit;
			
			applyErosion(pos_old.x, pos_old.y, amount_to_deposit, settings.erosion_radius);
			
		} else { // Steal some sediment from position and add it to particle sediment
			const float amount_to_erode = std::min((capacity - sediment) * settings.erode_speed, -h_dif);
			applyErosion(pos_old.x, pos_old.y, -amount_to_erode, settings.erosion_radius);
			sediment += amount_to_erode;
		}

		// Update the speed and evaporate water
		vel = sqrtf(std::max(((vel * vel) + h_dif * settings.gravity), 0.0f));
		water = water * (1.0f - settings.evaporate_speed);

		// Stop if no water left
		if (water < 0.01f) {break;}
	}
}

void HydraulicErosion::simulate(int iterations) {
	if (iterations == -1) iterations = settings.iterations;
	// rng = std::mt19937(1337); // DEBUG - for if i want the same rng

	for (int i = 0; i < iterations; i++) {
		simulateDroplet();

		// Optional: progress feedback every 1000 iterations
		if (i % 1000 == 0 && i > 0) {
			// You could add a progress callback here if needed
		}
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
		settingsChanged |= ImGui::SliderFloat("Min Sediment Capacity", &erosionSettings.min_slope, 0.001f, 0.1f);
		settingsChanged |= ImGui::SliderFloat("Deposit Speed", &erosionSettings.deposition, 0.01f, 1.0f);
		settingsChanged |= ImGui::SliderFloat("Erosion Speed", &erosionSettings.erode_speed, 0.01f, 1.0f);
	}

	if (use_own_window) {ImGui::End();}
}

void HydraulicErosion::setSimulationDims(int w, int h) {
	width = w;
	height = h;
}
