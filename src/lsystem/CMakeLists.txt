set(sources
	"node/common.cpp"
	"node/common.hpp"
	"node/node.cpp"
	"node/node.hpp"
	"node/tree.cpp"
	"node/tree.hpp"
	"node/bush.cpp"
	"node/bush.hpp"
)


# Add these sources to the project target
target_relative_sources(${CGRA_PROJECT} ${sources})
