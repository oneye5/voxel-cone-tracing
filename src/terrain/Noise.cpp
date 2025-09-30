
#include <algorithm>
#include <iostream>
#include "Noise.hpp"
#include "FastNoiseLite.h"
#include "../opengl.hpp"
#include <cmath>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <vector>
#include <imgui.h>

using namespace Terrain;


Noise::Noise() : erosionSim(DEFAULT_WIDTH, DEFAULT_HEIGHT, 1337) {
	heightmap.resize(width * height);
	pixels.resize(width * height); // Reserve the size
	noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

	// Initialize domain warp noise
	domainWarp.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	domainWarp.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
	domainWarp.SetSeed(1337);
	domainWarp.SetFrequency(0.010);
	domainWarp.SetDomainWarpAmp(1.0f);
	
	updatePixels();
	createTexture();
}

// Make the imgui panel for this
// TODO - maybe make some like noisesettings struct
void Noise::makeEditUI(bool use_own_window) {
	if (use_own_window) {
		ImGui::SetNextWindowPos(ImVec2(400, 5), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);
		ImGui::Begin("Noise", 0);
	} else {ImGui::Separator();}

	ImGui::Text("General Settings");

	int w = width;
	if (ImGui::SliderInt("Texture size (square)", &w, 64, 1024)) {
		changeTextureSize(w, w);
	}
	
	bool settings_updated = false;

	// ------- Noise Type -----
	
	static const std::pair<const char*, FastNoiseLite::NoiseType> noise_types[] = {
		{"OpenSimplex2",  FastNoiseLite::NoiseType_OpenSimplex2},
		{"OpenSimplex2S", FastNoiseLite::NoiseType_OpenSimplex2S},
		{"Cellular",      FastNoiseLite::NoiseType_Cellular},
		{"Perlin",        FastNoiseLite::NoiseType_Perlin},
		{"ValueCubic",    FastNoiseLite::NoiseType_ValueCubic},
		{"Value",         FastNoiseLite::NoiseType_Value}
	};

	// kinda gross but idk a better way lmao
	const char* current_noise_name = "Unknown";
	for (const auto& item : noise_types) {
		if (item.second == settings.noise_type) {
			current_noise_name = item.first;
			break;
		}
	}
	
	if (ImGui::BeginCombo("Noise Types", current_noise_name)) {
		for (const auto& item : noise_types) {
			if (ImGui::Selectable(item.first, settings.noise_type == item.second)) {
				settings.noise_type = item.second;
				settings_updated = true;
			}
		}
		ImGui::EndCombo();
	}

	settings_updated |= ImGui::DragInt("Seed", &settings.seed, 1.0f, 1, 0);
	settings_updated |= ImGui::SliderFloat("Frequency", &settings.frequency, 0.005f, 0.5f);
	settings_updated |= ImGui::SliderFloat("Noise Exponent", &settings.noise_exp, 0.01f, 10.0f);

	ImGui::Separator();
	
	// ------- Fractal Settings --------
	ImGui::Text("Fractal Settings");
	static const std::pair<const char*, FastNoiseLite::FractalType> fractal_types[] = {
		{"None", FastNoiseLite::FractalType_None},
		{"FBm", FastNoiseLite::FractalType_FBm},
		{"Rigid", FastNoiseLite::FractalType_Ridged},
		{"PingPong", FastNoiseLite::FractalType_PingPong}
	};
	
	const char* current_fractal_name = "Unknown";
	for (const auto& item : fractal_types) {
		if (item.second == settings.fractal_type) {
			current_fractal_name = item.first;
			break;
		}
	}

	if (ImGui::BeginCombo("FractalType", current_fractal_name)) {
		for (const auto& item: fractal_types) {
			if (ImGui::Selectable(item.first, settings.fractal_type == item.second)) {
				settings.fractal_type = item.second;
				settings_updated = true;
			}
		}
		ImGui::EndCombo();
	}

	ImGui::BeginDisabled(settings.fractal_type == FastNoiseLite::FractalType_None);
	settings_updated |= ImGui::DragInt("Octaves", &settings.fractal_octaves, 0.2f, 1, 20);
	settings_updated |= ImGui::DragFloat("Lacunarity", &settings.fractal_lacunarity, 0.01f, 0.0f, 10.0f);
	settings_updated |= ImGui::DragFloat("Gain", &settings.fractal_gain, 0.01f, 0.0f, 3.0f);
	settings_updated |= ImGui::DragFloat("Weighted Strength", &settings.fractal_weighted_strength, 0.01f, 0.0f, 1.0f);
	settings_updated |= ImGui::DragFloat("PingPong Strength", &settings.fractal_pingpong_strength, 0.1f, 0.0f, 10.0f);
	ImGui::EndDisabled();
	ImGui::Separator();

	// Cellular settings
	settings_updated |= makeCellularSettingsUI();
	
	ImGui::Separator();

	// ------- Domain Warping --------
	ImGui::Text("Domain Warping");
	
	settings_updated |= ImGui::Checkbox("Use Domain Warping", &settings.use_domain_warp);
	
	if (settings.use_domain_warp) {
		// Domain Warp Type
		static const std::pair<const char*, FastNoiseLite::DomainWarpType> warp_types[] = {
			{"OpenSimplex2", FastNoiseLite::DomainWarpType_OpenSimplex2},
			{"OpenSimplex2Reduced", FastNoiseLite::DomainWarpType_OpenSimplex2Reduced},
			{"BasicGrid", FastNoiseLite::DomainWarpType_BasicGrid}
		};

		const char* current_warp_name = "Unknown";
		for (const auto& item: warp_types) {
			if (item.second == settings.domain_warp_type) {
				current_warp_name = item.first;
				break;
			}
		}

		if (ImGui::BeginCombo("Warp Type", current_warp_name)) {
			for (const auto& item: warp_types) {
				if (ImGui::Selectable(item.first, settings.domain_warp_type == item.second)) {
					settings.domain_warp_type = item.second;
					settings_updated = true;
				}
			}
			ImGui::EndCombo();
		}

		// TODO - temp, move these into settings later (but haven't used domain warp too much so it's fine for now :p)
		static int warp_seed = 1338;
		static float warp_freq = 0.005f;
		settings_updated |= ImGui::DragInt("Warp Seed", &warp_seed, 1.0f, 1, 0);
		settings_updated |= ImGui::SliderFloat("Warp Frequency", &warp_freq, 0.001f, 0.1f);
		settings_updated |= ImGui::DragFloat("Warp Amplitude", &settings.domain_warp_amp, 1.0f, 0.0f, 300.0f);

		if (settings_updated) {
			domainWarp.SetSeed(warp_seed);
			domainWarp.SetFrequency(warp_freq);
		}
	}

	if (settings_updated) {
		updateNoiseFromSettings();
		updatePixels();
		updateTexture();
	}

	if (ImGui::Button("Regenerate terrain")) {
		updatePixels();
		updateTexture();
	}


	ImGui::Separator();

	// ------- Erosion Controls -------
	// TODO - move these out of here, not sure why I put them here lmao
	ImGui::Text("Erosion Settings");
	if (ImGui::Checkbox("Use Erosion", &use_erosion)) {
		updatePixels();
		updateTexture();
	}

	if (use_erosion) {
		erosionSim.renderUI();
		
		if (ImGui::Button("Apply Erosion Once")) {
			applyErosion();
			updatePixelsWithHeightmap();
			updateTexture();
		}
    
		ImGui::SameLine();
		if (ImGui::Button("Regenerate with Erosion")) {
			updatePixels(true);  // This will regenerate noise and apply erosion
			updateTexture();
		}
	}	
	
	// static const float PREV_SIZE = 256;
	// ImGui::Image((ImTextureID)(intptr_t)texID, ImVec2(PREV_SIZE, PREV_SIZE));

	if (use_own_window) {ImGui::End();}
}


