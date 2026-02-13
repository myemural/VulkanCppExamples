set(CURRENT_TARGET_NAME ClusteredForwardDepth)
set(CURRENT_EXAMPLE_NAME "Clustered Forward Shading with Depth Binning")
set(CURRENT_LIB_NAMES Common LightingArchitecturesBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})