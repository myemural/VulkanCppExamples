set(CURRENT_TARGET_NAME TextureAtlases)
set(CURRENT_EXAMPLE_NAME "Using Texture Atlases")
set(CURRENT_LIB_NAMES Utility VulkanFramework VulkanWrapper WindowWrapper ImagesAndSamplersBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})