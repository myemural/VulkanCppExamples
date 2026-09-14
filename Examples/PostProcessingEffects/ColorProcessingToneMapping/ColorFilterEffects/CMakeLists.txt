set(CURRENT_TARGET_NAME ColorFilterEffects)
set(CURRENT_EXAMPLE_NAME "Color Filter Effects (Grayscale, Invert Colors, Sepia, Duotone, Channel Swap)")
set(CURRENT_LIB_NAMES Common ColorProcessingToneMappingBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
