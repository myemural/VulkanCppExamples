set(CURRENT_TARGET_NAME OmnidirectionalShadowMapping)
set(CURRENT_EXAMPLE_NAME "Omnidirectional Shadow Mapping with Point Lights")
set(CURRENT_LIB_NAMES Common BasicShadowMappingBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})