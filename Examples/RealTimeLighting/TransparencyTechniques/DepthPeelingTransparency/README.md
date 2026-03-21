# Order-Independent Transparency with Depth Peeling

**Code Name:** DepthPeelingTransparency

## Description

This example demonstrates Order-Independent Transparency (OIT) using the Depth Peeling technique.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/TransparencyTechniques/DepthPeelingTransparency.png?raw=true)

## Controls

| Input   | Action                               |
|---------|--------------------------------------|
| W/A/S/D | Move the camera                      |
| Mouse   | Look around with the camera          |
| Esc     | Close the window                     |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |
| AppSettings.LightDirection   | glm:vec3          | AppSettings::LightDirection   | Direction vector of the light    |               |
| AppSettings.LightColor       | glm:vec3          | AppSettings::LightColor       | Color of the light (RGB)         |               |
| AppSettings.AmbientStrength  | float             | AppSettings::AmbientStrength  | Ambient strength of the material |               |
| AppSettings.SpecularStrength | float             | AppSettings::SpecularStrength | Specular strength factor         |               |
| AppSettings.Shininess        | float             | AppSettings::Shininess        | Shininess exponent               |               |
| AppSettings.PeelLayerCount   | std::uint32_t     | AppSettings::PeelLayerCount   | Number of layers to be peeled    |               |


## Learning Objectives

- Implementing order-independent transparency with using Depth Peeling technique in Vulkan

## Shader Status

| Shader Type | Status             | Notes                |
|-------------|--------------------|----------------------|
| GLSL        | :white_check_mark: |                      |
| HLSL        | :x:                | Will be implemented. |
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
