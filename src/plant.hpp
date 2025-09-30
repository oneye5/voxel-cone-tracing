#pragma once
#include <glm/glm.hpp>
#include "lsystem.hpp"
#include "cgra/cgra_mesh.hpp"
#include "opengl.hpp"

namespace plant {
	class Plant {
		// TODO: Store like an rng or something
		cgra::gl_mesh mesh;
		GLuint shader;

		lsystem::ruleset ruleset;
		std::string seed;
		/** Current representation of the plant as a string */
		std::string current;
		void recalculate_mesh();

		public:
		Plant(std::string seed, lsystem::ruleset ruleset, int steps = 0, GLuint shader = 0);
		void grow(int steps = 1);
		void draw(const glm::mat4 &modelTransform, const glm::mat4 &view, const glm::mat4 proj);
	};
}
