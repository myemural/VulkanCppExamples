set(CURRENT_TARGET_NAME HardwarePercentageCloserFiltering)
set(CURRENT_EXAMPLE_NAME "Hardware-Accelerated Percentage Closer Filtering")
set(CURRENT_LIB_NAMES Common ShadowFilteringAndSofteningBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})