// Update the texture using the current state of pixels
// TODO - implement discarding the old one
void Noise::updateTexture(bool reuse_old) {
	if (!reuse_old) {throw std::runtime_error("not implemented");}

	glBindTexture(GL_TEXTURE_2D, texID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
					width, height, GL_RED, GL_UNSIGNED_SHORT,
					pixels.data());

	glBindTexture(GL_TEXTURE_2D, 0);
}

// Updates the heightmap vector using the noise generator (with values mapped to
// [0.0f,1.0f]) and updates pixels vector with uint16 values
// Also applies the erosion on request.
void Noise::updatePixels(bool apply_erosion) {
	int index = 0;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			float fx = (float)x;
			float fy = (float)y;
			
			// Apply domain warping if enabled
			if (settings.use_domain_warp) {
				domainWarp.DomainWarp(fx, fy);
			}
			
			float data = noise.GetNoise(fx, fy);
			float normal = (data + 1.0f) / 2.0f;
			normal = pow(normal, settings.noise_exp);
			heightmap[index++] = normal;
			
			// uint16_t result = static_cast<uint16_t>(std::round(normal * max));
			// pixels[index++] = result;
		}
	}

	if (apply_erosion) {
		applyErosion();
	}

	updatePixelsWithHeightmap();
}


void Noise::applyErosion() {
	erosionSim.setHeightmap(heightmap);

	erosionSim.simulate();

	heightmap = erosionSim.getHeightmap();
}

