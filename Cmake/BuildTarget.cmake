macro(build_target TARGET_NAME LIB_NAMES EXAMPLE_NAME)
    file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

    add_executable(${TARGET_NAME} ${SRC_FILES})
    target_link_libraries(${TARGET_NAME} PUBLIC ${LIB_NAMES})
    target_compile_definitions(${TARGET_NAME} PRIVATE
            EXAMPLE_APPLICATION_NAME="${CATEGORY_NAME} - ${SUBCATEGORY_NAME} - ${EXAMPLE_NAME}"
            SHADERS_DIR="${CMAKE_SOURCE_DIR}/Shaders/${CATEGORY_NAME}/${SUBCATEGORY_NAME}/${TARGET_NAME}/"
            ASSETS_DIR="${CMAKE_SOURCE_DIR}/Assets/")

    if (ENABLE_EXAMPLE_TESTS)
        add_test(NAME "${TARGET_NAME}_test"
                COMMAND ${TARGET_NAME}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    endif ()
endmacro()