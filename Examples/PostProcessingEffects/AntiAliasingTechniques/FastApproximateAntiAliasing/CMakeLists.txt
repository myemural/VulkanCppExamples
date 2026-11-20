set(CURRENT_TARGET_NAME FastApproximateAntiAliasing)
set(CURRENT_EXAMPLE_NAME "Fast Approximate Anti-Aliasing (FXAA)")
set(CURRENT_LIB_NAMES Common AntiAliasingTechniquesBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
