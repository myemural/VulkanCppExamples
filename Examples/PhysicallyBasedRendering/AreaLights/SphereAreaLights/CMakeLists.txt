set(CURRENT_TARGET_NAME SphereAreaLights)
set(CURRENT_EXAMPLE_NAME "Sphere Area Lights with Representative Point Method")
set(CURRENT_LIB_NAMES Common AreaLightsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})