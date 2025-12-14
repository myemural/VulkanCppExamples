set(CURRENT_TARGET_NAME PointLight)
set(CURRENT_EXAMPLE_NAME "Point Light")
set(CURRENT_LIB_NAMES Common LightSourcesBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})