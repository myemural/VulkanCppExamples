set(CURRENT_TARGET_NAME RayTracedTriangle)
set(CURRENT_EXAMPLE_NAME "Drawing a Triangle with Ray Tracing Pipeline")
set(CURRENT_LIB_NAMES Common RayTracingFundamentalsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
