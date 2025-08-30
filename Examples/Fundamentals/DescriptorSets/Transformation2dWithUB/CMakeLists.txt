set(CURRENT_TARGET_NAME Transformation2dWithUB)
set(CURRENT_EXAMPLE_NAME "Rotating and Scaling a Square Constantly")
set(CURRENT_LIB_NAMES Utility VulkanFramework VulkanWrapper WindowWrapper DescriptorSetsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})