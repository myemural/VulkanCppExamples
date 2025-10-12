set(CURRENT_TARGET_NAME GltfMeshWireframe)
set(CURRENT_EXAMPLE_NAME "Rendering glTF Mesh with Wireframe")
set(CURRENT_LIB_NAMES Utility VulkanFramework VulkanWrapper WindowWrapper ModelLoadingBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})