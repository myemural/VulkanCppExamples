set(CURRENT_TARGET_NAME IndirectDrawing)
set(CURRENT_EXAMPLE_NAME "Reduce CPU Bottleneck with Indirect Drawing")
set(CURRENT_LIB_NAMES Common QueriesAndPerformanceBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})