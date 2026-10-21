# Histogram-based Auto Exposure

**Code Name:** HistogramBasedAutoExposure

## Description

In this example, a histogram-based auto exposure has been applied by building a luminance histogram of the HDR scene and deriving the exposure value from its percentile-filtered average before the tone mapping pass.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/ExposureAndAdaptation/HistogramBasedAutoExposure.png?raw=true)

## Controls

| Input   | Action                         |
|---------|--------------------------------|
| W/A/S/D | Move the camera                |
| Mouse   | Look around with the camera    |
| Esc     | Close the window               |
| 1       | Reinhard tone mapping          |
| 2       | ACES tone mapping              |
| 3       | Uchimura tone mapping          |
| 4       | AgX tone mapping               |
| Z       | Decrease low percentile value  |
| X       | Increase low percentile value  |
| C       | Decrease high percentile value |
| V       | Increase high percentile value |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Histogram-based auto exposure calculation

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
