# Shaders

This directory contains all shader files related to examples. Currently, **GLSL**, **HLSL** and **Slang** shader files (and their compiled SPIR-V files) are available.

## Shader Compilers

If you build a target that related to an example, related shader files will be compiled automatically to SPIR-V. You can change which type of shader will be compiled via `SHADER_TYPE` variable in main `CMakeLists.txt` file of the project. To successfully compile shaders, these compilers should be installed on your system (versions can be different, but these are from my test system):

For *glslc* (GLSL):

```
shaderc v2023.8 v2025.3-10-gc7e73e8
spirv-tools v2025.4 v2022.4-970-g19042c89
glslang 11.1.0-1302-gd213562e
```

For *dxc* (HLSL):

```
dxcompiler.dll: 1.9 - 1.8.0.5049 (d72f75ee3); dxil.dll: 1.9(1.8.2505.32)
```

For *slangc* (Slang):

```
2025.17.2
```

Be careful about you should use the *dxc* version that support SPIR-V. For Windows, all these tools should be included in [LunarG SDK](https://vulkan.lunarg.com/sdk/home). Linux and macOS are not fully tested yet.

## Formats and Structures

### Shader File Name Formats

Supported shader stages and their file name formats should be like this:

| Shader Stage                            | GLSL File Format      | HLSL File Format    | Slang File Format      | SPIR-V File Format   |
|-----------------------------------------|-----------------------|---------------------|------------------------|----------------------|
| Vertex Shader                           | shader_name.vert.glsl | shader_name.vs.hlsl | shader_name.vert.slang | shader_name.vert.spv |
| Fragment/Pixel Shader                   | shader_name.frag.glsl | shader_name.ps.hlsl | shader_name.frag.slang | shader_name.frag.spv |
| Geometry Shader                         | shader_name.geom.glsl | shader_name.gs.hlsl | shader_name.geom.slang | shader_name.geom.spv |
| Tessellation Control / Hull Shader      | shader_name.tesc.glsl | shader_name.hs.hlsl | shader_name.tesc.slang | shader_name.tesc.spv |
| Tessellation Evaluation / Domain Shader | shader_name.tese.glsl | shader_name.ds.hlsl | shader_name.tese.slang | shader_name.tese.spv |
| Compute Shader                          | shader_name.comp.glsl | shader_name.cs.hlsl | shader_name.comp.slang | shader_name.comp.spv |
| Mesh Shader                             | shader_name.mesh.glsl | shader_name.ms.hlsl | shader_name.mesh.slang | shader_name.mesh.spv |
| Task Shader                             | shader_name.task.glsl | shader_name.as.hlsl | shader_name.task.slang | shader_name.task.spv |

### Directory Structure

- **GLSL:** CATEGORY_NAME/SUBCATEGORY_NAME/EXAMPLE_NAME/glsl
- **HLSL:** CATEGORY_NAME/SUBCATEGORY_NAME/EXAMPLE_NAME/hlsl
- **Slang:** CATEGORY_NAME/SUBCATEGORY_NAME/EXAMPLE_NAME/slang

- **SPIRV Output for GLSL**: CATEGORY_NAME/SUBCATEGORY_NAME/EXAMPLE_NAME/glsl/spirv
- **SPIRV Output for HLSL**: CATEGORY_NAME/SUBCATEGORY_NAME/EXAMPLE_NAME/hlsl/spirv
- **SPIRV Output for Slang**: CATEGORY_NAME/SUBCATEGORY_NAME/EXAMPLE_NAME/slang/spirv
