#include "renderable.hpp"
#include "cgra/cgra_mesh.hpp"

namespace plant {
	struct Mesh : Renderable {
		cgra::gl_mesh mesh;
		GLuint shader;
		glm::mat4 modelTransform;
		GLuint alt_vbo;

		Mesh();
		Mesh(GLuint shader);

		virtual void draw() override;
		virtual void setProjViewUniforms(const glm::mat4& view, const glm::mat4& proj) const override;
		virtual GLuint getShader() override;
		virtual glm::mat4 getModelTransform() override;
	};

}
