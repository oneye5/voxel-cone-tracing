#pragma once
#include <glm/glm.hpp>
#include "lsystem.hpp"
#include "cgra/cgra_mesh.hpp"
#include "opengl.hpp"

namespace plant {
	class Plant {
		// TODO: Store like an rng or something
		cgra::gl_mesh trunk;
		cgra::gl_mesh canopy;
		GLuint trunk_shader;
		GLuint canopy_shader;

		lsystem::ruleset ruleset;
		std::string seed;
		/** Current representation of the plant as a string */
		std::string current;
		void recalculate_mesh();

		public:
		Plant(std::string seed, lsystem::ruleset ruleset, int steps = 0, GLuint trunk_shader = 0, GLuint canopy_shader = 0);
		void grow(int steps = 1);
		void draw(const glm::mat4 &modelTransform, const glm::mat4 &view, const glm::mat4 proj);
	};
}
