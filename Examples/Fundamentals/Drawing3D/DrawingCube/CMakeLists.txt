set(CURRENT_TARGET_NAME DrawingCube)
set(CURRENT_EXAMPLE_NAME "Drawing a Cube")
set(CURRENT_LIB_NAMES Common Drawing3dBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})