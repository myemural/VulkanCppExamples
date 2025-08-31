set(CURRENT_TARGET_NAME WrapAndFilteringModes)
set(CURRENT_EXAMPLE_NAME "Using Different Wrap and Filtering Modes")
set(CURRENT_LIB_NAMES Utility VulkanFramework VulkanWrapper WindowWrapper ImagesAndSamplersBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})