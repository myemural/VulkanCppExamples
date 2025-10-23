file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(VulkanFramework ${SRC_FILES})
target_link_libraries(VulkanFramework Utility VulkanWrapper)