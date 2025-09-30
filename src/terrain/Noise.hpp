#pragma once

#include <FastNoiseLite.h>
#include <cstdint>
#include <vector>
#include "../opengl.hpp"
#include "HydraulicErosion.hpp"

namespace Terrain {
	struct NoiseSettings {
		// General settings
		int seed = 1337; // The seed to use for noise generation
		float frequency = 0.01f; // Noise frequency
		FastNoiseLite::NoiseType noise_type = FastNoiseLite::NoiseType_Perlin; // The noise type, perlin by default
		float noise_exp = 1.0f; // Power exponent to use on noise (not too useful but fun)

		// Settings for fractal types
		FastNoiseLite::FractalType fractal_type = FastNoiseLite::FractalType_None; // Fractal type to use
		int fractal_octaves = 3; // Amount of layers to use for fractal noise
		float fractal_lacunarity = 2.0f; // Frequency multiplier between each octave for fractal noise
		float fractal_gain = 0.5f; // Relative strength of noise in each fractal layer compared to last
		float fractal_weighted_strength = 0.0f; // Octave weighting for fractal types;
		float fractal_pingpong_strength = 2.0f; // Strength for ping pong fractal type (not needed for other stuff)

		// Settings for the cellular noise type
		// Distance function for calculating cell for a given point
		FastNoiseLite::CellularDistanceFunction cellular_dist_function = FastNoiseLite::CellularDistanceFunction_EuclideanSq;
		FastNoiseLite::CellularReturnType cellular_return_type = FastNoiseLite::CellularReturnType_Distance; // Value that cellular function returns
		float cellular_jitter = 1.0f; // Maximum distance a cellular point can move from grid pos

		// Domain warp settings
		bool use_domain_warp = false; // Whether or not to use domain warp
		FastNoiseLite::DomainWarpType domain_warp_type = FastNoiseLite::DomainWarpType_OpenSimplex2; // domain warp algorithm
		float domain_warp_amp = 1.0f; // Max warp distance from original pos
		// TODO - copy the like frequency settings and stuff into here since domain warp needs separate
	};

	class Noise {
	public:
		GLuint texID = 0; // The texture ID
		std::vector<float> heightmap; // The heightmap as float for erosion
		std::vector<uint16_t> pixels; // The heightmap as uint16 for the gpu texture
		HydraulicErosion erosionSim; // The simulator for erosion simulation (TODO - maybe move this to baseterrain)
		bool use_erosion = true;
		
		static constexpr int DEFAULT_WIDTH = 512;
		static constexpr int DEFAULT_HEIGHT = 512;
		
		int width = DEFAULT_WIDTH;
		int height = DEFAULT_HEIGHT;
		
		FastNoiseLite noise;
		FastNoiseLite domainWarp;
		NoiseSettings settings; // The noise settings to use

		Noise();
		void makeEditUI(bool use_own_window = false);
		void updatePixels(bool apply_erosion = false);
		void applyErosion();
		void updateNoiseFromSettings(); // Updates the noise generators with the noise settings in the struct
		void setNoiseSettings(NoiseSettings new_settings);

		// Change the size of the noise texture and update related info
		void changeTextureSize(int w, int h);

	private:
		void updateTexture(bool reuse_old = true);
		void createTexture();

		// Make the imgui interface part for the cellular settings cuz it's like 2 named fields..
		bool makeCellularSettingsUI();

		// Update the pixels vector by converting the values in the heightmap vector
		void updatePixelsWithHeightmap();
	};
}
