set(CURRENT_TARGET_NAME PrimaryColorCorrections)
set(CURRENT_EXAMPLE_NAME "Primary Color Corrections on LDR (Brightness, Contrast, Saturation, Hue Shift)")
set(CURRENT_LIB_NAMES Common ColorProcessingToneMappingBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
