#pragma once
#include <glm/glm.hpp>
#include <string>
#include "lsystem.hpp"
#include "cgra/cgra_mesh.hpp"
#include "opengl.hpp"
#include "renderable.hpp"
#include "renderer.hpp"

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
		Plant(PlantData data, int steps = 2);
		void grow(int steps = 1);
	};

	struct KnownPlants{
		PlantData tree;
	};
	extern KnownPlants known_plants;

	struct plants_manager_input {
		glm::vec3 pos;

		friend std::ostream& operator<<(std::ostream& os, plants_manager_input const& m) {
			return os << "PM_Input["
				<< "pos=(" << m.pos.x <<", " << m.pos.y << ", " << m.pos.z << ")"
				<<"]";
		}
	};

	// std::vector<Plant> create_plants(std::vector<create_plants_input> inputs);

	class PlantManager {
		std::vector<std::pair<unsigned int, Plant>> plants;
		Renderer *renderer;

		public:
		PlantManager();
		PlantManager(Renderer* renderer);

		void grow(int step = 1);
		void clear();
		void update_plants(const std::vector<plants_manager_input>& inputs);
	};
}
