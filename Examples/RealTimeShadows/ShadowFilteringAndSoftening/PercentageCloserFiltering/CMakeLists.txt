set(CURRENT_TARGET_NAME PercentageCloserFiltering)
set(CURRENT_EXAMPLE_NAME "Percentage Closer Filtering (PCF)")
set(CURRENT_LIB_NAMES Common ShadowFilteringAndSofteningBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})