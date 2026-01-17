set(CURRENT_TARGET_NAME CubemapReflections)
set(CURRENT_EXAMPLE_NAME "Cubemap Reflections")
set(CURRENT_LIB_NAMES Common EnvironmentMappingBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})