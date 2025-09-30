
set(sources
  "voxelizer.hpp"
  "voxelizer.cpp"
  "gBufferPrepass.hpp"
  "gBufferLightingPass.hpp"
)

target_relative_sources(${CGRA_PROJECT} ${sources})
