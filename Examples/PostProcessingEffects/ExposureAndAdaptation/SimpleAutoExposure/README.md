# Simple Auto Exposure

**Code Name:** SimpleAutoExposure

## Description

In this example, a simple auto exposure has been applied by computing the log-average luminance of the HDR scene in a compute pass and deriving the exposure value from it before the tone mapping pass.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/ExposureAndAdaptation/SimpleAutoExposure.png?raw=true)

## Controls

| Input   | Action                         |
|---------|--------------------------------|
| W/A/S/D | Move the camera                |
| Mouse   | Look around with the camera    |
| Esc     | Close the window               |
| Q       | Increase exposure value        |
| E       | Decrease exposure value        |
| 1       | Reinhard tone mapping          |
| 2       | ACES tone mapping              |
| 3       | Uchimura tone mapping          |
| 4       | AgX tone mapping               |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Auto exposure value calculation with log-average luminance of the HDR scene

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
