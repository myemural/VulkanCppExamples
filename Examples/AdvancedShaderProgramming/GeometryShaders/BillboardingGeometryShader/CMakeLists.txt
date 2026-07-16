set(CURRENT_TARGET_NAME BillboardingGeometryShader)
set(CURRENT_EXAMPLE_NAME "Billboarding with Geometry Shader")
set(CURRENT_LIB_NAMES Common GeometryShadersBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})