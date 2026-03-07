set(CURRENT_TARGET_NAME HybridAlphaBlending)
set(CURRENT_EXAMPLE_NAME "Using Alpha Blending in Hybrid Lighting Architectures")
set(CURRENT_LIB_NAMES Common TransparencyTechniquesBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})