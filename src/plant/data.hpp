#pragma once
#include <vector>
#include "lsystem.hpp"
#include "opengl.hpp"
#include "cgra/cgra_image.hpp"

namespace plant::data {
	struct PlantData {
		lsystem::ruleset initial;
		float size;
		GLuint trunk_shader;
		GLuint canopy_shader;
		GLuint trunk_texture_colour;
		GLuint trunk_texture_normal;
		GLuint canopy_texture_colour;
		GLuint canopy_texture_normal;
	};

	struct KnownPlants{
		PlantData tree;
		PlantData bush;
	};

	extern KnownPlants known_plants;

	void init_known_plants();
}
