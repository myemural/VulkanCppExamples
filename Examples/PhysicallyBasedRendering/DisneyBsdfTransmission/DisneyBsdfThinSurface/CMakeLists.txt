set(CURRENT_TARGET_NAME DisneyBsdfThinSurface)
set(CURRENT_EXAMPLE_NAME "Thin Surface Approximation in Disney BSDF")
set(CURRENT_LIB_NAMES Common DisneyBsdfTransmissionBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
