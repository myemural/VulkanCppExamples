# Cutout Transparency with Alpha to Coverage

**Code Name:** AlphaToCoverageTransparency

## Description

In this example, three planes containing metal chain textures are placed on the scene and order-independent rendering is performed. The Alpha-To-Coverage (A2C) technique is used to ensure that the completely transparent parts of this metal chain are discarded correctly.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/TransparencyTechniques/AlphaToCoverageTransparency.png?raw=true)

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

## Learning Objectives

- Implementing order-independent cutout transparency with Alpha-To-Coverage (A2C) technique in Vulkan
- Using multisampling for cutout transparency

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
