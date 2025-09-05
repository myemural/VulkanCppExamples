cmake_minimum_required(VERSION 3.31)
project(VulkanCodeExamples)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Cmake")

option(ENABLE_EXAMPLE_TESTS "Enable running example tests" ON)

# Try to auto locate Vulkan
find_package(Vulkan REQUIRED)

if(Vulkan_FOUND)
    message(STATUS "Vulkan found: ${Vulkan_LIBRARIES}")
    include_directories(AFTER ${Vulkan_INCLUDE_DIRS})
endif()

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(ThirdParty/glfw)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glfw/include")

set(GLM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glm")
include_directories("${GLM_DIR}/include")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/stb")

include_directories("Common/Utility")
include_directories("Common/VulkanFramework")
include_directories("Common/VulkanWrapper")
include_directories("Common/WindowWrapper")

if (ENABLE_EXAMPLE_TESTS)
    enable_testing()
endif ()

add_subdirectory(Common)
add_subdirectory("Examples/Fundamentals")

