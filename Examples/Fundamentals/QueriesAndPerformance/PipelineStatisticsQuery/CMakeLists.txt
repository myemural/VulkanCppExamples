set(CURRENT_TARGET_NAME PipelineStatisticsQuery)
set(CURRENT_EXAMPLE_NAME "Measuring Pipeline Statistics")
set(CURRENT_LIB_NAMES Common QueriesAndPerformanceBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})