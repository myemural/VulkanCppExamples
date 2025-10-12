file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(Utility ${SRC_FILES})
target_link_libraries(Utility tinygltf)