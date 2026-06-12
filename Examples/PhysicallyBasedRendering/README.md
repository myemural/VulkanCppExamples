# Physically Based Rendering

This section contains Vulkan examples that related to physically based rendering (PBR). The examples start from simple topics and gradually progress to more advanced topics.

## [Basic PBR](/Examples/PhysicallyBasedRendering/BasicPbr)

- Implementing Fresnel calculation and energy conversion principle in PBR
- Using Reinhard Tone Mapping to convert lighting calculations from HDR to LDR
- Implementing the entire Cook-Torrance BRDF formula
- Adjusting material using the roughness value
- Implementing full metallic/roughness workflow
- Adjusting material using the metallic value

## [Textured PBR](/Examples/PhysicallyBasedRendering/TexturedPbr)

- Sampling the albedo of PBR material from a texture
- Sampling the roughness of PBR material from a texture
- Sampling the metallic of PBR material from a texture
- Sampling the normals of the mesh geometry from a texture in PBR material system
- Sampling the ambient occlusion of PBR material from a texture (with a fake ambient light)
- Sampling the emissive of PBR material from a texture

## [Area Lights](/Examples/PhysicallyBasedRendering/AreaLights)

- Implementing rectangular area lights using Linearly Transformed Cosines (LTC) method in Vulkan
- Applying the logic of double-sided area lighting
- Implementing sphere area lights using Representative Point method in Vulkan
- Enabling the simultaneous use of multiple rectangular area lights, multiple sphere area lights, and a directional light in the scene

## [IBL and Reflections](/Examples/PhysicallyBasedRendering/IblAndReflections)

- Loading HDR images
- Converting an equirectangular HDR image to a cubemap
- Using equirectangular HDR images as a skybox
- Generating an irradiance map via convolution on a HDR skybox
- Using irradiance map to calculate diffuse part of the IBL
- Generating pre-calculated BRDF Lookup Table (LUT) via compute shaders
- Generating prefiltered cubemap image via compute shaders
- Using prefiltered cubemap image to calculate specular part of the IBL
- Integrate emissive and ambient occlusion maps with the IBL calculations
- Applying complete IBL calculations to a textured glTF 3D model
- Implementing PBR material system with deferred rendering technique
- Implementing Screen-Space Reflection (SSR) with ray marching

## [Disney Principled BRDF](/Examples/PhysicallyBasedRendering/DisneyPrincipledBrdf)

- Implementing Disney BRDF's roughness and metallic parameters with IBL in Vulkan
- Implementing Disney BRDF's subsurface parameter with IBL in Vulkan
- Implementing Disney BRDF's specular and specular tint parameters with IBL in Vulkan
- Adding clear coat layer to the PBR material system
- Implementing Disney BRDF's clear coat and clear coat glossiness parameters with IBL in Vulkan
- Adding sheen layer to the PBR material system
- Implementing Disney BRDF's sheen and sheen tint parameters with IBL in Vulkan
- Implementing Disney BRDF's anisotropic parameter with IBL in Vulkan