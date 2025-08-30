set(CURRENT_TARGET_NAME ArrayOfUB)
set(CURRENT_EXAMPLE_NAME "Multiple Transform with Descriptor Arrays")
set(CURRENT_LIB_NAMES Utility VulkanFramework VulkanWrapper WindowWrapper DescriptorSetsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})