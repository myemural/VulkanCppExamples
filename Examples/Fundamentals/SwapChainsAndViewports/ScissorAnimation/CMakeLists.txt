set(CURRENT_TARGET_NAME ScissorAnimation)
set(CURRENT_EXAMPLE_NAME "Scissor Animation")
set(CURRENT_LIB_NAMES Common SwapChainsAndViewportsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})