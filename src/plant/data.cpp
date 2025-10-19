#include "plant/data.hpp"
#include "lsystem/node/tree.hpp"
#include "cgra/cgra_shader.hpp"

using namespace plant::data;

KnownPlants plant::data::known_plants;

static void tree(PlantData &data) {
	data.initial = lsystem::ruleset{new lsystem::node::tree::Branch()};
	{
		cgra::shader_builder sb;
		sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//plant_trunk_vert.glsl"));
		sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//plant_trunk_frag.glsl"));
		sb.set_shader(GL_GEOMETRY_SHADER, CGRA_SRCDIR + std::string("//res//shaders//plant_trunk_geom.glsl"));
		data.trunk_shader = sb.build();
	}
	{
		cgra::shader_builder sb;
		sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//plant_canopy_vert.glsl"));
		sb.set_shader(GL_GEOMETRY_SHADER, CGRA_SRCDIR + std::string("//res//shaders//plant_canopy_geom.glsl"));
		sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//plant_canopy_frag.glsl"));
		data.canopy_shader = sb.build();
	}
}

void plant::data::init_known_plants() {
	tree(known_plants.tree);
}
