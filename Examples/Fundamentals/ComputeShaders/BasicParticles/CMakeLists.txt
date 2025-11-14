set(CURRENT_TARGET_NAME BasicParticles)
set(CURRENT_EXAMPLE_NAME "Basic GPU-Generated Particle Effect")
set(CURRENT_LIB_NAMES Common ComputeShadersBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})