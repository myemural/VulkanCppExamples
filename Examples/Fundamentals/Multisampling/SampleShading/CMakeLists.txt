set(CURRENT_TARGET_NAME SampleShading)
set(CURRENT_EXAMPLE_NAME "Sample Shading")
set(CURRENT_LIB_NAMES Common MultisamplingBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})