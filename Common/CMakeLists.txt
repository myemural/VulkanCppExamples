file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(Common SHARED ${SRC_FILES})
set_target_properties(Common PROPERTIES ENABLE_EXPORTS ON)
target_compile_definitions(Common PRIVATE COMMON_EXPORTS)
target_link_libraries(Common PRIVATE glfw tinygltf ${Vulkan_LIBRARIES})
