#pragma once

#include <FastNoiseLite.h>
#include <cstdint>
#include <vector>
#include "../opengl.hpp"
#include "TerrainSettings.hpp"

namespace Terrain {
	class Noise {
	public:
		GLuint texID = 0; // The texture ID
		std::vector<float> heightmap; // The heightmap as float for erosion
		std::vector<uint16_t> pixels; // The heightmap as uint16 for the gpu texture
		
		static constexpr int DEFAULT_WIDTH = 512;
		static constexpr int DEFAULT_HEIGHT = 512;
		
		int width = DEFAULT_WIDTH;
		int height = DEFAULT_HEIGHT;
		float min_height = 0.0f; // The minimum height level in the generated heightmap, used to normalize the heights to a base level
		
		FastNoiseLite noise;
		FastNoiseLite domainWarp;
		NoiseSettings settings; // The noise settings to use

		Noise();
		void makeEditUI(bool use_own_window = false);
		void generateHeightmap(bool update_pixels = true);
		void updateNoiseFromSettings(); // Updates the noise generators with the noise settings in the struct
		void setNoiseSettings(NoiseSettings new_settings);
		void setHeightmap(const std::vector<float>& new_heightmap); // Set heightmap to new_heightmap and update texture
		void setHeightPixels(const std::vector<uint16_t>& new_pixels); // Set pixels and update texture

		// Change the size of the noise texture and update related info
		void changeTextureSize(int w, int h);

		// Update the pixels vector by converting the values in the heightmap vector
		void updatePixelsWithHeightmap();

	private:
		void updateTexture(bool reuse_old = true);
		void createTexture();

		// Make the imgui interface part for the cellular settings cuz it's like 2 named fields..
		bool makeCellularSettingsUI();
	};
}
