#include "plant/data.hpp"
#include "lsystem/node/common.hpp"
#include "lsystem/node/tree.hpp"
#include "lsystem/node/bush.hpp"
#include "cgra/cgra_shader.hpp"
#include <memory>

using namespace plant::data;
using lsystem::node::common::RotateZ;
using lsystem::node::common::RotateY;

KnownPlants plant::data::known_plants;

static void tree(PlantData &data) {
	data.initial = lsystem::ruleset{lsystem::node::tree::leaf};
	data.size = 0.5;
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

	data.trunk_texture_colour = cgra::rgba_image(CGRA_SRCDIR "//res//textures//plant//bark//wood_0025_color_1k.jpg").uploadTexture();
	data.trunk_texture_normal = cgra::rgba_image(CGRA_SRCDIR "//res//textures//plant//bark//wood_0025_normal_opengl_1k.jpg").uploadTexture();
	data.canopy_texture_colour = cgra::rgba_image(CGRA_SRCDIR "//res//textures//plant//leaf//plants_0001_color_1k.jpg").uploadTexture();
	data.canopy_texture_normal = cgra::rgba_image(CGRA_SRCDIR "//res//textures//plant//leaf//plants_0001_normal_opengl_1k.jpg").uploadTexture();
}

static void bush(PlantData &data) {
	data.initial = lsystem::ruleset{std::make_shared<RotateY>(0.0f, 6.283185307f), lsystem::node::bush::vertical, std::make_shared<RotateZ>(0.5, 0.3), lsystem::node::bush::branch, lsystem::node::bush::leaf};
	data.size = 0.1;
	{
		cgra::shader_builder sb;
		sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//bush_trunk_vert.glsl"));
		sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//bush_trunk_frag.glsl"));
		sb.set_shader(GL_GEOMETRY_SHADER, CGRA_SRCDIR + std::string("//res//shaders//bush_trunk_geom.glsl"));
		data.trunk_shader = sb.build();
	}
	{
		cgra::shader_builder sb;
		sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//bush_canopy_vert.glsl"));
		sb.set_shader(GL_GEOMETRY_SHADER, CGRA_SRCDIR + std::string("//res//shaders//bush_canopy_geom.glsl"));
		sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//bush_canopy_frag.glsl"));
		data.canopy_shader = sb.build();
	}

	data.trunk_texture_colour = cgra::rgba_image(CGRA_SRCDIR "//res//textures//plant//bark//wood_0025_color_1k.jpg").uploadTexture();
	data.trunk_texture_normal = cgra::rgba_image(CGRA_SRCDIR "//res//textures//plant//bark//wood_0025_normal_opengl_1k.jpg").uploadTexture();
	data.canopy_texture_colour = cgra::rgba_image(CGRA_SRCDIR "//res//textures//plant//leaf//plants_0001_color_1k.jpg").uploadTexture();
	data.canopy_texture_normal = cgra::rgba_image(CGRA_SRCDIR "//res//textures//plant//leaf//plants_0001_normal_opengl_1k.jpg").uploadTexture();
}

void plant::data::init_known_plants() {
	tree(known_plants.tree);
	bush(known_plants.bush);
}
