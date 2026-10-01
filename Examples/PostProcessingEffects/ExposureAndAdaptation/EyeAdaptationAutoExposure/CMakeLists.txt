set(CURRENT_TARGET_NAME EyeAdaptationAutoExposure)
set(CURRENT_EXAMPLE_NAME "Eye (Temporal) Adaptation on Auto Exposure")
set(CURRENT_LIB_NAMES Common ExposureAndAdaptationBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
