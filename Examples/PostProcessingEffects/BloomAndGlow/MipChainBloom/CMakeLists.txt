set(CURRENT_TARGET_NAME MipChainBloom)
set(CURRENT_EXAMPLE_NAME "Mip-Chain Bloom (Physically Based, Downsample/Upsample)")
set(CURRENT_LIB_NAMES Common BloomAndGlowBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})
