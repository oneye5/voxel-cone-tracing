set(sources
	"node/node.hpp"
	"node/tree.cpp"
	"node/common.hpp"
	"node/tree.hpp"
	"node/common.cpp"
	"node/node.cpp")

# Add these sources to the project target
target_relative_sources(${CGRA_PROJECT} ${sources})
