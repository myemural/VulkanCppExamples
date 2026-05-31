set(CURRENT_TARGET_NAME ScreenSpaceReflections)
set(CURRENT_EXAMPLE_NAME "Screen-Space Reflections (SSR)")
set(CURRENT_LIB_NAMES Common IblAndReflectionsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})