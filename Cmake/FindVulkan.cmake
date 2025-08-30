# FindVulkan.cmake
#
# This file contains code that tries to find Vulkan SDK on your system. First it searches the system paths, then it
# searches the user-provided VULKAN_SDK or VULKAN_PATH variables.
#
# Usage:
#   find_package(Vulkan REQUIRED)
#
# Exported variables:
#   Vulkan_FOUND        - TRUE if found
#   Vulkan_INCLUDE_DIRS - Vulkan header directories
#   Vulkan_LIBRARIES    - Vulkan libraries

find_path(Vulkan_INCLUDE_DIR
        NAMES vulkan/vulkan.h
        HINTS
        $ENV{VULKAN_SDK}/include
        $ENV{VULKAN_PATH}/include
        ${VULKAN_SDK}/include
        ${VULKAN_PATH}/include
        PATH_SUFFIXES include
)

# Platform specific sets
if(WIN32)
    # Windows
    set(VULKAN_LIBRARY_NAMES vulkan-1)
elseif(APPLE)
    # MoltenVK for MacOS
    set(VULKAN_LIBRARY_NAMES vulkan MoltenVK)
else()
    # Linux
    set(VULKAN_LIBRARY_NAMES vulkan)
endif()

find_library(Vulkan_LIBRARY
        NAMES ${VULKAN_LIBRARY_NAMES}
        HINTS
        $ENV{VULKAN_SDK}/lib
        $ENV{VULKAN_PATH}/lib
        ${VULKAN_SDK}/lib
        ${VULKAN_PATH}/lib
        PATH_SUFFIXES lib lib64
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vulkan
        REQUIRED_VARS Vulkan_LIBRARY Vulkan_INCLUDE_DIR
        HANDLE_COMPONENTS
)

if(Vulkan_FOUND)
    set(Vulkan_LIBRARIES ${Vulkan_LIBRARY})
    set(Vulkan_INCLUDE_DIRS ${Vulkan_INCLUDE_DIR})
endif()

mark_as_advanced(Vulkan_INCLUDE_DIR Vulkan_LIBRARY)