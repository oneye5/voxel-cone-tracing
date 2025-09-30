#pragma once

#include "Noise.hpp"
#include "renderable.hpp"
#include "cgra/cgra_mesh.hpp"

namespace Terrain {

	struct TerrainSettings {
		float max_height = 1.0f; // Max height of the terrain
		float min_height = 0.0; // Min height of the terrain
	};

	struct PlaneTerrain {
		cgra::gl_mesh mesh;

		glm::mat4 init_transform = glm::mat4(1.0f);
	};
	
	class BaseTerrain : public Renderable {
	public:
		GLuint shader;
		Noise t_noise; // The noise to use for the terrain, contains texture
		PlaneTerrain t_mesh; // The plane mesh to use
		int plane_subs = 512;
		TerrainSettings t_settings;
		bool useTexturing = true;
		GLuint water_texture;
		GLuint sand_texture;
		GLuint grass_texture;
		GLuint rock_texture;
		GLuint snow_texture;

		bool useFakedLighting = false; // whether to use faked lighting for testing

		void renderUI();
		
		BaseTerrain();

		// Regenerate the plane mesh with a specific subdivision count
		void changePlaneSubdivision(int subs);

		GLuint getShader() override;

		void setProjViewUniforms(const glm::mat4 &view, const glm::mat4 &proj) const override;

		void draw() override;

		glm::mat4 getModelTransform() override;

	private:
		// Load the textures for the terrain and store them in the fields
		void loadTextures();

	};
}
