set(CURRENT_TARGET_NAME PbrMetallicWorkflow)
set(CURRENT_EXAMPLE_NAME "Metallic Workflow in PBR")
set(CURRENT_LIB_NAMES Common BasicPbrBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})