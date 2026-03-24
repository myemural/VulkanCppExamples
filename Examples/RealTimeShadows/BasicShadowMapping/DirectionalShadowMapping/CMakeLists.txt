set(CURRENT_TARGET_NAME DirectionalShadowMapping)
set(CURRENT_EXAMPLE_NAME "Directional Shadow Mapping")
set(CURRENT_LIB_NAMES Common BasicShadowMappingBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})