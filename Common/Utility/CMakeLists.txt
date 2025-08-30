file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(Utility ${SRC_FILES})
target_compile_definitions(Utility PRIVATE STB_IMAGE_IMPLEMENTATION)