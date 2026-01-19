set(CURRENT_TARGET_NAME FresnelEffect)
set(CURRENT_EXAMPLE_NAME "Fresnel Effect")
set(CURRENT_LIB_NAMES Common EnvironmentMappingBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})