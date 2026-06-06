set(CURRENT_TARGET_NAME DisneyDiffuseSpecular)
set(CURRENT_EXAMPLE_NAME "Fundamental Diffuse and Specular Model of the Disney BRDF")
set(CURRENT_LIB_NAMES Common DisneyPrincipledBrdfBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})