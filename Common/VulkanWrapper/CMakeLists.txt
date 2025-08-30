file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(VulkanWrapper ${SRC_FILES})
target_link_libraries(VulkanWrapper ${Vulkan_LIBRARIES})