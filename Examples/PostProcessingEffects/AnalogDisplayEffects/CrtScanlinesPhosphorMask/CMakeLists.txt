set(CURRENT_TARGET_NAME CrtScanlinesPhosphorMask)
set(CURRENT_EXAMPLE_NAME "CRT Scanlines and Phosphor Masks")
set(CURRENT_LIB_NAMES Common AnalogDisplayEffectsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
