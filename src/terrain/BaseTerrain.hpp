#pragma once

#include "HydraulicErosion.hpp"
#include "Noise.hpp"
#include "renderable.hpp"
#include "WaterPlane.hpp"
#include "plant.hpp"
#include "cgra/cgra_mesh.hpp"

namespace Terrain {

	struct TreePlacementSettings {
		float min_distance = 1.0f; // Min distance apart
		int max_trees = 10.0f; // The max number of trees to spawn
		int placement_attempts = 50; // Number of placement attempts (Stops if max_trees already chosen)
	};

	struct TerrainSettings {
		float max_height = 1.0f; // Max height of the terrain
		float amplitude = 1.0f; // Amplitude of the height generation
		float sea_level = 0.0f; // The y level to draw the water plane at

		glm::vec3 model_scale = glm::vec3(5.0f, 2.5f, 5.0f); // The scale of the terrain

		float min_rock_slope = 0.95f; // The min slope for rock texturing (0-1)
		float max_grass_slope = 1.0f; // The max slope for grass texturing (0-1)

		bool use_triplanar_mapping = true;
		float tex_base_scalar = 8.0f; // The scale to apply to the uv coordinates for texture mapping
		float triplanar_sharpness = 4.0f; // Sharpness of the triplanar blending
	};

	struct PlaneTerrain {
		cgra::gl_mesh mesh;

		glm::mat4 init_transform = glm::mat4(1.0f);
		// Update the model transform matrix, calculates translation so terrain will be centered.
		void updateTransformCentered(glm::vec3 scale);
	};

	struct Textures {
		static GLuint water;
		static GLuint sand;
		static GLuint grass;
		static GLuint rock;
		static GLuint snow;
	};
	
	class BaseTerrain : public Renderable {
	public:
		// The default amount to scale the terrain up by (for model transform)
		static constexpr float DEFAULT_TERRAIN_SCALE = 10.0f;

		GLuint shader;
		Noise t_noise; // The noise to use for the terrain, contains texture
		PlaneTerrain t_mesh; // The plane mesh to use
		int plane_subs = 512;
		TreePlacementSettings tree_settings;
		TerrainSettings t_settings;
		plant::PlantManager * plant_manager;

		HydraulicErosion t_erosion;
		bool erosion_running = false; // Whether or not the erosion sim is currently running (in real-time)

		bool useTexturing = true;
		GLuint water_texture; // The first texture, bottom most (default water)
		GLuint sand_texture; // The second texture (default sand)
		GLuint dirt_texture; // Third texture (default dirt)
		GLuint rock_texture; // Forth texture (default rock)
		GLuint snow_texture; // Fifth texture (default snow)

		WaterPlane* water_plane = nullptr; // The water plane (passed as pointer so renderer can draw it and terrain can set settings)

		bool useFakedLighting = false; // whether to use faked lighting for testing
		bool draw_from_min = true; // Whether or not to get the terrain shader to draw with the min height at y=0

		void renderUI();
		BaseTerrain();
		// Regenerate the plane mesh with a specific subdivision count
		void changePlaneSubdivision(int subs);

		// Apply erosion to the heightmap in t_noise
		void applyErosion();
		// Step real-time erosion simulation once
		void stepErosion();

		GLuint getShader() override;
		void setProjViewUniforms(const glm::mat4 &view, const glm::mat4 &proj) const override;
		void draw() override;
		glm::mat4 getModelTransform() override;

		// Calculate new tree placement positions based on tree_settings and send the data to the plant manager
		void calculateAndSendTreePlacements(int seed = -1);
		// Basic function to send a vector of tree positions to the tree manager, send empty to prevent trees maybe
		void sendTreePlacements(std::vector<plant::plants_manager_input> &positions);

	private:
		// Load the textures for the terrain and store them in the fields
		void loadTextures();
		// Take a vec2 of x,z position from 0-1 and map it to the actual terrain position when rendered (using the size scalars and heightmap etc)
		glm::vec3 normalizedXZToWorldPos(const glm::vec2& n_pos);
		// Approximate the y position at provided normalize 0-1 x,z point and return the float value
		float approximateYAtPoint(const glm::vec2& pos);
	};
}
