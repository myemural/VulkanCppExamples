set(CURRENT_TARGET_NAME CubicBezierCurveTessellation)
set(CURRENT_EXAMPLE_NAME "Cubic Bézier Curve with Tessellation Shaders")
set(CURRENT_LIB_NAMES Common TessellationShadersBase)

include(BuildTarget)
include(CompileShaders)

build_target(${CURRENT_TARGET_NAME} "${CURRENT_LIB_NAMES}" "${CURRENT_EXAMPLE_NAME}")
compile_shaders_for_target(${CURRENT_TARGET_NAME})