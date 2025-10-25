# Vulkan C++ Examples

This repo aims to include implementations of many Vulkan examples that built with C++20 standard, from simple to advanced. First examples based on the basic features of Vulkan, then real-time rendering topics such as light, shadow, PBR and post-processing will be added.

- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Clone](#clone)
  - [Build](#build)
- [General Info](#general-info)
  - [Common Parameters](#common-parameters)
- [Environment Support](#environment-support)
- [Contributing](#contributing)
  - [Code of Conduct](#code-of-conduct)
  - [How to Contribute?](#how-to-contribute)
- [License](#license)

## Getting Started

### Prerequisites

Before start, you should download and install the latest LunarG Vulkan SDK from here (*1.4.328.1* version has been tested): https://vulkan.lunarg.com/sdk/home

To compile and test the examples, the *MSVC 19.40+* compiler and *CMake 3.31+* are recommended. All third party libraries are added with using [Git Submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules). Therefore, you do not need to worry about installing them separately. Here is a list of 3rd party libraries (repos) that are dependent on:
- GLFW
- glm (OpenGL Mathematics)
- tinygltf

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

### Build

Every example has its own directory and CMake target. You can build what you want with CMake command line tools or IDE tools.

## General Info

### Common Parameters

**Window Parameters**

| Parameter / Key    | Type          | Usage in Code             | Description                             | Default Value |
|--------------------|---------------|---------------------------|-----------------------------------------|---------------|
| Window.Width       | std::uint32_t | WindowParams::Width       | Initial width of the window (in pixel)  | 800           |
| Window.Height      | std::uint32_t | WindowParams::Height      | Initial height of the window (in pixel) | 600           |
| Window.Title       | std::string   | WindowParams::Title       | Title of the window                     |               |
| Window.Resizable   | bool          | WindowParams::Resizable   | Specifies window is resizable or not    | false         |
| Window.SampleCount | unsigned int  | WindowParams::SampleCount | Sample count of the window              | 1             |

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

## Examples

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
  - [Simple Ghosting Effect](/Examples/Fundamentals/PipelinesAndPasses/LoadStoreOps)
  - [Changing Blending Factor with Dynamic State](/Examples/Fundamentals/PipelinesAndPasses/DynamicStatePipelines)
  - [Using Multiple Pipelines and Pipeline Derivatives](/Examples/Fundamentals/PipelinesAndPasses/MultiplePipelines)
  - [Object Outlining with Stencil Testing](/Examples/Fundamentals/PipelinesAndPasses/BasicStencilTesting)
  - [Using Multiple Subpasses and Input Attachments](/Examples/Fundamentals/PipelinesAndPasses/MultipleSubpasses)
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
  - [Camera Usage with glTF](/Examples/Fundamentals/ModelLoading/GltfCamera)

## Environment Support

Currently only tested on Windows. Other operating systems will be added.

| Operating System | Support | Minimum Tested Version |
|------------------|---------|------------------------|
| Windows          | Yes     | Windows 11 Pro 24H2    |
| GNU/Linux        | No      | -                      |
| MacOS            | No      | -                      |


## Contributing

### Code of Conduct

Before start, please review our [CODE_OF_CONDUCT.md](/CODE_OF_CONDUCT.md) to learn community rules and agreements.

### How to Contribute?

You can contribute to this repo with suggestions, code corrections, or bug reports. You can suggest changes to the visual outputs of the examples. If you have an example you'd like to see in the repo, you can report it or open a Pull Request for your own implementations. Additionally, you can open any title you like in the Issues section. There's currently no specific format for topics, so feel free to open any topic you like.

## License

Distributed under the MIT License. See [LICENSE](/LICENSE) file for more information.