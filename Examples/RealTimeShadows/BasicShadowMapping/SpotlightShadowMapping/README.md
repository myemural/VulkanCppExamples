# Spotlight Shadow Mapping

**Code Name:** SpotlightShadowMapping

## Description

In this example, perspective projection is used on the camera for lighting, applying shadow mapping to the spotlight's light in the scene.

## Output

![](/Docs/ExampleMedia/RealTimeShadows/BasicShadowMapping/SpotlightShadowMapping.png?raw=true)

## Controls

| Input   | Action                               |
|---------|--------------------------------------|
| W/A/S/D | Move the camera                      |
| Mouse   | Look around with the camera          |
| Esc     | Close the window                     |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                         | Default Value |
|------------------------------|-------------------|-------------------------------|-------------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen      |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value             |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera                 |               |

## Learning Objectives

- Implementing spotlight shadow mapping technique for spotlight sources in Vulkan
- Using perspective camera for generating shadow maps

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
