set(CURRENT_TARGET_NAME BodyParticleCompute)
set(CURRENT_EXAMPLE_NAME "N-body Gravitational Particle Simulation with Compute Shader")
set(CURRENT_LIB_NAMES Common AdvancedComputeShadersBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
