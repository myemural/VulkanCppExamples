set(CURRENT_TARGET_NAME SinglePassCubemapRendering)
set(CURRENT_EXAMPLE_NAME "Single-Pass Cubemap Rendering")
set(CURRENT_LIB_NAMES Common GeometryShadersBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})