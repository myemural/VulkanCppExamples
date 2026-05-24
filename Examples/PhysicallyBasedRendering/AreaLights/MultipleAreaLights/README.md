# Using Multiple Area Lights

**Code Name:** MultipleAreaLights

## Description

This example demonstrates how to use multiple types and numbers of area lights and directional lights in a scene containing PBR materials.

## Output

![](/Docs/ExampleMedia/PhysicallyBasedRendering/AreaLights/MultipleAreaLights.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key                       | Type              | Usage in Code                          | Description                         | Default Value |
|---------------------------------------|-------------------|----------------------------------------|-------------------------------------|---------------|
| AppSettings.ClearColor                | VkClearColorValue | AppSettings::ClearColor                | Background color of the screen      |               |
| AppSettings.MouseSensitivity          | float             | AppSettings::MouseSensitivity          | Mouse sensitivity value             |               |
| AppSettings.CameraSpeed               | float             | AppSettings::CameraSpeed               | Speed of the camera                 |               |
| AppSettings.DirectionalLightDirection | glm:vec3          | AppSettings::DirectionalLightDirection | Direction of the directional light  |               |
| AppSettings.DirectionalLightColor     | glm:vec3          | AppSettings::DirectionalLightColor     | Color of the directional light      |               |
| AppSettings.DirectionalLightIntensity | float             | AppSettings::DirectionalLightIntensity | Intensity of the direcrtional light |               |

## Learning Objectives

- Enabling the simultaneous use of multiple rectangular area lights, multiple sphere area lights, and a directional light in the scene

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
