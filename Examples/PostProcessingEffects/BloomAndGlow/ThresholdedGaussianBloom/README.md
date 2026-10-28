# Thresholded Bloom with Separable Gaussian Blur

**Code Name:** ThresholdedGaussianBloom

## Description

In this example, a classic thresholded bloom effect has been applied to the HDR scene before tone mapping. The bright parts of the exposed scene color are extracted into a half-res image with a soft-knee threshold, blurred with a separable gaussian filter in ping-pong passes, and added back onto the scene in the composite pass.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/BloomAndGlow/ThresholdedGaussianBloom.png?raw=true)

## Controls

| Input   | Action                                     |
|---------|--------------------------------------------|
| W/A/S/D | Move the camera                            |
| Mouse   | Look around with the camera                |
| Esc     | Close the window                           |
| Space   | Turn on/off bloom effect                   |
| 1       | Bright-pass source is luminance threshold  |
| 2       | Bright-pass source is emissive mask (glow) |
| B       | Switch debug view (final / bloom only)     |
| Q       | Increase exposure value                    |
| E       | Decrease exposure value                    |
| R       | Increase bloom threshold value             |
| F       | Decrease bloom threshold value             |
| T       | Increase soft knee value                   |
| G       | Decrease soft knee value                   |
| Y       | Increase bloom intensity value             |
| H       | Decrease bloom intensity value             |
| U       | Increase blur iteration count              |
| J       | Decrease blur iteration count              |
| Z       | Increase gaussian blur sigma value         |
| X       | Decrease gaussian blur sigma value         |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Understanding how to implement a thresholded bloom post-processing effect on HDR screen color
- Extracting bright areas with a soft-l-knee threshold into a half-resolution image
- Implementing a separable gaussian blur with linear sampling and ping-pong render targets

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
