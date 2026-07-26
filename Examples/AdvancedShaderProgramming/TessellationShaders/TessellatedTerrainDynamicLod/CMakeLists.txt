set(CURRENT_TARGET_NAME TessellatedTerrainDynamicLod)
set(CURRENT_EXAMPLE_NAME "Tessellated Terrain Rendering with Dynamic LOD")
set(CURRENT_LIB_NAMES Common TessellationShadersBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})