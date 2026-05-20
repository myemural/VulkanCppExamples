set(CURRENT_TARGET_NAME RectangularAreaLights)
set(CURRENT_EXAMPLE_NAME "Rectangular Area Lights")
set(CURRENT_LIB_NAMES Common AreaLightsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})