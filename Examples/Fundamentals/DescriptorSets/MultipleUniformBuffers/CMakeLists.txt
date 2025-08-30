set(CURRENT_TARGET_NAME MultipleUniformBuffers)
set(CURRENT_EXAMPLE_NAME "Multiple Uniform Buffers")
set(CURRENT_LIB_NAMES Utility VulkanFramework VulkanWrapper WindowWrapper DescriptorSetsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})