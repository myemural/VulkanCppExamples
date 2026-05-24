# Normal Map in PBR

**Code Name:** NormalMapPbr

## Description

This example demonstrates the use of normal map textures in a PBR metallic/roughness workflow.

## Output

![](/Docs/ExampleMedia/PhysicallyBasedRendering/TexturedPbr/NormalMapPbr.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                    | Default Value |
|------------------------------|-------------------|-------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value        |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera            |               |
| AppSettings.LightDirection   | glm:vec3          | AppSettings::LightDirection   | Direction of the light         |               |
| AppSettings.LightColor       | glm:vec3          | AppSettings::LightColor       | Color of the light             |               |

## Learning Objectives

- Sampling the normals of the mesh geometry from a texture in PBR material system

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
