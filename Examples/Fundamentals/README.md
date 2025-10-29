# Fundamentals

This section contains examples of basic Vulkan features. These examples aim to understand the use of Vulkan objects (device, pipeline, render pass, framebuffer, etc.) and to simplify understanding of the real-time rendering examples we'll do later. The examples start from simple topics and gradually progress to more advanced topics.

## [Basics](/Examples/Fundamentals/Basics)

The topics covered by this subsection:
- Creating a window
- Get information of the Vulkan devices (GPUs) on your system
- Creating a default surface from window system
- Creating logical devices and queues from physical devices
- Creating and managing a swapchain
- Creating command buffers from command pools and recording command buffers
- Creating synchronization objects (fence, semaphore) and basic synchronization
- Creating a render pass, simple pipeline and framebuffers
- Creating shader modules and loading external shader codes into these modules
- Creating and managing vertex and index buffers
- Drawing with only vertex information
- Drawing with vertices and indices (indexed drawing)
- Understanding primitive restart index
- Understanding and using staging buffers
- Drawing with a different topology (triangle strip)
- Using different polygon modes (lines, points etc.)

## [Descriptor Sets](/Examples/Fundamentals/DescriptorSets)

The topics covered by this subsection:
- Creating and managing descriptor pools
- Creating and managing descriptor sets and descriptor set layouts
- Updating descriptor sets
- Binding descriptor sets to the shaders
- Creating and updating uniform buffers
- Using multiple uniform buffers
- Basic 2D transformation (translation, rotation, scaling) with uniform buffers
- Using push constants in shaders and updating push constants
- Using array of uniform buffers

## [Images and Samplers](/Examples/Fundamentals/ImagesAndSamplers)

The topics covered by this subsection:
- Loading a texture from disk into an image buffer using staging buffer
- Creating and using samplers
- Creating descriptor sets for samplers and sampled images
- Applying textures into the meshes using shaders
- Understanding the usage of UV coordinates
- Creating descriptor sets for combined image samplers and using them in shaders
- Understanding of different wrap and filtering modes of samplers
- Using multiple samplers
- Using multiple textures and blending two textures in shaders
- Using transparent textures and understanding simple blending
- Using texture atlases for multiple texture and managing texture coordinates

## [Drawing 3D](/Examples/Fundamentals/Drawing3D)

The topics covered by this subsection:
- Understanding and using the MVP (Model-View-Projection) matrix to drawing in 3D
- Updating the MVP matrix in shaders via descriptor sets and push constants
- Enabling depth testing
- Creating a basic virtual camera with perspective projection
- Understanding and implementing a basic input system for keyboard and mouse
- Navigating a camera in 3D scene via keyboard and mouse input
- Understanding and implementing the delta time logic
- Understanding the face culling mechanism and using different kind of cull modes
- Using instanced drawing
- Managing the depth testing compare operations

## [Pipelines and Passes](/Examples/Fundamentals/PipelinesAndPasses)

The topics covered by this subsection:
- Understanding the load/store operations of the attachments in a render pass
- Enabling dynamic states in a pipeline and setting blending constant states dynamically
- Creating and using the multiple pipelines
- Understanding pipeline derivatives and creating a derived pipelines
- Understanding the basics of the stencil testing and enabling stencil testing
- Implementing a simple outlining with stencil testing operations
- Creating multiple subpass for a renderpass and using input attachments in shaders
- Rendering scene to a texture (offscreen rendering)

## [Swap Chains and Viewports](/Examples/Fundamentals/SwapChainsAndViewports)

The topics covered by this subsection:
- Handling window resizing with recreating swap chains
- Using multiple viewports and scissors (via extension)
- Clear the one part of the screen dynamically
- Using dynamic pipeline states related to viewports and scissors
- Creating simple screen animation with setting scissor position dynamically

## [Model Loading](/Examples/Fundamentals/ModelLoading)

The topics covered by this subsection:
- Loading binary and ASCII formatted glTF models from disk to buffers
- Loading single and multiple meshes and rendering them as single colored wireframe
- Loading mesh textures and rendering textured meshes
- Applying node transformation in the glTF
- Adding camera that defined in the glTF to our scene

## [Multisampling](/Examples/Fundamentals/Multisampling)

TBD