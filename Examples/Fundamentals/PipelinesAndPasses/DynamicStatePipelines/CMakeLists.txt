set(CURRENT_TARGET_NAME DynamicStatePipelines)
set(CURRENT_EXAMPLE_NAME "Changing Blending Factor with Dynamic State")
set(CURRENT_LIB_NAMES Common PipelinesAndPassesBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})