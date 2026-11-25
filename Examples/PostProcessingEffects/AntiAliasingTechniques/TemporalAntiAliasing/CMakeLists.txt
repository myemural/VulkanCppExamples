set(CURRENT_TARGET_NAME TemporalAntiAliasing)
set(CURRENT_EXAMPLE_NAME "Temporal Anti-Aliasing (TAA)")
set(CURRENT_LIB_NAMES Common AntiAliasingTechniquesBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
