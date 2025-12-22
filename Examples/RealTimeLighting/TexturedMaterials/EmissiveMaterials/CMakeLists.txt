set(CURRENT_TARGET_NAME EmissiveMaterials)
set(CURRENT_EXAMPLE_NAME "Emissive Materials")
set(CURRENT_LIB_NAMES Common TexturedMaterialsBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})