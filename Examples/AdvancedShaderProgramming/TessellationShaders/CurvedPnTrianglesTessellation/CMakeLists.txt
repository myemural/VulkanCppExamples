set(CURRENT_TARGET_NAME CurvedPnTrianglesTessellation)
set(CURRENT_EXAMPLE_NAME "Model Tessellation with Curved PN Triangles")
set(CURRENT_LIB_NAMES Common TessellationShadersBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})