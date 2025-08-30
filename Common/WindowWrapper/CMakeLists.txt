file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(WindowWrapper ${SRC_FILES})
target_link_libraries(WindowWrapper glfw3)