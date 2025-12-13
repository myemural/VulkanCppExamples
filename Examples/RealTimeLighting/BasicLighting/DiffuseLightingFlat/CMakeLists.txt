set(CURRENT_TARGET_NAME DiffuseLightingFlat)
set(CURRENT_EXAMPLE_NAME "Diffuse Lighting in Flat Shading")
set(CURRENT_LIB_NAMES Common BasicLightingBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})