set(CURRENT_TARGET_NAME CubemapRefractions)
set(CURRENT_EXAMPLE_NAME "Refraction with Cubemaps")
set(CURRENT_LIB_NAMES Common EnvironmentMappingBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})