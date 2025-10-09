#pragma once
#include <glm/glm.hpp>
#include <string>
#include "lsystem.hpp"
#include "cgra/cgra_mesh.hpp"
#include "opengl.hpp"
#include "renderable.hpp"

namespace plant {
	struct Mesh : Renderable {
		cgra::gl_mesh mesh;
		GLuint shader;
		glm::mat4 modelTransform;

		Mesh();
		Mesh(GLuint shader);

		virtual void draw() override;
		virtual void setProjViewUniforms(const glm::mat4& view, const glm::mat4& proj) const override;
		virtual GLuint getShader() override;
		virtual glm::mat4 getModelTransform() override;
	};

	struct PlantData {
		lsystem::ruleset rules;
		std::string seed;
		GLuint trunk_shader;
		GLuint canopy_shader;
		// TODO: Textures?
	};

	class Plant {
		// TODO: Store like an rng or something
		lsystem::ruleset ruleset;
		std::string seed;
		/** Current representation of the plant as a string */
		std::string current;
		void recalculate_mesh();

		public:
		Mesh trunk;
		Mesh canopy;

		Plant(std::string seed, GLuint trunk_shader, GLuint canopy_shader, lsystem::ruleset ruleset, int steps = 0);
		Plant(PlantData data, int steps = 4);
		void grow(int steps = 1);
	};

	struct KnownPlants{
		PlantData tree;
	};
	extern KnownPlants known_plants;

	struct create_plants_input {
		glm::vec3 pos;
	};

	std::vector<Plant> create_plants(std::vector<create_plants_input> inputs);
}
