set(CURRENT_TARGET_NAME CelShading)
set(CURRENT_EXAMPLE_NAME "Cel Shading with Toon Ramp")
set(CURRENT_LIB_NAMES Common StylizationAndImageEffectsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
