
set(sources
  "Noise.hpp"
  "Noise.cpp"
  "BaseTerrain.hpp"
  "BaseTerrain.cpp"
  "HydraulicErosion.hpp"
  "HydraulicErosion.cpp"
)

target_relative_sources(${CGRA_PROJECT} ${sources})
