set(CURRENT_TARGET_NAME ExponentialVarianceWithGaussian)
set(CURRENT_EXAMPLE_NAME "Exponential Variance Shadow Mapping with Gaussian Blur")
set(CURRENT_LIB_NAMES Common ShadowFilteringAndSofteningBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})