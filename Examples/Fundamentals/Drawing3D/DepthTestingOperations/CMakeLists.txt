set(CURRENT_TARGET_NAME DepthTestingOperations)
set(CURRENT_EXAMPLE_NAME "Depth Testing Operations")
set(CURRENT_LIB_NAMES Common Drawing3dBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})