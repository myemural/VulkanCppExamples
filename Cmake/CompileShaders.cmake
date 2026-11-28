function(compile_shaders_for_target TARGET_NAME)
    set(SHADER_BASE_DIR ${CMAKE_SOURCE_DIR}/Shaders/${CATEGORY_NAME}/${SUBCATEGORY_NAME}/${TARGET_NAME})

    set(ALL_SHADER_OUTPUTS)

    # Compile GLSL Shaders
    set(GLSL_SHADERS_DIR "${SHADER_BASE_DIR}/glsl")
    if(IS_DIRECTORY "${GLSL_SHADERS_DIR}")
        file(GLOB GLSL_SHADERS ${GLSL_SHADERS_DIR}/*.glsl)
        set(GLSL_SPIRV_OUTPUT_DIR "${GLSL_SHADERS_DIR}/spirv")
        file(MAKE_DIRECTORY "${GLSL_SPIRV_OUTPUT_DIR}")

        # SPIR-V version is selected once for the whole target:
        # - Ray-tracing stages or ray queries need SPIR-V 1.4 with Vulkan 1.2 environment
        # - Mesh or task shaders need SPIR-V 1.4
        # - Otherwise SPIR-V 1.3 is used
        set(GLSL_NEEDS_SPV14 FALSE)
        set(GLSL_NEEDS_RAY_TRACING FALSE)
        foreach(shader ${GLSL_SHADERS})
            if(shader MATCHES "\\.(mesh|task)\\.")
                set(GLSL_NEEDS_SPV14 TRUE)
            elseif(shader MATCHES "\\.(rgen|rmiss|rchit|rahit|rint|rcall)\\.")
                set(GLSL_NEEDS_RAY_TRACING TRUE)
            else()
                file(STRINGS "${shader}" RAY_QUERY_LINES REGEX "GL_EXT_ray_query")
                if(RAY_QUERY_LINES)
                    set(GLSL_NEEDS_RAY_TRACING TRUE)
                endif()
            endif()
        endforeach()

        if(GLSL_NEEDS_RAY_TRACING)
            set(TARGET_SPV --target-env=vulkan1.2 --target-spv=spv1.4)
        elseif(GLSL_NEEDS_SPV14)
            set(TARGET_SPV --target-spv=spv1.4)
        else()
            set(TARGET_SPV --target-spv=spv1.3)
        endif()

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
            elseif(FILE_NAME MATCHES "\\.mesh\\.")
                set(STAGE mesh)
            elseif(FILE_NAME MATCHES "\\.task\\.")
                set(STAGE task)
            elseif(FILE_NAME MATCHES "\\.rgen\\.")
                set(STAGE rgen)
            elseif(FILE_NAME MATCHES "\\.rmiss\\.")
                set(STAGE rmiss)
            elseif(FILE_NAME MATCHES "\\.rchit\\.")
                set(STAGE rchit)
            elseif(FILE_NAME MATCHES "\\.rahit\\.")
                set(STAGE rahit)
            elseif(FILE_NAME MATCHES "\\.rint\\.")
                set(STAGE rint)
            elseif(FILE_NAME MATCHES "\\.rcall\\.")
                set(STAGE rcall)
            else()
                message(FATAL_ERROR "Shader stage not recognized in filename: ${FILE_NAME}")
            endif()

            set(GLSL_SPIRV_SHADER_PATH ${GLSL_SPIRV_OUTPUT_DIR}/${FILE_BASE}.spv)

            add_custom_command(
                    OUTPUT "${GLSL_SPIRV_SHADER_PATH}"
                    COMMAND ${CMAKE_COMMAND} -E echo "Started compiling for ${FILE_NAME}"
                    COMMAND glslc -fshader-stage=${STAGE} ${SHADER} -o ${GLSL_SPIRV_SHADER_PATH} ${TARGET_SPV}
                    COMMAND ${CMAKE_COMMAND} -E echo "Compilation finished for ${FILE_NAME} -> ${GLSL_SPIRV_SHADER_PATH}"
                    DEPENDS "${SHADER}"
                    VERBATIM
            )
            list(APPEND ALL_SHADER_OUTPUTS "${GLSL_SPIRV_SHADER_PATH}")
        endforeach()
    endif ()

    # Compile HLSL Shaders
    set(HLSL_SHADERS_DIR "${SHADER_BASE_DIR}/hlsl")
    if(IS_DIRECTORY "${HLSL_SHADERS_DIR}" AND DEFINED ENV{VULKAN_SDK})
        file(GLOB HLSL_SHADERS ${HLSL_SHADERS_DIR}/*.hlsl)
        set(HLSL_SPIRV_OUTPUT_DIR "${HLSL_SHADERS_DIR}/spirv")
        file(MAKE_DIRECTORY "${HLSL_SPIRV_OUTPUT_DIR}")

        foreach(SHADER ${HLSL_SHADERS})
            get_filename_component(FILE_NAME ${SHADER} NAME)
            string(REPLACE ".hlsl" "" FILE_BASE ${FILE_NAME})

            # Stage detection: vs=vertex, ps=pixel, cs=compute, etc.
            if(FILE_NAME MATCHES "\\.vs\\.")
                string(REPLACE ".vs" ".vert" FILE_BASE ${FILE_BASE})
                set(STAGE vs)
            elseif(FILE_NAME MATCHES "\\.ps\\.")
                string(REPLACE ".ps" ".frag" FILE_BASE ${FILE_BASE})
                set(STAGE ps)
            elseif(FILE_NAME MATCHES "\\.cs\\.")
                string(REPLACE ".cs" ".comp" FILE_BASE ${FILE_BASE})
                set(STAGE cs)
            elseif(FILE_NAME MATCHES "\\.hs\\.")
                string(REPLACE ".hs" ".tesc" FILE_BASE ${FILE_BASE})
                set(STAGE hs)
            elseif(FILE_NAME MATCHES "\\.ds\\.")
                string(REPLACE ".ds" ".tese" FILE_BASE ${FILE_BASE})
                set(STAGE ds)
            elseif(FILE_NAME MATCHES "\\.gs\\.")
                string(REPLACE ".gs" ".geom" FILE_BASE ${FILE_BASE})
                set(STAGE gs)
            elseif(FILE_NAME MATCHES "\\.ms\\.")
                string(REPLACE ".ms" ".mesh" FILE_BASE ${FILE_BASE})
                set(STAGE ms)
            elseif(FILE_NAME MATCHES "\\.as\\.")
                string(REPLACE ".as" ".task" FILE_BASE ${FILE_BASE})
                set(STAGE as)
            else()
                message(FATAL_ERROR "HLSL shader stage not recognized: ${FILE_NAME}")
            endif()

            set(HLSL_SPIRV_SHADER_PATH ${HLSL_SPIRV_OUTPUT_DIR}/${FILE_BASE}.spv)

            add_custom_command(
                    OUTPUT "${HLSL_SPIRV_SHADER_PATH}"
                    COMMAND ${CMAKE_COMMAND} -E echo "Started compiling for ${FILE_NAME}"
                    COMMAND "$ENV{VULKAN_SDK}/bin/dxc" -spirv -T ${STAGE}_6_0 -E main "${SHADER}" -Fo "${HLSL_SPIRV_SHADER_PATH}"
                    COMMAND ${CMAKE_COMMAND} -E echo "Compilation finished for ${FILE_NAME} -> ${HLSL_SPIRV_SHADER_PATH}"
                    DEPENDS "${SHADER}"
                    COMMENT "Compiling HLSL shader ${FILE_NAME}"
                    VERBATIM
            )
            list(APPEND ALL_SHADER_OUTPUTS "${HLSL_SPIRV_SHADER_PATH}")
        endforeach()
    endif ()

    # Compile Slang Shaders
    set(SLANG_SHADERS_DIR "${SHADER_BASE_DIR}/slang")
    if(IS_DIRECTORY "${SLANG_SHADERS_DIR}" AND DEFINED ENV{VULKAN_SDK})
        file(GLOB SLANG_SHADERS ${SLANG_SHADERS_DIR}/*.slang)
        set(SLANG_SPIRV_OUTPUT_DIR "${SLANG_SHADERS_DIR}/spirv")
        file(MAKE_DIRECTORY "${SLANG_SPIRV_OUTPUT_DIR}")

        foreach(SHADER ${SLANG_SHADERS})
            get_filename_component(FILE_NAME ${SHADER} NAME)
            string(REPLACE ".slang" "" FILE_BASE ${FILE_NAME})

            # Stage detection: vert=vertex, frag=fragment, comp=compute, etc.
            if(FILE_NAME MATCHES "\\.vert\\.")
                set(STAGE vertex)
            elseif(FILE_NAME MATCHES "\\.frag\\.")
                set(STAGE fragment)
            elseif(FILE_NAME MATCHES "\\.comp\\.")
                set(STAGE compute)
            elseif(FILE_NAME MATCHES "\\.geom\\.")
                set(STAGE geometry)
            elseif(FILE_NAME MATCHES "\\.tesc\\.")
                set(STAGE hull)
            elseif(FILE_NAME MATCHES "\\.tese\\.")
                set(STAGE domain)
            elseif(FILE_NAME MATCHES "\\.mesh\\.")
                set(STAGE mesh)
            elseif(FILE_NAME MATCHES "\\.task\\.")
                set(STAGE amplification)
            else()
                message(FATAL_ERROR "Slang shader stage not recognized: ${FILE_NAME}")
            endif()

            set(SLANG_SPIRV_SHADER_PATH "${SLANG_SPIRV_OUTPUT_DIR}/${FILE_BASE}.spv")

            ### NOTE: For now use SPIR-V 1.3 version
            add_custom_command(
                    OUTPUT "${SLANG_SPIRV_SHADER_PATH}"
                    COMMAND ${CMAKE_COMMAND} -E echo "Started compiling Slang shader for ${FILE_NAME}"
                    COMMAND "$ENV{VULKAN_SDK}/bin/slangc" "${SHADER}" -entry main -stage ${STAGE} -target spirv -profile spirv_1_3 -o "${SLANG_SPIRV_SHADER_PATH}"
                    COMMAND ${CMAKE_COMMAND} -E echo "Compilation finished for ${FILE_NAME} -> ${SLANG_SPIRV_SHADER_PATH}"
                    DEPENDS "${SHADER}"
                    COMMENT "Compiling Slang shader ${FILE_NAME}"
                    VERBATIM
            )
            list(APPEND ALL_SHADER_OUTPUTS "${SLANG_SPIRV_SHADER_PATH}")
        endforeach()
    endif()

    # Add custom target and shader dependency
    if(ALL_SHADER_OUTPUTS)
        add_custom_target(${TARGET_NAME}_shaders DEPENDS ${ALL_SHADER_OUTPUTS})
        add_dependencies(${TARGET_NAME} ${TARGET_NAME}_shaders)
    endif()
endfunction()