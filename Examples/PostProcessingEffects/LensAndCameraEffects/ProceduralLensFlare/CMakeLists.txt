set(CURRENT_TARGET_NAME ProceduralLensFlare)
set(CURRENT_EXAMPLE_NAME "Procedural Lens Flare Effect")
set(CURRENT_LIB_NAMES Common LensAndCameraEffectsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
