set(CURRENT_TARGET_NAME SpotlightShadowMapping)
set(CURRENT_EXAMPLE_NAME "Spotlight Shadow Mapping")
set(CURRENT_LIB_NAMES Common BasicShadowMappingBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})