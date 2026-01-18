# Ambient Occlusion Mapping

**Code Name:** BasicAmbientOcclusion

## Description

In this example, diffuse texture and ambient occlusion texture are applied to objects in the scene, symbolically demonstrating the contribution of ambient occlusion to blinn-phong shading.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/TexturedMaterials/BasicAmbientOcclusion.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key               | Type              | Usage in Code                  | Description                      | Default Value |
|-------------------------------|-------------------|--------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor        | VkClearColorValue | AppSettings::ClearColor        | Background color of the screen   |               |
| AppSettings.MouseSensitivity  | float             | AppSettings::MouseSensitivity  | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed       | float             | AppSettings::CameraSpeed       | Speed of the camera              |               |
| AppSettings.LightDirection    | glm:vec3          | AppSettings::LightDirection    | Direction of the light           |               |
| AppSettings.LightColor        | glm:vec3          | AppSettings::LightColor        | Color of the light (RGB)         |               |
| AppSettings.LightAmbientColor | glm:vec3          | AppSettings::LightAmbientColor | Ambient color of the light (RGB) |               |
| AppSettings.AmbientStrength   | float             | AppSettings::AmbientStrength   | Ambient strength of the material |               |
| AppSettings.SpecularStrength  | float             | AppSettings::SpecularStrength  | Specular strength factor         |               |
| AppSettings.Shininess         | float             | AppSettings::Shininess         | Shininess exponent               |               |


## Learning Objectives

- Applying ambient occlusion texture to objects via shaders
- Symbolically including ambient occlusion texture to the lighting calculations (for blinn-phong)

## Shader Status

| Shader Type | Status             | Notes                |
|-------------|--------------------|----------------------|
| GLSL        | :white_check_mark: |                      |
| HLSL        | :white_check_mark: |                      |
| Slang       | :x:                | Will be implemented. |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
