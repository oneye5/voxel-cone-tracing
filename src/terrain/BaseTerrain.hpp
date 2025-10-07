#pragma once

#include "HydraulicErosion.hpp"
#include "Noise.hpp"
#include "renderable.hpp"
#include "WaterPlane.hpp"
#include "cgra/cgra_mesh.hpp"

namespace Terrain {

	struct TerrainSettings {
		float max_height = 1.0f; // Max height of the terrain
		float min_height = 0.0; // Min height of the terrain
		float amplitude = 1.0f; // Amplitude of the height generation
		float sea_level = 0.0f; // The y level to draw the water plane at

		glm::vec3 model_scale = glm::vec3(5.0f, 2.5f, 5.0f); // The scale of the terrain

		float min_rock_slope = 0.95f; // The min slope for rock texturing (0-1)
		float max_grass_slope = 1.0f; // The max slope for grass texturing (0-1)
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
		TerrainSettings t_settings;

		HydraulicErosion t_erosion;
		bool erosion_running = false; // Whether or not the erosion sim is currently running (in real-time)

		bool useTexturing = true;
		GLuint texture1; // The first texture, bottom most (default water)
		GLuint texture2; // The second texture (default sand)
		GLuint texture3; // Third texture (default dirt)
		GLuint texture4; // Forth texture (default rock)
		GLuint texture5; // Fifth texture (default snow)

		WaterPlane* water_plane = nullptr; // The water plane (passed as pointer so renderer can draw it and terrain can set settings)

		bool useFakedLighting = false; // whether to use faked lighting for testing

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

	private:
		// Load the textures for the terrain and store them in the fields
		void loadTextures();

	};
}
