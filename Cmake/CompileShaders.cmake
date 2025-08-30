function(compile_shaders_for_target TARGET_NAME)
    set(SHADER_BASE_DIR ${CMAKE_SOURCE_DIR}/Shaders/${CATEGORY_NAME}/${SUBCATEGORY_NAME}/${TARGET_NAME})

    set(ALL_SHADER_OUTPUTS)

    # Compile GLSL Shaders
    set(GLSL_SHADERS_DIR "${SHADER_BASE_DIR}/glsl")
    if(IS_DIRECTORY "${GLSL_SHADERS_DIR}")
        file(GLOB GLSL_SHADERS ${GLSL_SHADERS_DIR}/*.glsl)
        set(GLSL_SPIRV_OUTPUT_DIR "${GLSL_SHADERS_DIR}/spirv")
        file(MAKE_DIRECTORY "${GLSL_SPIRV_OUTPUT_DIR}")

        foreach(SHADER ${GLSL_SHADERS})
            get_filename_component(FILE_NAME ${SHADER} NAME)
            string(REPLACE ".glsl" "" FILE_BASE ${FILE_NAME})

            # Stage detection: vert=vertex, frag=fragment, comp=compute, etc.
            if(FILE_NAME MATCHES "\\.vert\\.")
                set(STAGE vert)
            elseif(FILE_NAME MATCHES "\\.frag\\.")
                set(STAGE frag)
            elseif(FILE_NAME MATCHES "\\.comp\\.")
                set(STAGE comp)
            elseif(FILE_NAME MATCHES "\\.geom\\.")
                set(STAGE geom)
            elseif(FILE_NAME MATCHES "\\.tesc\\.")
                set(STAGE tesc)
            elseif(FILE_NAME MATCHES "\\.tese\\.")
                set(STAGE tese)
            else()
                message(FATAL_ERROR "Shader stage not recognized in filename: ${FILE_NAME}")
            endif()

            set(GLSL_SPIRV_SHADER_PATH ${GLSL_SPIRV_OUTPUT_DIR}/${FILE_BASE}.spv)

            add_custom_command(
                    OUTPUT "${GLSL_SPIRV_SHADER_PATH}"
                    COMMAND ${CMAKE_COMMAND} -E echo "Started compiling for ${FILE_NAME}"
                    COMMAND glslc -fshader-stage=${STAGE} ${SHADER} -o ${GLSL_SPIRV_SHADER_PATH}
                    COMMAND ${CMAKE_COMMAND} -E echo "Compilation finished for ${FILE_NAME} -> ${GLSL_SPIRV_SHADER_PATH}"
                    DEPENDS "${SHADER}"
                    VERBATIM
            )
            list(APPEND ALL_SHADER_OUTPUTS "${GLSL_SPIRV_SHADER_PATH}")
        endforeach()
    endif ()

    # Compile HLSL Shaders
    set(HLSL_SHADERS_DIR "${SHADER_BASE_DIR}/hlsl")
    if(IS_DIRECTORY "${HLSL_SHADERS_DIR}")
        file(GLOB HLSL_SHADERS ${HLSL_SHADERS_DIR}/*.hlsl)
        set(HLSL_SPIRV_OUTPUT_DIR "${HLSL_SHADERS_DIR}/spirv")
        file(MAKE_DIRECTORY "${HLSL_SPIRV_OUTPUT_DIR}")

        foreach(SHADER ${HLSL_SHADERS})
            get_filename_component(FILE_NAME ${SHADER} NAME)
            string(REPLACE ".hlsl" "" FILE_BASE ${FILE_NAME})

            # Stage detection: vs=vertex, ps=pixel, cs=compute, etc.
            if(FILE_NAME MATCHES "\\.vs\\.")
                set(STAGE vs)
            elseif(FILE_NAME MATCHES "\\.ps\\.")
                set(STAGE ps)
            elseif(FILE_NAME MATCHES "\\.cs\\.")
                set(STAGE cs)
            elseif(FILE_NAME MATCHES "\\.hs\\.")
                set(STAGE hs)
            elseif(FILE_NAME MATCHES "\\.ds\\.")
                set(STAGE ds)
            elseif(FILE_NAME MATCHES "\\.gs\\.")
                set(STAGE gs)
            else()
                message(FATAL_ERROR "HLSL shader stage not recognized: ${FILE_NAME}")
            endif()

            set(HLSL_SPIRV_SHADER_PATH ${HLSL_SPIRV_OUTPUT_DIR}/${FILE_BASE}.spv)

            add_custom_command(
                    OUTPUT "${HLSL_SPIRV_SHADER_PATH}"
                    COMMAND ${CMAKE_COMMAND} -E echo "Started compiling for ${FILE_NAME}"
                    COMMAND dxc -T ${STAGE}_6_0 -E main "${SHADER}" -Fo "${SPIRV_PATH}"
                    COMMAND ${CMAKE_COMMAND} -E echo "Compilation finished for ${FILE_NAME} -> ${SPIRV_PATH}"
                    DEPENDS "${SHADER}"
                    COMMENT "Compiling HLSL shader ${FILE_NAME}"
                    VERBATIM
            )
            list(APPEND ALL_SHADER_OUTPUTS "${HLSL_SPIRV_SHADER_PATH}")
        endforeach()
    endif ()

    # Add custom target and shader dependency
    if(ALL_SHADER_OUTPUTS)
        add_custom_target(${TARGET_NAME}_shaders DEPENDS ${ALL_SHADER_OUTPUTS})
        add_dependencies(${TARGET_NAME} ${TARGET_NAME}_shaders)
    endif()
endfunction()