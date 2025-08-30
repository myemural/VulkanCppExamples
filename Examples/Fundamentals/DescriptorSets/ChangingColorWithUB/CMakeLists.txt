set(CURRENT_TARGET_NAME ChangingColorWithUB)
set(CURRENT_EXAMPLE_NAME "Changing Color of a Triangle with Uniform Buffer")
set(CURRENT_LIB_NAMES Utility VulkanFramework VulkanWrapper WindowWrapper DescriptorSetsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})