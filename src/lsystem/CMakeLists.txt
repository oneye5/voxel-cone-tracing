file(GLOB sources *.c *.cpp *.h *.hpp)

# Add these sources to the project target
target_relative_sources(${CGRA_PROJECT} ${sources})
