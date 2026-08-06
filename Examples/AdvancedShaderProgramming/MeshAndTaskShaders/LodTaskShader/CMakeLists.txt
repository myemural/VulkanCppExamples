set(CURRENT_TARGET_NAME LodTaskShader)
set(CURRENT_EXAMPLE_NAME "LOD Rendering with Task Shader")
set(CURRENT_LIB_NAMES Common MeshAndTaskShadersBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})