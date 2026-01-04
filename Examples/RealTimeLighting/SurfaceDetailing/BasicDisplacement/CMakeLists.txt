set(CURRENT_TARGET_NAME BasicDisplacement)
set(CURRENT_EXAMPLE_NAME "Displacement Mapping without Tessellation")
set(CURRENT_LIB_NAMES Common SurfaceDetailingBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})