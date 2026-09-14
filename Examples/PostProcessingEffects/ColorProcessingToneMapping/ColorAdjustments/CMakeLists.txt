set(CURRENT_TARGET_NAME ColorAdjustments)
set(CURRENT_EXAMPLE_NAME "Color Adjustments (Brightness, Contrast, Saturation, Hue Shift, Color Temperature, White Balance)")
set(CURRENT_LIB_NAMES Common ColorProcessingToneMappingBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
