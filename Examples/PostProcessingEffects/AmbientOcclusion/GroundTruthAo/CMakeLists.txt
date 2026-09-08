set(CURRENT_TARGET_NAME GroundTruthAo)
set(CURRENT_EXAMPLE_NAME "Ground-Truth Ambient Occlusion (GTAO)")
set(CURRENT_LIB_NAMES Common AmbientOcclusionBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