// TODO - check this isn't slow as hell
void Noise::updateNoiseFromSettings() {
	noise.SetSeed(settings.seed);
	noise.SetFrequency(settings.frequency);
	noise.SetNoiseType(settings.noise_type);

	noise.SetFractalType(settings.fractal_type);
	noise.SetFractalOctaves(settings.fractal_octaves);
	noise.SetFractalLacunarity(settings.fractal_lacunarity);
	noise.SetFractalGain(settings.fractal_gain);
	noise.SetFractalWeightedStrength(settings.fractal_weighted_strength);
	noise.SetFractalPingPongStrength(settings.fractal_pingpong_strength);

	noise.SetCellularDistanceFunction(settings.cellular_dist_function);
	noise.SetCellularReturnType(settings.cellular_return_type);
	noise.SetCellularJitter(settings.cellular_jitter);

	noise.SetDomainWarpType(settings.domain_warp_type);
	noise.SetDomainWarpAmp(settings.domain_warp_amp);
}

void Noise::setNoiseSettings(NoiseSettings new_settings) {
	settings = new_settings;
	updateNoiseFromSettings();
	updatePixels();
	updateTexture();
}

void Noise::changeTextureSize(int w, int h) {
	width = w;
	height = h;
	pixels.resize(width * height);
	heightmap.resize(width * height);

	updatePixels();
	createTexture();
	erosionSim.setSimulationDims(width, height);
	erosionSim.setHeightmap(heightmap);
	// updateTexture();
}

// Allocates a texture for the noise to use and sets texid to it
void Noise::createTexture() {
	if (texID == 0) { // Generate new texID if hasn't been created
		glGenTextures(1, &texID);
	}
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
	
	GLint format = GL_R16; // NOTE - not sure if right format but hopefully
	glTexImage2D(GL_TEXTURE_2D, 0, format,
				 width, height, 0,
				 GL_RED, GL_UNSIGNED_SHORT, pixels.data());

	// This will let me preview it nicely (it won't be red :D)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);

	glBindTexture(GL_TEXTURE_2D, 0);
}


void Noise::updatePixelsWithHeightmap() {
	for (size_t i = 0; i < heightmap.size(); i++) {
		uint16_t norm = static_cast<uint16_t>(std::clamp(heightmap[i], 0.0f, 1.0f) * UINT16_MAX);
		pixels[i] = norm;
	}
}


bool Noise::makeCellularSettingsUI() {
	bool settings_changed = false;
	ImGui::Text("Cellular Settings");
	
	static const std::map<FastNoiseLite::CellularDistanceFunction, const char*> distance_funcs = {
		{FastNoiseLite::CellularDistanceFunction_Euclidean, "Euclidian"},
		{FastNoiseLite::CellularDistanceFunction_EuclideanSq, "EuclidianSq"},
		{FastNoiseLite::CellularDistanceFunction_Manhattan, "Manhattan"},
		{FastNoiseLite::CellularDistanceFunction_Hybrid, "Hybrid"}
	};
	const char* current_distance_name = distance_funcs.at(settings.cellular_dist_function);

	if (ImGui::BeginCombo("Cellular Distance Function", current_distance_name)) {
		for (const auto& e: distance_funcs) {
			if (ImGui::Selectable(e.second, e.first == settings.cellular_dist_function)) {
				settings.cellular_dist_function = e.first;
				settings_changed = true;
			}
		}
		ImGui::EndCombo();
	}

	// --------

	static const std::map<FastNoiseLite::CellularReturnType, const char*> return_types = {
		{FastNoiseLite::CellularReturnType_CellValue, "CellValue"},
		{FastNoiseLite::CellularReturnType_Distance, "Distance"},
		{FastNoiseLite::CellularReturnType_Distance2, "Distance2"},
		{FastNoiseLite::CellularReturnType_Distance2Add, "Distance2Add"},
		{FastNoiseLite::CellularReturnType_Distance2Sub, "Distance2Sub"},
		{FastNoiseLite::CellularReturnType_Distance2Mul, "Distance2Mul"},
		{FastNoiseLite::CellularReturnType_Distance2Div, "Distance2Div"}
	};
	const char* current_return_name = return_types.at(settings.cellular_return_type);

	if (ImGui::BeginCombo("Cellular Return Function", current_return_name)) {
		for (const auto& e: return_types) {
			if (ImGui::Selectable(e.second, e.first == settings.cellular_return_type)) {
				settings.cellular_return_type = e.first;
				settings_changed = true;
			}
		}
		ImGui::EndCombo();
	}

	settings_changed |= ImGui::SliderFloat("Cellular Jitter", &settings.cellular_jitter, 0.0f, 1.0f);
	
	return settings_changed;
}
