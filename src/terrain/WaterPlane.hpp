#pragma once
#include <string>

#include "renderable.hpp"
#include <glm/glm.hpp>

#include "cgra/cgra_mesh.hpp"

namespace Terrain {
	class WaterPlane : public Renderable {
	public:
		// Path to the water texture
		inline static const std::string WATER_TEXTURE_PATH = CGRA_SRCDIR + std::string("//res//textures//terrain//tex_Water.jpg");
		inline static const std::string WATER_NORMAL_TEX_PATH = CGRA_SRCDIR + std::string("//res//textures//terrain//water_normal.jpg");
		inline static const std::string WATER_DUDV_TEX_PATH = CGRA_SRCDIR + std::string("//res//textures//terrain//water_dudv.png");

		cgra::gl_mesh plane_mesh; // the plane mesh to draw with
		GLuint shader = 0;
		GLuint water_texture;
		GLuint water_normal_texture = 0;
		GLuint water_dudv_texture = 0;
		glm::mat4 model_transform = glm::mat4(1.0f); // Water model matrix, has height translation applied
		float size_scalar = 1.2; // scale to apply to the x and z components of the mesh, to make it extend past terrain

		explicit WaterPlane(GLuint texid = 0);

		void update_transform(glm::vec3 model_scale, float sea_level);

		float wave_speed = 0.0005f;

		// For testing shader things
		float metallic = 0.5f;
		float smoothness = 0.77f;
		float move_factor = 0.0f; // wave move factor

		// Renderable methods
		GLuint getShader() override;
		void setProjViewUniforms(const glm::mat4 &view, const glm::mat4 &proj) const override;
		void draw() override;
		glm::mat4 getModelTransform() override; // Get the model transform,
	};
}
