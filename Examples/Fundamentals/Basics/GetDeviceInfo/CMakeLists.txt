set(CURRENT_TARGET_NAME GetDeviceInfo)
set(CURRENT_EXAMPLE_NAME "Getting Vulkan and Device Info")
set(CURRENT_LIB_NAMES VulkanWrapper)

include(BuildTarget)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")