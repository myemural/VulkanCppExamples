set(CURRENT_TARGET_NAME PbrEnergyConservation)
set(CURRENT_EXAMPLE_NAME "Principle of Conservation of Energy and Tone Mapping")
set(CURRENT_LIB_NAMES Common BasicPbrBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})