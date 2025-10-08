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

		cgra::gl_mesh plane_mesh; // the plane mesh to draw with
		GLuint shader = 0;
		GLuint water_texture;
		GLuint water_normal_texture = 0;
		glm::mat4 model_transform = glm::mat4(1.0f); // Water model matrix, has height translation applied

		explicit WaterPlane(GLuint texid = 0);

		void update_transform(glm::vec3 model_scale, float sea_level);

		// For testing shader things
		float metallic = 0.5f;
		float smoothness = 0.77f;

		// Renderable methods
		GLuint getShader() override;
		void setProjViewUniforms(const glm::mat4 &view, const glm::mat4 &proj) const override;
		void draw() override;
		glm::mat4 getModelTransform() override; // Get the model transform,
	};
}
