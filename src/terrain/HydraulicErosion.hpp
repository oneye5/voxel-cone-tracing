#pragma once
#include <cstdint>
#include <random>
#include <vector>
#include <glm/glm.hpp>

namespace Terrain {
	// Hydraulic particle droplet.
	struct Particle {
		glm::vec2 c_pos;
		glm::vec2 c_dir = glm::vec2(0.0f);
		float c_vel;
		float c_water;
		float c_sediment = 0.0f;
		int c_steps = 0; // How many iterations has this cell had
		bool valid = true; // Whether the particle is valid (still has lifetime, on the terrain etc)
	};

	struct ErosionSettings {
		int iterations = 300000;        // Number of droplet simulations
		int max_lifetime = 50;           // Max steps per droplet
		float inertia = 0.30f;          // How much droplet resists direction change (0-1)
		float capacity_s = 4.0f; // Sediment capacity (P_capacity)
		float min_slope = 0.01f;   // Minimum sediment capacity (Algorithm calls it P_minslope)
		float evaporate_speed = 0.01f;   // How fast water evaporates (0-1)
		float deposit_speed = 0.3f;      // How fast sediment is deposited (0-1)
		float erode_speed = 0.3f;        // How fast erosion happens (0-1) (P_erosion)
		float gravity = 9.81f;           // Strength of gravity
		float start_velocity = 1.0f;        // Initial droplet speed
		float start_water = 1.0f;        // Initial droplet water volume
		int erosion_radius = 3;          // Radius of erosion brush

		int particles_per_frame = 5000; // Number of particles/ iterations to simulate each frame when running in real-time
	};

	class HydraulicErosion {
	public:
		Particle c_particle; // current particle being simulated
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

		// Run the erosion simulation without viewing progress (so run whole simulation as batch)
		void simulate(int iterations = -1);

		// Start a new erosion simulation for real time viewing
		void newSimulation(const std::vector<float>& init_heights, int w, int h);
		void stepSimulation();
		int iterations_ran = 0; // Amount of iterations executed, for real-time

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
		float getBilinearHeight(glm::vec2 pos) const;
		glm::vec2 calculateBilinearGradient(glm::vec2 pos) const;
		void applyErosion(glm::vec2 pos, float amount, int radius);
		void applyDeposition(glm::vec2 pos, float amount);

		// Simulate current droplet for number of steps specified (default max_lifetime for batch and steps_per_frame for real-time)
		void simulateDroplet(int steps);

		Particle createParticle() {
			float x = uniform_dist(rng) * static_cast<float>(width - 1);
			float y = uniform_dist(rng) * static_cast<float>(height - 1);
			glm::vec2 pos(x, y);

			return Particle{.c_pos = pos, .c_vel = settings.start_velocity, .c_water = settings.start_water};
		}
	};
}
