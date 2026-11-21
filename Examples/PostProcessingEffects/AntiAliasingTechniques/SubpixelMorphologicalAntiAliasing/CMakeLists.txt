set(CURRENT_TARGET_NAME SubpixelMorphologicalAntiAliasing)
set(CURRENT_EXAMPLE_NAME "Subpixel Morphological Anti-Aliasing (SMAA 1x)")
set(CURRENT_LIB_NAMES Common AntiAliasingTechniquesBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
