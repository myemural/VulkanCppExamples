set(CURRENT_TARGET_NAME AlphaToCoverageTransparency)
set(CURRENT_EXAMPLE_NAME "Cutout Transparency with Alpha to Coverage")
set(CURRENT_LIB_NAMES Common TransparencyTechniquesBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})