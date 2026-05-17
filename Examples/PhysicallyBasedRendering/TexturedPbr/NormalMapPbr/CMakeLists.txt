set(CURRENT_TARGET_NAME NormalMapPbr)
set(CURRENT_EXAMPLE_NAME "Normal Map in PBR")
set(CURRENT_LIB_NAMES Common TexturedPbrBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})