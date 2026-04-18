# Basic Mipmapping and Trilinear Filtering

**Code Name:** BasicMipmapping

## Description

In this example, basic mipmapping is applied to the diffuse texture of the objects drawn on the screen. Here, the mipmap images are manually generated, while the LOD control is automatically handled in the shader.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/TextureSamplingAndFiltering/BasicMipmapping.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |
| AppSettings.MinMipmapLevel   | float             | AppSettings::MinMipmapLevel   | Minimum mipmap level             |               |


## Learning Objectives

- Creating images and image views with mipmapping support in Vulkan
- Generating mipmaps for images in GPU memory (with `vkCmdBlitImage`)
- Understanding automatic mipmap level selection mechanism when sampling images in shaders

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
