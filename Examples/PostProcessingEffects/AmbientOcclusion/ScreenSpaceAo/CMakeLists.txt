set(CURRENT_TARGET_NAME ScreenSpaceAo)
set(CURRENT_EXAMPLE_NAME "Screen-Space Ambient Occlusion (SSAO)")
set(CURRENT_LIB_NAMES Common AmbientOcclusionBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
