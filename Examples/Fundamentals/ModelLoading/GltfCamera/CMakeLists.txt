set(CURRENT_TARGET_NAME GltfCamera)
set(CURRENT_EXAMPLE_NAME "Camera Usage with glTF")
set(CURRENT_LIB_NAMES Utility VulkanFramework VulkanWrapper WindowWrapper ModelLoadingBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})