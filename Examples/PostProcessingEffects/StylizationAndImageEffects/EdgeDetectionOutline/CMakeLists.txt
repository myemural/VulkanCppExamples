set(CURRENT_TARGET_NAME EdgeDetectionOutline)
set(CURRENT_EXAMPLE_NAME "Edge Detection and Screen Space Outlines")
set(CURRENT_LIB_NAMES Common StylizationAndImageEffectsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
