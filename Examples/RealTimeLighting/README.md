# Real-Time Lighting

This section contains Vulkan examples that related to real-time lighting. Examples aim to understand how we can implement basic lighting and shading methods, light casters (sources), materials, texture sampling, surface detailing, environment mapping, advanced lighting architectures and advanced transparency in Vulkan. The examples start from simple topics and gradually progress to more advanced topics.

## [Basic Lighting](/Examples/RealTimeLighting/BasicLighting)

- Implementing diffuse lighting in flat shading model
- Sending light data to the shaders
- Calculating Lambert diffuse reflection in the shaders
- Implementing diffuse lighting in gouraud shading model
- Doing light calculations in vertex shaders
- Implementing ambient lighting in phong shading model
- Implementing diffuse lighting in phong shading model
- Implementing specular lighting in phong shading model
- Implementing ambient, diffuse and specular lighting in blinn-phong shading model

## [Light Sources](/Examples/RealTimeLighting/LightSources)

- Implementing directional light behavior in the shaders
- Integrating blinn-phong shading model into the directional lights
- Implementing point light behavior in the shaders
- Integrating blinn-phong shading model into the point lights
- Implementing attenuation calculation to the shaders for distance-based lighting
- Implementing spotlight behavior in the shaders
- Integrating blinn-phong shading model into the spotlights
- Implementing hard-cutoff (hard edges) spotlight behavior with only one cutoff angle
- Implementing multiple lights behavior with the same type
- Implementing camera-based spotlight (flashlight) behavior
- Implementing soft-cutoff (soft edges) spotlight behavior with two cutoff angles
- Implementing various and numerous light sources behavior in the shaders
- Sending multiple light data and count via storage buffers and specialization constants

## [Textured Materials](/Examples/RealTimeLighting/TexturedMaterials)

TBD