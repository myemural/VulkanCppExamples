set(CURRENT_TARGET_NAME LoadStoreOps)
set(CURRENT_EXAMPLE_NAME "Simple Ghosting Effect")
set(CURRENT_LIB_NAMES Utility VulkanFramework VulkanWrapper WindowWrapper PipelinesAndPassesBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})