# Anisotropic Filtering

**Code Name:** AnisotropicFiltering

## Description

In this example, the diffuse texture coordinates of the objects drawn on the screen are scaled, and anisotropic filtering is activated. Additionally, the maximum anisotropy level supported by the device is used.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/TextureSamplingAndFiltering/AnisotropicFiltering.png?raw=true)

## Controls

| Input           | Action                                                 |
|-----------------|--------------------------------------------------------|
| W/A/S/D         | Move the camera                                        |
| Mouse           | Look around with the camera                            |
| Esc             | Close the window                                       |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |
| AppSettings.MinMipmapLevel   | float             | AppSettings::MinMipmapLevel   | Minimum mipmap level             |               |


## Learning Objectives

- Enabling anisotropic filtering for samplers in Vulkan
- Setting max anisotropy level that supported your GPU

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
