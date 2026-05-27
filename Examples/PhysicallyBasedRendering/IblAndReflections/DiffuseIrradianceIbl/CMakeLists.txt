set(CURRENT_TARGET_NAME DiffuseIrradianceIbl)
set(CURRENT_EXAMPLE_NAME "Diffuse Irradiance Image Based Lighting")
set(CURRENT_LIB_NAMES Common IblAndReflectionsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})