set(CURRENT_TARGET_NAME TiledForwardShading)
set(CURRENT_EXAMPLE_NAME "Tiled Forward Shading (Forward+)")
set(CURRENT_LIB_NAMES Common LightingArchitecturesBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})