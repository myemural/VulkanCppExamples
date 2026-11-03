set(CURRENT_TARGET_NAME GatherDepthOfField)
set(CURRENT_EXAMPLE_NAME "Depth of Field (Gather-Based, Thin Lens CoC)")
set(CURRENT_LIB_NAMES Common BlurAndMotionEffectsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
