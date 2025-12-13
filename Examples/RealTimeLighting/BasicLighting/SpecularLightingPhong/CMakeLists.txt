set(CURRENT_TARGET_NAME SpecularLightingPhong)
set(CURRENT_EXAMPLE_NAME "Specular Lighting in Phong Shading")
set(CURRENT_LIB_NAMES Common BasicLightingBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})