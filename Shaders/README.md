# Shaders

This directory contains all shader files related to examples. Currently, only **GLSL** and **HLSL** shader files (and their compiled SPIR-V files) are available. **Slang** will be added in the future.

## Shader Compilers

If you build a one target that related to an example, related shader files will be compiled automatically to SPIR-V. You can change which type of shader will be compiled via example's itself. To successfully compile shaders, these compilers should be installed on your system:

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

Be careful about you should use the *dxc* version that support SPIR-V. For Windows, all these tools should be included in [LunarG SDK](https://vulkan.lunarg.com/sdk/home). Linux and macOS are not fully tested yet.

## Formats and Structures

### Shader File Name Formats

Supported shader stages and their file name formats should be like this:

| Shader Stage                            | GLSL File Format      | HLSL File Format    | SPIR-V File Format (GLSL/HLSL)             |
|-----------------------------------------|-----------------------|---------------------|--------------------------------------------|
| Vertex Shader                           | shader_name.vert.glsl | shader_name.vs.hlsl | shader_name.vert.spv / shader_name.vs.spv  |
| Fragment/Pixel Shader                   | shader_name.frag.glsl | shader_name.ps.hlsl | shader_name.frag.spv  / shader_name.ps.spv |
| Geometry Shader                         | shader_name.geom.glsl | shader_name.gs.hlsl | shader_name.geom.spv / shader_name.gs.spv  |
| Tessellation Control / Hull Shader      | shader_name.tesc.glsl | shader_name.hs.hlsl | shader_name.tesc.spv / shader_name.hs.spv  |
| Tessellation Evaluation / Domain Shader | shader_name.tese.glsl | shader_name.ds.hlsl | shader_name.tese.spv / shader_name.ds.spv  |
| Compute Shader                          | shader_name.comp.glsl | shader_name.cs.hlsl | shader_name.comp.spv / shader_name.cs.spv  |

### Directory Structure

- **GLSL:** CATEGORY_NAME/SUBCATEGORY_NAME/EXAMPLE_NAME/glsl
- **HLSL:** CATEGORY_NAME/SUBCATEGORY_NAME/EXAMPLE_NAME/hlsl

- **SPIRV Output for GLSL**: CATEGORY_NAME/SUBCATEGORY_NAME/EXAMPLE_NAME/glsl/spirv
- **SPIRV Output for HLSL**: CATEGORY_NAME/SUBCATEGORY_NAME/EXAMPLE_NAME/hlsl/spirv

## Shader Support by Example

In this section you can find which instance currently supports which shader type.

### Fundamentals

**Basics**

| Example                                                                                         | GLSL Support       | HLSL Support       |
|-------------------------------------------------------------------------------------------------|--------------------|--------------------|
| [Creating a Window](/Examples/Fundamentals/Basics/CreatingWindow)                               | No Shader          | No Shader          |
| [Getting Vulkan and Device Info](/Examples/Fundamentals/Basics/GetDeviceInfo)                   | No Shader          | No Shader          |
| [Clear Screen with a Color](/Examples/Fundamentals/Basics/ClearScreenWithColor)                 | No Shader          | No Shader          |
| [Drawing a Single Color Triangle](/Examples/Fundamentals/Basics/DrawingSingleColorTriangle)     | :white_check_mark: | :white_check_mark: |
| [Drawing Multiple Triangles](/Examples/Fundamentals/Basics/DrawingMultipleTriangles)            | :white_check_mark: | :white_check_mark: |
| [Drawing Multicolor Triangles](/Examples/Fundamentals/Basics/DrawingMulticolorTriangles)        | :white_check_mark: | :white_check_mark: |
| [Drawing a Quad with Indices](/Examples/Fundamentals/Basics/DrawingQuad)                        | :white_check_mark: | :white_check_mark: |
| [Drawing with Different Topologies](/Examples/Fundamentals/Basics/DrawingWithDifferentTopology) | :white_check_mark: | :white_check_mark: |
| [Using Staging Buffer](/Examples/Fundamentals/Basics/UsingStagingBuffer)                        | :white_check_mark: | :white_check_mark: |

**Descriptor Sets**

| Example                                                                                                               | GLSL Support       | HLSL Support       |
|-----------------------------------------------------------------------------------------------------------------------|--------------------|--------------------|
| [Changing Color of a Triangle with Uniform Buffer](/Examples/Fundamentals/DescriptorSets/ChangingColorWithUB)         | :white_check_mark: | :white_check_mark: |
| [Using Different UBs for Different Areas of the Screen](/Examples/Fundamentals/DescriptorSets/MultipleUniformBuffers) | :white_check_mark: | :white_check_mark: |
| [Rotating and Scaling a Square Constantly](/Examples/Fundamentals/DescriptorSets/Transformation2dWithUB)              | :white_check_mark: | :white_check_mark: |
| [Change Square Color with Keyboard Input](/Examples/Fundamentals/DescriptorSets/BasicPushConstants)                   | :white_check_mark: | :white_check_mark: |
| [Multiple Transform with Descriptor Arrays](/Examples/Fundamentals/DescriptorSets/ArrayOfUB)                          | :white_check_mark: | :white_check_mark: |

**Images and Samplers**

| Example                                                                                                    | GLSL Support       | HLSL Support       |
|------------------------------------------------------------------------------------------------------------|--------------------|--------------------|
| [Textured Quad](/Examples/Fundamentals/ImagesAndSamplers/TexturedQuad)                                     | :white_check_mark: | :white_check_mark: |
| [Combined Image Sampler](/Examples/Fundamentals/ImagesAndSamplers/CombinedImageSampler)                    | :white_check_mark: | :white_check_mark: |
| [Using Different Wrap and Filtering Modes](/Examples/Fundamentals/ImagesAndSamplers/WrapAndFilteringModes) | :white_check_mark: | :white_check_mark: |
| [Using Multiple Textures](/Examples/Fundamentals/ImagesAndSamplers/UsingMultipleTextures)                  | :white_check_mark: | :white_check_mark: |
| [Drawing Transparent Texture to Quads](/Examples/Fundamentals/ImagesAndSamplers/SimpleBlending)            | :white_check_mark: | :white_check_mark: |
| [Using Texture Atlases](/Examples/Fundamentals/ImagesAndSamplers/TextureAtlases)                           | :white_check_mark: | :white_check_mark: |

**Drawing 3D**

| Example                                                                             | GLSL Support       | HLSL Support       |
|-------------------------------------------------------------------------------------|--------------------|--------------------|
| [Drawing a Cube](/Examples/Fundamentals/Drawing3D/DrawingCube)                      | :white_check_mark: | :white_check_mark: |
| [Basic Camera Control](/Examples/Fundamentals/Drawing3D/BasicCameraControl)         | :white_check_mark: | :white_check_mark: |
| [Face Culling](/Examples/Fundamentals/Drawing3D/FaceCulling)                        | :white_check_mark: | :white_check_mark: |
| [Instanced Rendering](/Examples/Fundamentals/Drawing3D/InstancedRendering)          | :white_check_mark: | :white_check_mark: |
| [Depth Testing Operations](/Examples/Fundamentals/Drawing3D/DepthTestingOperations) | :white_check_mark: | :white_check_mark: |

**Pipelines And Passes**

| Example                                                                                                          | GLSL Support       | HLSL Support       |
|------------------------------------------------------------------------------------------------------------------|--------------------|--------------------|
| [Simple Ghosting Effect](/Examples/Fundamentals/PipelinesAndPasses/LoadStoreOps)                                 | :white_check_mark: | :white_check_mark: |
| [Changing Blending Factor with Dynamic State](/Examples/Fundamentals/PipelinesAndPasses/DynamicStatePipelines)   | :white_check_mark: | :white_check_mark: |
| [Using Multiple Pipelines and Pipeline Derivatives](/Examples/Fundamentals/PipelinesAndPasses/MultiplePipelines) | :white_check_mark: | :white_check_mark: |
| [Object Outlining with Stencil Testing](/Examples/Fundamentals/PipelinesAndPasses/BasicStencilTesting)           | :white_check_mark: | :white_check_mark: |
| [Using Multiple Subpasses and Input Attachments](/Examples/Fundamentals/PipelinesAndPasses/MultipleSubpasses)    | :white_check_mark: | :white_check_mark: |
| [Rendering Scene to a Texture](/Examples/Fundamentals/PipelinesAndPasses/OffscreenRendering)                     | :white_check_mark: | :white_check_mark: |

**Swap Chains and Viewports**

| Example                                                                                                   | GLSL Support       | HLSL Support        |
|-----------------------------------------------------------------------------------------------------------|--------------------|---------------------|
| [Handling Window Resizing](/Examples/Fundamentals/SwapChainsAndViewports/SwapChainRecreation)             | :white_check_mark: | :white_check_mark:  |
| [Using Multi-Viewports for Different Colors](/Examples/Fundamentals/SwapChainsAndViewports/MultiViewport) | :white_check_mark: | Cannot Be Supported |
| [Split Screen with Dynamic Viewport](/Examples/Fundamentals/SwapChainsAndViewports/DynamicViewport)       | :white_check_mark: | Cannot Be Supported |
| [Scissor Animation](/Examples/Fundamentals/SwapChainsAndViewports/ScissorAnimation)                       | :white_check_mark: | :white_check_mark:  |

**Model Loading**

| Example                                                                                                 | GLSL Support       | HLSL Support       |
|---------------------------------------------------------------------------------------------------------|--------------------|--------------------|
| [Rendering glTF Mesh with Wireframe](/Examples/Fundamentals/ModelLoading/GltfMeshWireframe)             | :white_check_mark: | :white_check_mark: |
| [Rendering Textured glTF Mesh](/Examples/Fundamentals/ModelLoading/GltfMeshTextured)                    | :white_check_mark: | :white_check_mark: |
| [Multiple glTF Meshes and Node Transformations](/Examples/Fundamentals/ModelLoading/GltfMultipleMeshes) | :white_check_mark: | :white_check_mark: |
| [Camera Usage with glTF](/Examples/Fundamentals/ModelLoading/GltfCamera)                                | :white_check_mark: | :white_check_mark: |