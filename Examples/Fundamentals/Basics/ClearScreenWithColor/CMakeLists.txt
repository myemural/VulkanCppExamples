set(CURRENT_TARGET_NAME ClearScreenWithColor)
set(CURRENT_EXAMPLE_NAME "Clear Screen with a Color")
set(CURRENT_LIB_NAMES VulkanFramework VulkanWrapper WindowWrapper BasicsBase)

include(BuildTarget)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
