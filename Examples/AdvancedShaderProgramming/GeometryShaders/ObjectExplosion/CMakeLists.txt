set(CURRENT_TARGET_NAME ObjectExplosion)
set(CURRENT_EXAMPLE_NAME "Object Explosion via Geometry Shader")
set(CURRENT_LIB_NAMES Common GeometryShadersBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})