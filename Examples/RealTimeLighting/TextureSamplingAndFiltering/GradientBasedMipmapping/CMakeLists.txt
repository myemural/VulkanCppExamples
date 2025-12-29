set(CURRENT_TARGET_NAME GradientBasedMipmapping)
set(CURRENT_EXAMPLE_NAME "Gradient-Based LOD Control with Anisotropy")
set(CURRENT_LIB_NAMES Common TextureSamplingAndFilteringBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})