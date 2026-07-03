# Real-Time Shadows

This section contains Vulkan examples that related to real-time shadows. Examples aim to understand how we can implement basic shadow mapping, shadow filtering and softening techniques, shadows in large scenes, screen-space shadows, shadow resource management techniques and transparency with shadows. The examples start from simple topics and gradually progress to more advanced topics.

## [Basic Shadow Mapping](/Examples/RealTimeShadows/BasicShadowMapping)

- Implementing directional shadow mapping technique in Vulkan
- Fixing shadow acne problem with shadow bias
- Handling regions outside the shadow map
- Implementing spotlight shadow mapping technique for spotlight sources in Vulkan
- Using perspective camera for generating shadow maps
- Implementing omnidirectional shadow mapping technique for point light sources in Vulkan
- Using cubemaps for generating shadow maps
- Implementing multiple shadow mapping techniques for one scene in Vulkan
- Blending different kind of shadows

## [Shadow Filtering And Softening](/Examples/RealTimeShadows/ShadowFilteringAndSoftening)

- Implementing Percentage Closer Filtering (PCF) technique in Vulkan
- Changing kernel size with user inputs
- Implementing Percentage Closer Filtering (PCF) technique with hardware supported samplers in Vulkan
- Implementing Exponential Shadow Mapping (ESM) technique in Vulkan
- Applying box filtering kernel to Exponential Shadow Mapping (ESM)
- Implementing Variance Shadow Mapping (VSM) technique in Vulkan
- Applying box filtering kernel to Variance Shadow Mapping (VSM)
- Implementing Exponential Variance Shadow Mapping (EVSM) technique in Vulkan
- Applying box filtering kernel to Exponential Variance Shadow Mapping (EVSM)
- Implementing Percentage Closer Soft Shadows (PCSS) technique in Vulkan
- Applying a constant-sized Poisson-disk filtering to PCSS
- Implementing Gaussian Blur shader in Vulkan
- Applying Gaussian Blur filtering to Exponential Variance Shadow Mapping (EVSM)
- Implementing Summed Area Variance Shadow Mapping (SAVSM) technique in Vulkan
- Generated Summed Area Table with compute shaders
- Applying box filtering kernel to Summed Area Variance Shadow Mapping (SAVSM)

## [Shadow Map Splitting And Warping](/Examples/RealTimeShadows/ShadowMapSplittingWarping)

- Implementing Cascaded Shadow Maps without any stabilization in Vulkan
- Applying debug visualization to show cascades on the screen

## [Shadow Resource Management](/Examples/RealTimeShadows/ShadowResourceManagement)

- Creating and using shadow map atlas in Vulkan
- Creating and using layered shadow maps in Vulkan via texture arrays
- Creating mipmapped variance shadow maps
- Apply trilinear filtering to the mipmapped shadow map to achieve soft shadows by distances to the camera
- Apply anisotropic filtering to the mipmapped shadow map to achieve better soft shadows by distances to the camera