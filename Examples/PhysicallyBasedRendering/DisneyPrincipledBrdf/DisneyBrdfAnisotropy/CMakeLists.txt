set(CURRENT_TARGET_NAME DisneyBrdfAnisotropy)
set(CURRENT_EXAMPLE_NAME "Anisotropic Specular in Disney BRDF")
set(CURRENT_LIB_NAMES Common DisneyPrincipledBrdfBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})