# Vulkan C++ Examples

This repository contains Vulkan examples written in modern C++20, structured in a granular, step-by-step progression from basic to advanced concepts. Each example builds incrementally on the previous ones, making it easier to understand Vulkan's low-level API in a practical way. These examples will first cover the basic features of Vulkan, and then include examples of implementing techniques such as real-time lighting, real-time shadowing, PBR, post-processing, ray tracing etc. using Vulkan.

![](/Docs/Images/ExamplesCollage.jpg)

- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Clone](#clone)
  - [Build and Run](#build-and-run)
- [General Info](#general-info)
  - [Common Parameters](#common-parameters)
- [All Examples](#all-examples)
  - [Fundamentals](#fundamentals)
  - [Real-Time Lighting](#real-time-lighting)
  - [Real-Time Shadows](#real-time-shadows)
  - [Physically Based Rendering](#physically-based-rendering)
  - [Advanced Shader Programming](#advanced-shader-programming)
- [Environment Support](#environment-support)
- [Contributing](#contributing)
  - [Code of Conduct](#code-of-conduct)
  - [How to Contribute?](#how-to-contribute)
- [License](#license)

## Getting Started

### Prerequisites

Before start, you should download and install the latest LunarG Vulkan SDK from here (*1.4.328.1* version has been tested): https://vulkan.lunarg.com/sdk/home

To compile and test the examples, *CMake 3.31+* is recommended. All third party libraries are added with using [Git Submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules). Therefore, you do not need to worry about installing them separately. Here is a list of 3rd party libraries (repos):
- [GLFW](https://github.com/glfw/glfw)
- [glm (OpenGL Mathematics)](https://github.com/g-truc/glm)
- [tinygltf](https://github.com/syoyo/tinygltf)

**NOTE:** Normally, the *stb_image* library is also used for texture loading. However, this library is already integrated with the *tinygltf* library.

### Clone

To clone this repository, you should run this command on your local terminal:

~~~bash
git clone --recursive https://github.com/myemural/VulkanCppExamples.git
~~~

Since this project contains various submodule dependencies, you will then need to run the following commands:

~~~bash
git submodule init
git submodule update
~~~

### Build and Run

Every example has its own directory and CMake target. You can build what you want with CMake command line tools or IDE tools. Additionally, the built examples create executable files in the `bin/<CONFIG>` directory. You can run any example from this directory.

## General Info

### Common Parameters

**Window Parameters**

| Parameter / Key    | Type          | Usage in Code             | Description                             | Default Value |
|--------------------|---------------|---------------------------|-----------------------------------------|---------------|
| Window.Width       | std::uint32_t | WindowParams::Width       | Initial width of the window (in pixel)  | 800           |
| Window.Height      | std::uint32_t | WindowParams::Height      | Initial height of the window (in pixel) | 600           |
| Window.Title       | std::string   | WindowParams::Title       | Title of the window                     |               |
| Window.Resizable   | bool          | WindowParams::Resizable   | Specifies window is resizable or not    | false         |

**Vulkan Parameters**

| Parameter / Key           | Type                           | Usage in Code                    | Description                       | Default Value            |
|---------------------------|--------------------------------|----------------------------------|-----------------------------------|--------------------------|
| Vulkan.ApplicationName    | std::string                    | VulkanParams::ApplicationName    | Name of the Vulkan application    |                          |
| Vulkan.VulkanApiVersion   | std::uint32_t                  | VulkanParams::VulkanApiVersion   | Version of the Vulkan API         | VK_API_VERSION_1_0       |
| Vulkan.ApplicationVersion | std::uint32_t                  | VulkanParams::ApplicationVersion | Version of the Vulkan application | VK_MAKE_VERSION(1, 0, 0) |
| Vulkan.EngineName         | std::string                    | VulkanParams::EngineName         | Name of the engine                | "DefaultEngine"          |
| Vulkan.EngineVersion      | std::uint32_t                  | VulkanParams::EngineVersion      | Version of the engine             | VK_MAKE_VERSION(1, 0, 0) |
| Vulkan.InstanceLayers     | std::vector&lt;std::string&gt; | VulkanParams::InstanceLayers     | List of the instance layers       |                          |
| Vulkan.InstanceExtensions | std::vector&lt;std::string&gt; | VulkanParams::InstanceExtensions | List of the instance extensions   |                          |

## All Examples

### [Fundamentals](/Examples/Fundamentals)

- **[Basics](/Examples/Fundamentals/Basics)**
  - [Creating a Window](/Examples/Fundamentals/Basics/CreatingWindow)
  - [Getting Vulkan and Device Info](/Examples/Fundamentals/Basics/GetDeviceInfo)
  - [Clear Screen with a Color](/Examples/Fundamentals/Basics/ClearScreenWithColor)
  - [Drawing a Single Color Triangle](/Examples/Fundamentals/Basics/DrawingSingleColorTriangle)
  - [Drawing Multiple Triangles](/Examples/Fundamentals/Basics/DrawingMultipleTriangles)
  - [Drawing Multicolor Triangles](/Examples/Fundamentals/Basics/DrawingMulticolorTriangles)
  - [Drawing a Quad with Indices](/Examples/Fundamentals/Basics/DrawingQuad)
  - [Drawing with Different Topologies](/Examples/Fundamentals/Basics/DrawingWithDifferentTopology)
  - [Using Staging Buffer](/Examples/Fundamentals/Basics/UsingStagingBuffer)
- **[Descriptor Sets](/Examples/Fundamentals/DescriptorSets)**
  - [Changing Color of a Triangle with Uniform Buffer](/Examples/Fundamentals/DescriptorSets/ChangingColorWithUB)
  - [Using Different UBs for Different Areas of the Screen](/Examples/Fundamentals/DescriptorSets/MultipleUniformBuffers)
  - [Rotating and Scaling a Square Constantly](/Examples/Fundamentals/DescriptorSets/Transformation2dWithUB)
  - [Change Square Color with Keyboard Input](/Examples/Fundamentals/DescriptorSets/BasicPushConstants)
  - [Multiple Transform with Descriptor Arrays](/Examples/Fundamentals/DescriptorSets/ArrayOfUB)
- **[Images and Samplers](/Examples/Fundamentals/ImagesAndSamplers)**
  - [Textured Quad](/Examples/Fundamentals/ImagesAndSamplers/TexturedQuad)
  - [Combined Image Sampler](/Examples/Fundamentals/ImagesAndSamplers/CombinedImageSampler)
  - [Using Different Wrap and Filtering Modes](/Examples/Fundamentals/ImagesAndSamplers/WrapAndFilteringModes)
  - [Using Multiple Textures](/Examples/Fundamentals/ImagesAndSamplers/UsingMultipleTextures)
  - [Drawing Transparent Texture to Quads](/Examples/Fundamentals/ImagesAndSamplers/SimpleBlending)
  - [Using Texture Atlases](/Examples/Fundamentals/ImagesAndSamplers/TextureAtlases)
- **[Drawing 3D](/Examples/Fundamentals/Drawing3D)**
  - [Drawing a Cube](/Examples/Fundamentals/Drawing3D/DrawingCube)
  - [Basic Camera Control](/Examples/Fundamentals/Drawing3D/BasicCameraControl)
  - [Face Culling](/Examples/Fundamentals/Drawing3D/FaceCulling)
  - [Instanced Rendering](/Examples/Fundamentals/Drawing3D/InstancedRendering)
  - [Depth Testing Operations](/Examples/Fundamentals/Drawing3D/DepthTestingOperations)
- **[Pipelines And Passes](/Examples/Fundamentals/PipelinesAndPasses)**
  - [Changing Blending Factor with Dynamic State](/Examples/Fundamentals/PipelinesAndPasses/DynamicStatePipelines)
  - [Using Multiple Pipelines and Pipeline Derivatives](/Examples/Fundamentals/PipelinesAndPasses/MultiplePipelines)
  - [Object Outlining with Stencil Testing](/Examples/Fundamentals/PipelinesAndPasses/BasicStencilTesting)
  - [Using Multiple Subpasses and Input Attachments](/Examples/Fundamentals/PipelinesAndPasses/MultipleSubpasses)
  - [Using Multiple Render Passes](/Examples/Fundamentals/PipelinesAndPasses/MultipleRenderPasses)
  - [Rendering Scene to a Texture](/Examples/Fundamentals/PipelinesAndPasses/OffscreenRendering)
- **[Swap Chains and Viewports](/Examples/Fundamentals/SwapChainsAndViewports)**
  - [Handling Window Resizing](/Examples/Fundamentals/SwapChainsAndViewports/SwapChainRecreation)
  - [Using Multi-Viewports for Different Colors](/Examples/Fundamentals/SwapChainsAndViewports/MultiViewport)
  - [Split Screen with Dynamic Viewport](/Examples/Fundamentals/SwapChainsAndViewports/DynamicViewport)
  - [Scissor Animation](/Examples/Fundamentals/SwapChainsAndViewports/ScissorAnimation)
- **[Model Loading](/Examples/Fundamentals/ModelLoading)**
  - [Rendering glTF Mesh with Wireframe](/Examples/Fundamentals/ModelLoading/GltfMeshWireframe)
  - [Rendering Textured glTF Mesh](/Examples/Fundamentals/ModelLoading/GltfMeshTextured)
  - [Multiple glTF Meshes and Node Transformations](/Examples/Fundamentals/ModelLoading/GltfMultipleMeshes)
- **[Multisampling](/Examples/Fundamentals/Multisampling)**
  - [MSAA Basics](/Examples/Fundamentals/Multisampling/MsaaBasics)
  - [Sample Shading](/Examples/Fundamentals/Multisampling/SampleShading)
  - [Explicit Resolving](/Examples/Fundamentals/Multisampling/ExplicitResolving)
- **[Compute Shaders](/Examples/Fundamentals/ComputeShaders)**
  - [Fullscreen Moving Gradient Generator](/Examples/Fundamentals/ComputeShaders/FullscreenGradient)
  - [Checkerboard Texture Generator](/Examples/Fundamentals/ComputeShaders/CheckerboardGenerator)
  - [Mandelbrot Fractal Rendering](/Examples/Fundamentals/ComputeShaders/MandelbrotFractal)
  - [Basic GPU-Generated Particle Effect](/Examples/Fundamentals/ComputeShaders/BasicParticles)
- **[Queries and Performance](/Examples/Fundamentals/QueriesAndPerformance)**
  - [Visibility Check with Occlusion Queries](/Examples/Fundamentals/QueriesAndPerformance/OcclusionQuery)
  - [Measuring The GPU Time with Timestamp Queries](/Examples/Fundamentals/QueriesAndPerformance/TimestampQuery)
  - [Measuring Pipeline Statistics](/Examples/Fundamentals/QueriesAndPerformance/PipelineStatisticsQuery)
  - [Using Dynamic Uniform Buffers](/Examples/Fundamentals/QueriesAndPerformance/DynamicUniformBuffer)
  - [Buffer Suballocation](/Examples/Fundamentals/QueriesAndPerformance/BufferSuballocation)
  - [Reduce CPU Bottleneck with Indirect Drawing](/Examples/Fundamentals/QueriesAndPerformance/IndirectDrawing)
  - [Specialization Constants](/Examples/Fundamentals/QueriesAndPerformance/SpecializationConstants)
  - [Pipeline Caches](/Examples/Fundamentals/QueriesAndPerformance/PipelineCaches)
  - [Descriptor Indexing](/Examples/Fundamentals/QueriesAndPerformance/DescriptorIndexing)

### [Real-Time Lighting](/Examples/RealTimeLighting)

- **[Basic Lighting](/Examples/RealTimeLighting/BasicLighting)**
  - [Diffuse Lighting in Flat Shading](/Examples/RealTimeLighting/BasicLighting/DiffuseLightingFlat)
  - [Diffuse Lighting in Gouraud Shading](/Examples/RealTimeLighting/BasicLighting/DiffuseLightingGouraud)
  - [Diffuse Lighting in Phong Shading](/Examples/RealTimeLighting/BasicLighting/DiffuseLightingPhong)
  - [Specular Lighting in Phong Shading](/Examples/RealTimeLighting/BasicLighting/SpecularLightingPhong)
  - [Blinn-Phong Shading](/Examples/RealTimeLighting/BasicLighting/BlinnPhong)
- **[Light Sources](/Examples/RealTimeLighting/LightSources)**
  - [Directional Light](/Examples/RealTimeLighting/LightSources/DirectionalLight)
  - [Point Light](/Examples/RealTimeLighting/LightSources/PointLight)
  - [Spotlight](/Examples/RealTimeLighting/LightSources/Spotlight)
  - [Flashlight with Soft-Cutoff](/Examples/RealTimeLighting/LightSources/Flashlight)
  - [Using Multiple Light Sources](/Examples/RealTimeLighting/LightSources/MultipleLightSources)
- **[Textured Materials](/Examples/RealTimeLighting/TexturedMaterials)**
  - [Diffuse Mapping](/Examples/RealTimeLighting/TexturedMaterials/DiffuseMapping)
  - [Specular Mapping](/Examples/RealTimeLighting/TexturedMaterials/SpecularMapping)
  - [Emissive Materials](/Examples/RealTimeLighting/TexturedMaterials/EmissiveMaterials)
  - [Shininess & Inverse Roughness Mapping](/Examples/RealTimeLighting/TexturedMaterials/ShininessMapping)
  - [Ambient Occlusion Mapping](/Examples/RealTimeLighting/TexturedMaterials/BasicAmbientOcclusion)
- **[Texture Sampling and Filtering](/Examples/RealTimeLighting/TextureSamplingAndFiltering)**
  - [Basic Mipmapping and Trilinear Filtering](/Examples/RealTimeLighting/TextureSamplingAndFiltering/BasicMipmapping)
  - [Manual Mipmap LOD Control](/Examples/RealTimeLighting/TextureSamplingAndFiltering/ManualMipmapping)
  - [Anisotropic Filtering](/Examples/RealTimeLighting/TextureSamplingAndFiltering/AnisotropicFiltering)
  - [Gradient-Based LOD Control with Anisotropy](/Examples/RealTimeLighting/TextureSamplingAndFiltering/GradientBasedMipmapping)
- **[Surface Detailing](/Examples/RealTimeLighting/SurfaceDetailing)**
  - [Bump Mapping](/Examples/RealTimeLighting/SurfaceDetailing/BumpMapping)
  - [Normal Mapping](/Examples/RealTimeLighting/SurfaceDetailing/NormalMapping)
  - [Displacement Mapping without Tessellation](/Examples/RealTimeLighting/SurfaceDetailing/BasicDisplacement)
  - [Parallax Offset Mapping](/Examples/RealTimeLighting/SurfaceDetailing/ParallaxOffsetMapping)
  - [Steep Parallax Mapping](/Examples/RealTimeLighting/SurfaceDetailing/SteepParallaxMapping)
  - [Parallax Occlusion Mapping](/Examples/RealTimeLighting/SurfaceDetailing/ParallaxOcclusionMapping)
  - [Relief Parallax Mapping](/Examples/RealTimeLighting/SurfaceDetailing/ReliefParallaxMapping)
- **[Environment Mapping](/Examples/RealTimeLighting/EnvironmentMapping)**
  - [Simple Cubemap Skybox](/Examples/RealTimeLighting/EnvironmentMapping/CubemapSkybox)
  - [Cubemap Reflections](/Examples/RealTimeLighting/EnvironmentMapping/CubemapReflections)
  - [Fresnel Effect](/Examples/RealTimeLighting/EnvironmentMapping/FresnelEffect)
  - [Refraction with Cubemaps](/Examples/RealTimeLighting/EnvironmentMapping/CubemapRefractions)
  - [Dynamic Planar Reflections](/Examples/RealTimeLighting/EnvironmentMapping/PlanarReflections)
  - [Dynamic Cubemap Reflections](/Examples/RealTimeLighting/EnvironmentMapping/DynamicCubemapReflections)
- **[Lighting Architectures](/Examples/RealTimeLighting/LightingArchitectures)**
  - [Simple Deferred Shading](/Examples/RealTimeLighting/LightingArchitectures/DeferredShading)
  - [Tiled Forward Shading (Forward+)](/Examples/RealTimeLighting/LightingArchitectures/TiledForwardShading)
  - [Clustered Forward Shading](/Examples/RealTimeLighting/LightingArchitectures/ClusteredForwardShading)
  - [Clustered Forward Shading with Unlimited Lights per Cluster](/Examples/RealTimeLighting/LightingArchitectures/ClusteredForwardUnlimited)
  - [Tiled Deferred Shading](/Examples/RealTimeLighting/LightingArchitectures/TiledDeferredShading)
  - [Clustered Deferred Shading](/Examples/RealTimeLighting/LightingArchitectures/ClusteredDeferredShading)
- **[Transparency Techniques](/Examples/RealTimeLighting/TransparencyTechniques)**
  - [Order-Dependent Transparency with Alpha Blending](/Examples/RealTimeLighting/TransparencyTechniques/AlphaBlendingTransparency)
  - [Cutout Transparency with Alpha to Coverage](/Examples/RealTimeLighting/TransparencyTechniques/AlphaToCoverageTransparency)
  - [Using Alpha Blending in Hybrid Lighting Architectures](/Examples/RealTimeLighting/TransparencyTechniques/HybridAlphaBlending)
  - [Weighted Blended Order-Independent Transparency](/Examples/RealTimeLighting/TransparencyTechniques/WeightedBlendedTransparency)
  - [Order-Independent Transparency with Depth Peeling](/Examples/RealTimeLighting/TransparencyTechniques/DepthPeelingTransparency)
  - [Order-Independent Transparency with Per-Pixel Linked Lists](/Examples/RealTimeLighting/TransparencyTechniques/PixelLinkedListsTransparency)

### [Real-Time Shadows](/Examples/RealTimeShadows)

- **[Basic Shadow Mapping](/Examples/RealTimeShadows/BasicShadowMapping)**
  - [Directional Shadow Mapping](/Examples/RealTimeShadows/BasicShadowMapping/DirectionalShadowMapping)
  - [Spotlight Shadow Mapping](/Examples/RealTimeShadows/BasicShadowMapping/SpotlightShadowMapping)
  - [Omnidirectional Shadow Mapping with Point Lights](/Examples/RealTimeShadows/BasicShadowMapping/OmnidirectionalShadowMapping)
  - [Multiple Shadow Mapping](/Examples/RealTimeShadows/BasicShadowMapping/MultipleShadowMapping)
- **[Shadow Filtering And Softening](/Examples/RealTimeShadows/ShadowFilteringAndSoftening)**
  - [Percentage Closer Filtering (PCF)](/Examples/RealTimeShadows/ShadowFilteringAndSoftening/PercentageCloserFiltering)
  - [Hardware-Accelerated Percentage Closer Filtering](/Examples/RealTimeShadows/ShadowFilteringAndSoftening/HardwarePercentageCloserFiltering)
  - [Exponential Shadow Mapping (ESM)](/Examples/RealTimeShadows/ShadowFilteringAndSoftening/ExponentialShadowMapping)
  - [Variance Shadow Mapping (VSM)](/Examples/RealTimeShadows/ShadowFilteringAndSoftening/VarianceShadowMapping)
  - [Exponential Variance Shadow Mapping (EVSM)](/Examples/RealTimeShadows/ShadowFilteringAndSoftening/ExponentialVarianceShadow)
  - [Percentage Closer Soft Shadows (PCSS)](/Examples/RealTimeShadows/ShadowFilteringAndSoftening/PercentageCloserSoftShadows)
  - [Exponential Variance Shadow Mapping with Gaussian Blur](/Examples/RealTimeShadows/ShadowFilteringAndSoftening/ExponentialVarianceWithGaussian)
  - [Summed-Area Variance Shadow Mapping (SAVSM)](/Examples/RealTimeShadows/ShadowFilteringAndSoftening/SummedAreaVarianceShadow)
- **[Shadow Map Splitting and Warping](/Examples/RealTimeShadows/ShadowMapSplittingWarping)**
  - [Cascaded Shadow Maps (CSM)](/Examples/RealTimeShadows/ShadowMapSplittingWarping/CascadedShadowMaps)
- **[Shadow Resource Management](/Examples/RealTimeShadows/ShadowResourceManagement)**
  - [Shadow Map Atlas](/Examples/RealTimeShadows/ShadowResourceManagement/ShadowMapAtlas)
  - [Layered Shadow Maps with Texture Arrays](/Examples/RealTimeShadows/ShadowResourceManagement/LayeredShadowMaps)
  - [Mipmapped Variance Shadow Maps](/Examples/RealTimeShadows/ShadowResourceManagement/MipmappedShadowMaps)
  - [Anisotropic Filtering with Variance Shadow Maps](/Examples/RealTimeShadows/ShadowResourceManagement/AnisotropicShadowMaps)

### [Physically Based Rendering](/Examples/PhysicallyBasedRendering)

- **[Basic PBR](/Examples/PhysicallyBasedRendering/BasicPbr)**
  - [Principle of Conservation of Energy and Tone Mapping](/Examples/PhysicallyBasedRendering/BasicPbr/PbrEnergyConservation)
  - [Cook-Torrance Microfacet BRDF](/Examples/PhysicallyBasedRendering/BasicPbr/CookTorranceMicrofacet)
  - [Metallic Workflow in PBR](/Examples/PhysicallyBasedRendering/BasicPbr/PbrMetallicWorkflow)
- **[Textured PBR](/Examples/PhysicallyBasedRendering/TexturedPbr)**
  - [Roughness Map in PBR](/Examples/PhysicallyBasedRendering/TexturedPbr/RoughnessMapPbr)
  - [Metallic Map in PBR](/Examples/PhysicallyBasedRendering/TexturedPbr/MetallicMapPbr)
  - [Normal Map in PBR](/Examples/PhysicallyBasedRendering/TexturedPbr/NormalMapPbr)
  - [Ambient Occlusion Map in PBR](/Examples/PhysicallyBasedRendering/TexturedPbr/AmbientOcclusionPbr)
  - [Emissive Map in PBR](/Examples/PhysicallyBasedRendering/TexturedPbr/EmissiveMapPbr)
- **[Area Lights](/Examples/PhysicallyBasedRendering/AreaLights)**
  - [Rectangular Area Lights with Linearly Transformed Cosines (LTC)](/Examples/PhysicallyBasedRendering/AreaLights/RectangularAreaLights)
  - [Sphere Area Lights with Representative Point Method](/Examples/PhysicallyBasedRendering/AreaLights/SphereAreaLights)
  - [Using Multiple Area Lights](/Examples/PhysicallyBasedRendering/AreaLights/MultipleAreaLights)
- **[IBL and Reflections](/Examples/PhysicallyBasedRendering/IblAndReflections)**
  - [Using Equirectangular HDR Images as Skybox](/Examples/PhysicallyBasedRendering/IblAndReflections/EquirectangularHdrSkybox)
  - [Diffuse Irradiance Image Based Lighting](/Examples/PhysicallyBasedRendering/IblAndReflections/DiffuseIrradianceIbl)
  - [Specular Image Based Lighting](/Examples/PhysicallyBasedRendering/IblAndReflections/SpecularIbl)
  - [Complete Image Based Lighting on Textured Materials](/Examples/PhysicallyBasedRendering/IblAndReflections/CompleteIblTextured)
  - [Screen-Space Reflections (SSR)](/Examples/PhysicallyBasedRendering/IblAndReflections/ScreenSpaceReflections)
- **[Disney Principled BRDF](/Examples/PhysicallyBasedRendering/DisneyPrincipledBrdf)**
  - [Fundamental Diffuse and Specular Model of the Disney BRDF](/Examples/PhysicallyBasedRendering/DisneyPrincipledBrdf/DisneyDiffuseSpecular)
  - [Clear Coat Layer in Disney BRDF](/Examples/PhysicallyBasedRendering/DisneyPrincipledBrdf/DisneyBrdfClearCoat)
  - [Sheen Layer in Disney BRDF](/Examples/PhysicallyBasedRendering/DisneyPrincipledBrdf/DisneyBrdfSheen)
  - [Anisotropic Specular in Disney BRDF](/Examples/PhysicallyBasedRendering/DisneyPrincipledBrdf/DisneyBrdfAnisotropy)

### [Advanced Shader Programming](/Examples/AdvancedShaderProgramming)

- **[Geometry Shaders](/Examples/AdvancedShaderProgramming/GeometryShaders)**
  - [Simple Primitive Generation](/Examples/AdvancedShaderProgramming/GeometryShaders/SimplePrimitiveGeneration)
  - [Object Explosion via Geometry Shader](/Examples/AdvancedShaderProgramming/GeometryShaders/ObjectExplosion)
  - [Normal Vector Visualization](/Examples/AdvancedShaderProgramming/GeometryShaders/NormalVectorVisualization)
  - [Wireframe Overlay Visualization](/Examples/AdvancedShaderProgramming/GeometryShaders/WireframeOverlayVisualization)

## Environment Support

The written code is primarily tested in a Windows environment. After a few examples are completed, it is then tested in a GNU/Linux environment.

| Operating System | Support | Tested Compiler  |
|------------------|---------|------------------|
| Windows          | Yes     | MSVC 19.42.34435 |
| GNU/Linux        | Yes     | GCC 15.2.1       |
| MacOS            | No      | -                |


## Contributing

### Code of Conduct

Before start, please review our [CODE_OF_CONDUCT.md](/CODE_OF_CONDUCT.md) to learn community rules and agreements.

### How to Contribute?

You can contribute to this repo with suggestions, code corrections, or bug reports. You can suggest changes to the visual outputs of the examples. If you have an example you'd like to see in the repo, you can report it or open a Pull Request for your own implementations. Additionally, you can open any title you like in the Issues section. There's currently no specific format for topics, so feel free to open any topic you like.

## License

Distributed under the MIT License. See [LICENSE](/LICENSE) file for more information.