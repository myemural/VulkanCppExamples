set(CURRENT_TARGET_NAME BloomLensDirt)
set(CURRENT_EXAMPLE_NAME "Bloom with Lens Dirt and Anamorphic Stretch")
set(CURRENT_LIB_NAMES Common BloomAndGlowBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
