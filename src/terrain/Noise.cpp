
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
#include <print>

using namespace Terrain;


Noise::Noise() {
	heightmap.resize(width * height);
	pixels.resize(width * height); // Reserve the size
	noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

	// Initialize domain warp noise
	domainWarp.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	domainWarp.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
	domainWarp.SetSeed(1337);
	domainWarp.SetFrequency(0.010);
	domainWarp.SetDomainWarpAmp(1.0f);
	
	generateHeightmap();
	createTexture();
	//setNoiseSettings(Presets::testSettings1);
}

// Make the imgui panel for this
// TODO - maybe make some like noisesettings struct
void Noise::makeEditUI(bool use_own_window) {
	if (use_own_window) {
		ImGui::SetNextWindowPos(ImVec2(400, 5), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_Once);
		ImGui::Begin("Noise", 0);
	}

	if (ImGui::BeginCombo("Noise Presets", "N/A", ImGuiComboFlags_NoPreview)) {
		for (const auto& p: Presets::NOISE_PRESET_MAP) {
			if (ImGui::Selectable(p.first, false)) {
				setNoiseSettings(p.second);
			}
		}

		ImGui::EndCombo();
	}

	int w = width;
	if (ImGui::SliderInt("Texture size (square)", &w, 64, 1024)) {
		changeTextureSize(w, w);
	}

	ImGui::Text("General Settings");
	
	bool settings_updated = false;

	settings_updated |= ImGui::DragFloat("Noise X Offset", &settings.offset_x, 0.5f, 0.0f);
	settings_updated |= ImGui::DragFloat("Noise Y Offset", &settings.offset_y, 0.5f, 0.0f);

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
		static const std::map<FastNoiseLite::DomainWarpType, const char*> warp_types = {
			{FastNoiseLite::DomainWarpType_OpenSimplex2, "OpenSimplex2"},
			{FastNoiseLite::DomainWarpType_OpenSimplex2Reduced, "OpenSimplex2Reduced"},
			{FastNoiseLite::DomainWarpType_BasicGrid, "BasicGrid"}
		};

		const char* current_warp_name = warp_types.at(settings.domain_warp_type);
		if (ImGui::BeginCombo("Warp Type", current_warp_name)) {
			for (const auto& item: warp_types) {
				if (ImGui::Selectable(item.second, settings.domain_warp_type == item.first)) {
					settings.domain_warp_type = item.first;
					settings_updated = true;
				}
			}
			ImGui::EndCombo();
		}

		settings_updated |= ImGui::DragInt("Warp Seed", &settings.domain_seed, 1.0f, 1, 0);
		settings_updated |= ImGui::SliderFloat("Warp Frequency", &settings.domain_frequency, 0.001f, 0.1f);
		settings_updated |= ImGui::DragFloat("Warp Amplitude", &settings.domain_warp_amp, 1.0f, 0.0f, 300.0f);

		ImGui::Text("Domain Warp Fractal Settings");
		static const std::map<FastNoiseLite::FractalType, const char*> domain_fractal_types = {
			{FastNoiseLite::FractalType_None, "None"},
			{FastNoiseLite::FractalType_DomainWarpProgressive, "Progressive"},
			{FastNoiseLite::FractalType_DomainWarpIndependent, "Independent"}
		};
		const char* current_domain_fractal_type = domain_fractal_types.at(settings.domain_warp_fractal_type);
		if (ImGui::BeginCombo("Domain Fractal Type", current_domain_fractal_type)) {
			for (const auto& item: domain_fractal_types) {
				if (ImGui::Selectable(item.second, settings.domain_warp_fractal_type == item.first)) {
					settings.domain_warp_fractal_type = item.first;
					settings_updated = true;
				}
			}

			ImGui::EndCombo();
		}

		settings_updated |= ImGui::DragInt("Domain Octaves", &settings.domain_fractal_octaves, 0.2f, 1, 20);
		settings_updated |= ImGui::DragFloat("Domain Lacunarity", &settings.domain_fractal_lacunarity, 0.01f, 0.0f, 10.0f);
		settings_updated |= ImGui::DragFloat("Domain Gain", &settings.domain_fractal_gain, 0.01f, 0.0f, 10.0f);
	}

	if (settings_updated) {
		updateNoiseFromSettings();
		generateHeightmap();
		updateTexture();
	}

	if (ImGui::Button("Regenerate terrain")) {
		generateHeightmap();
		updateTexture();
	}
	
	// static const float PREV_SIZE = 256;
	// ImGui::Image((ImTextureID)(intptr_t)texID, ImVec2(PREV_SIZE, PREV_SIZE));
	if (ImGui::Button("Print settings struct")) {
		settings.printSettings();
	}
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
// [0.0f,1.0f]) and updates pixels vector with uint16 values if update_pixels is true
// Pass update_pixels as false if don't need to recalculate pixels yet
void Noise::generateHeightmap(bool update_pixels) {
	int index = 0;

	float local_min = 1.0f;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			float fx = static_cast<float>(x) + settings.offset_x;
			float fy = static_cast<float>(y) + settings.offset_y;
			
			// Apply domain warping if enabled
			if (settings.use_domain_warp) {
				domainWarp.DomainWarp(fx, fy);
			}
			
			float data = noise.GetNoise(fx, fy);
			float normal = (data + 1.0f) / 2.0f;
			normal = powf(normal, settings.noise_exp);
			local_min = std::fminf(local_min, normal);
			heightmap[index++] = normal;
		}
	}

	min_height = local_min;
	if (update_pixels) {
		updatePixelsWithHeightmap();
	}
}

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

	domainWarp.SetSeed(settings.domain_seed);
	domainWarp.SetFrequency(settings.domain_frequency);
	domainWarp.SetDomainWarpType(settings.domain_warp_type);
	domainWarp.SetDomainWarpAmp(settings.domain_warp_amp);
	domainWarp.SetFractalType(settings.domain_warp_fractal_type);
	domainWarp.SetFractalOctaves(settings.domain_fractal_octaves);
	domainWarp.SetFractalGain(settings.domain_fractal_gain);

}

void Noise::setNoiseSettings(NoiseSettings new_settings) {
	settings = new_settings;
	updateNoiseFromSettings();
	generateHeightmap();
	updateTexture();
}

void Noise::setHeightmap(const std::vector<float> &new_heightmap) {
	if (new_heightmap.size() != heightmap.size()) {
		// XXX - should update the size outside if changing sizes for some reason
		throw std::runtime_error("heightmap size mismatch");
	}
	heightmap = new_heightmap;
	updatePixelsWithHeightmap();
	updateTexture();
}

void Noise::setHeightPixels(const std::vector<uint16_t> &new_pixels) {
	if (new_pixels.size() != pixels.size()) {
		throw std::runtime_error("pixels size mismatch");
	}
	pixels = new_pixels;
	updateTexture();
}

void Noise::changeTextureSize(int w, int h) {
	width = w;
	height = h;
	pixels.resize(width * height);
	heightmap.resize(width * height);

	generateHeightmap();
	createTexture();

	// TODO - implement this later, needs to update the stuff in base terrain
	//throw std::runtime_error("Implement this again later");
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

