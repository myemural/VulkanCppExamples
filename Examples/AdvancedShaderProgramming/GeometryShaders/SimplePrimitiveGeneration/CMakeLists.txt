set(CURRENT_TARGET_NAME SimplePrimitiveGeneration)
set(CURRENT_EXAMPLE_NAME "Simple Primitive Generation")
set(CURRENT_LIB_NAMES Common GeometryShadersBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})