set(CURRENT_TARGET_NAME BokehDepthOfField)
set(CURRENT_EXAMPLE_NAME "Bokeh Depth of Field")
set(CURRENT_LIB_NAMES Common BlurAndMotionEffectsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
