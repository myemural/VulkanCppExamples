# Exponential Variance Shadow Mapping with Gaussian Blur

**Code Name:** ExponentialVarianceWithGaussian

## Description

In this example, the shadows in the scene are created using Exponential Variance Shadow Mapping (EVSM) with Gaussian blur filtering in separate pass.

## Output

![](/Docs/ExampleMedia/RealTimeShadows/ShadowFilteringAndSoftening/ExponentialVarianceWithGaussian.png?raw=true)

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
| AppSettings.EsmExponent      | float             | AppSettings::EsmExponent      | ESM coefficient                  |               |

## Learning Objectives

- Implementing Gaussian Blur shader in Vulkan
- Applying Gaussian Blur filtering to Exponential Variance Shadow Mapping (EVSM)

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
