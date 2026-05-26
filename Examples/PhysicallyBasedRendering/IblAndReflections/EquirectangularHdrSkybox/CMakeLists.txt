set(CURRENT_TARGET_NAME EquirectangularHdrSkybox)
set(CURRENT_EXAMPLE_NAME "Using Equirectangular HDR Images as Skybox")
set(CURRENT_LIB_NAMES Common IblAndReflectionsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})