# Add all source Files in dir
set(sources
	"data.cpp"
	"mesh.cpp"
	"plant.hpp"
	"data.hpp"
	"plant.cpp"
	"mesh.hpp"
	"node/node.hpp"
	"node/common.hpp"
	"node/common.cpp"
	"node/node.cpp")
# Add these sources to the project target
target_relative_sources(${CGRA_PROJECT} ${sources})
