set(CURRENT_TARGET_NAME PerObjectMotionBlur)
set(CURRENT_EXAMPLE_NAME "Per-Object Motion Blur with Velocity Buffer")
set(CURRENT_LIB_NAMES Common BlurAndMotionEffectsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
