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