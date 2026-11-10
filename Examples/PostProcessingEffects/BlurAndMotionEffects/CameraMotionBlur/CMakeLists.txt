set(CURRENT_TARGET_NAME CameraMotionBlur)
set(CURRENT_EXAMPLE_NAME "Camera Motion Blur via Depth Reprojection")
set(CURRENT_LIB_NAMES Common BlurAndMotionEffectsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
