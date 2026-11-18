set(CURRENT_TARGET_NAME KuwaharaFilter)
set(CURRENT_EXAMPLE_NAME "Kuwahara Filter (Basic, Generalized, Anisotropic)")
set(CURRENT_LIB_NAMES Common StylizationAndImageEffectsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
