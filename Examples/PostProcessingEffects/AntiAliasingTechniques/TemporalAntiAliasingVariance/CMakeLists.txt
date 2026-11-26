set(CURRENT_TARGET_NAME TemporalAntiAliasingVariance)
set(CURRENT_EXAMPLE_NAME "Improved TAA with Variance Clipping")
set(CURRENT_LIB_NAMES Common AntiAliasingTechniquesBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
