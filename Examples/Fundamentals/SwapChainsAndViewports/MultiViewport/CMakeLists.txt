set(CURRENT_TARGET_NAME MultiViewport)
set(CURRENT_EXAMPLE_NAME "Using Multi-Viewports for Different Colors")
set(CURRENT_LIB_NAMES Utility VulkanFramework VulkanWrapper WindowWrapper SwapChainsAndViewportsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})