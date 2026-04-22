set(CURRENT_TARGET_NAME CascadedShadowMaps)
set(CURRENT_EXAMPLE_NAME "Cascaded Shadow Maps (CSM)")
set(CURRENT_LIB_NAMES Common ShadowMapSplittingWarpingBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})