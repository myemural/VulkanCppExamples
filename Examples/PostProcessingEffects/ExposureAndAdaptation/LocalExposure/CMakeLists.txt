set(CURRENT_TARGET_NAME LocalExposure)
set(CURRENT_EXAMPLE_NAME "Local Exposure with Bilateral Grid and Blurred Luminance")
set(CURRENT_LIB_NAMES Common ExposureAndAdaptationBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
