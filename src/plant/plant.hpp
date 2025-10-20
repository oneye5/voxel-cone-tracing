#pragma once
#include <glm/glm.hpp>
#include <random>
#include <string>
#include <vector>
#include "lsystem.hpp"
#include "opengl.hpp"
#include "renderer.hpp"
#include "plant/mesh.hpp"
#include "plant/data.hpp"

namespace plant {
	class Plant {
		std::minstd_rand rng;
		lsystem::ruleset current;
		void recalculate_mesh();
		float size;

		public:
		Mesh trunk;
		Mesh canopy;

		Plant(lsystem::ruleset current, GLuint trunk_shader, GLuint canopy_shader, unsigned long rng_seed, int steps = 0);
		Plant(data::PlantData data, int steps = 2);
		void grow(int steps = 1);
	};

	struct plants_manager_input {
		glm::vec3 pos;
		int type;

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
