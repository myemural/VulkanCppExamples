set(CURRENT_TARGET_NAME BlurTechniquesComparison)
set(CURRENT_EXAMPLE_NAME "Blur Techniques Comparison (Box, Gaussian, Kawase, Dual Kawase)")
set(CURRENT_LIB_NAMES Common BlurAndMotionEffectsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
