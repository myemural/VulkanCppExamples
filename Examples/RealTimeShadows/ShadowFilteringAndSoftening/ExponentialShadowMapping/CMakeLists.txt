set(CURRENT_TARGET_NAME ExponentialShadowMapping)
set(CURRENT_EXAMPLE_NAME "Exponential Shadow Mapping (ESM)")
set(CURRENT_LIB_NAMES Common ShadowFilteringAndSofteningBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})