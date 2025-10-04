set(CURRENT_TARGET_NAME SwapChainRecreation)
set(CURRENT_EXAMPLE_NAME "Handling Window Resizing")
set(CURRENT_LIB_NAMES Utility VulkanFramework VulkanWrapper WindowWrapper SwapChainsAndViewportsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})