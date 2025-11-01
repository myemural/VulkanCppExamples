set(CURRENT_TARGET_NAME DrawingWithDifferentTopology)
set(CURRENT_EXAMPLE_NAME "Drawing with Different Topologies")
set(CURRENT_LIB_NAMES Common BasicsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})