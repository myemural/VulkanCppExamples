set(CURRENT_TARGET_NAME FluidSimulationLbm)
set(CURRENT_EXAMPLE_NAME "Fluid Simulation with Lattice Boltzmann Method (LBM)")
set(CURRENT_LIB_NAMES Common AdvancedComputeShadersBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
