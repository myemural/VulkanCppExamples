set(CURRENT_TARGET_NAME RayTracingCompute)
set(CURRENT_EXAMPLE_NAME "Real-Time Ray Tracing using BVH Traversing with Compute Shader")
set(CURRENT_LIB_NAMES Common AdvancedComputeShadersBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
