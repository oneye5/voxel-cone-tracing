#pragma once
#include <vector>
#include "lsystem.hpp"
#include "opengl.hpp"

namespace plant::data {
	struct PlantData {
		lsystem::ruleset initial;
		GLuint trunk_shader;
		GLuint canopy_shader;
		// TODO: Textures?
	};

	struct KnownPlants{
		PlantData tree;
	};

	extern KnownPlants known_plants;

	void init_known_plants();
}
