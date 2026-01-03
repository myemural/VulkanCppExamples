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

- Applying diffuse texture to objects via shaders
- Including diffuse texture to the lighting calculations (for blinn-phong)
- Applying specular texture to objects via shaders
- Including specular texture to the lighting calculations (for blinn-phong)
- Applying emissive texture to objects via shaders
- Including emissive texture to the lighting calculations (for blinn-phong)
- Applying shininess or roughness texture to objects via shaders
- Including shininess or roughness texture to the lighting calculations (for blinn-phong)
- Applying alpha texture to objects via shaders for getting alpha values of the fragments
- Including alpha texture to the lighting calculations (for blinn-phong)
- Applying ambient occlusion texture to objects via shaders
- Symbolically including ambient occlusion texture to the lighting calculations (for blinn-phong)

## [Texture Sampling and Filtering](/Examples/RealTimeLighting/TextureSamplingAndFiltering)

- Creating images and image views with mipmapping support in Vulkan
- Generating mipmaps for images in GPU memory (with `vkCmdBlitImage`)
- Understanding automatic mipmap level selection mechanism when sampling images in shaders
- Understanding manual mipmap level selection mechanism when sampling images in shaders
- Changing current mipmap level via keyboard input
- Enabling anisotropic filtering for samplers in Vulkan
- Setting max anisotropy level that supported your GPU
- Implementing gradient-based manual mipmap level control in shaders
- Setting different color for objects that have different mipmap levels (for debugging)

## [Surface Detailing](/Examples/RealTimeLighting/SurfaceDetailing)

TBD