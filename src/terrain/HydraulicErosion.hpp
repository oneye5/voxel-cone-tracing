#pragma once
#include <cstdint>
#include <random>
#include <vector>
#include <glm/glm.hpp>

namespace Terrain {
	struct ErosionSettings {
		int iterations = 100000;        // Number of droplet simulations
		int max_lifetime = 30;           // Max steps per droplet
		float inertia = 0.05f;          // How much droplet resists direction change (0-1)
		float capacity_s = 4.0f; // Sediment capacity (P_capacity)
		float min_slope = 0.01f;   // Minimum sediment capacity (Algorithm calls it P_minslope)
		float evaporate_speed = 0.01f;   // How fast water evaporates (0-1)
		float deposition = 0.3f;      // How fast sediment is deposited (0-1)
		float erode_speed = 0.3f;        // How fast erosion happens (0-1) (P_erosion)
		float gravity = 4.0f;           // Strength of gravity
		float start_velocity = 1.0f;        // Initial droplet speed
		float start_water = 1.0f;        // Initial droplet water volume
		int erosion_radius = 3;          // Radius of erosion brush
	};

	class HydraulicErosion {
	public:
		int width, height; // The width and height of the heightmap
		std::vector<float> heightmap; // float based heightmap
		ErosionSettings settings;
		std::mt19937 rng;
		std::uniform_real_distribution<float> uniform_dist;
		// TODO - add a seed field and a bool for whether to use same seed on new rng for each erosion simulation :p

		HydraulicErosion(int w, int h, unsigned int seed = std::random_device{}());

		// Set the heightmap to use for erosion simulation
		void setHeightmap(const std::vector<float>& heights);

		// Get the eroded heightmap but with values mapped to uint16
		std::vector<uint16_t> getHeightmapAsUint16() const;

		void simulate(int iterations = -1);

		const std::vector<float>& getHeightmap() const {
			return heightmap;
		}

		void renderUI(bool use_own_window = false);

		// Set the width and height of the simulation, should be followed by setting a new heightmap
		void setSimulationDims(int w, int h);

	private:
		// Helper functions for erosion simulation
		float getHeight(int x, int y) const;
		void setHeight(int x, int y, float height);
		float getHeightInterpolated(float x, float y) const;
		glm::vec2 calculateBilinearGradient(glm::vec2 pos) const;
		void applyErosion(float x, float y, float amount, int radius);
		void simulateDroplet();
	};
}
