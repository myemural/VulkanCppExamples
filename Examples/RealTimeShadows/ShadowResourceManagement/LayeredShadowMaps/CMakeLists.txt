set(CURRENT_TARGET_NAME LayeredShadowMaps)
set(CURRENT_EXAMPLE_NAME "Layered Shadow Maps with Texture Arrays")
set(CURRENT_LIB_NAMES Common ShadowResourceManagementBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})