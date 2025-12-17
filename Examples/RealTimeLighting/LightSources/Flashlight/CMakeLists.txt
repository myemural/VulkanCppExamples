set(CURRENT_TARGET_NAME Flashlight)
set(CURRENT_EXAMPLE_NAME "Flashlight with Soft-Cutoff")
set(CURRENT_LIB_NAMES Common LightSourcesBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})