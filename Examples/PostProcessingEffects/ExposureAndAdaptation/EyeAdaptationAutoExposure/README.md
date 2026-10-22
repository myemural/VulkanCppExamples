# Eye (Temporal) Adaptation on Auto Exposure

**Code Name:** EyeAdaptationAutoExposure

## Description

In this example, eye (temporal) adaptation has been applied on top of the histogram-based auto exposure by smoothly moving the adapted luminance towards the metered target luminance over time in log2 space.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/ExposureAndAdaptation/EyeAdaptationAutoExposure.png?raw=true)

## Controls

| Input   | Action                                |
|---------|---------------------------------------|
| W/A/S/D | Move the camera                       |
| Mouse   | Look around with the camera           |
| Esc     | Close the window                      |
| Space   | Reset eye adaptation                  |
| 1       | Reinhard tone mapping                 |
| 2       | ACES tone mapping                     |
| 3       | Uchimura tone mapping                 |
| 4       | AgX tone mapping                      |
| Z       | Decrease low percentile value         |
| X       | Increase low percentile value         |
| C       | Decrease high percentile value        |
| V       | Increase high percentile value        |
| L       | Turn on/off lights of the bright room |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Applying eye adaptation on histogram-based auto exposure
- Observing instantaneous light changes on eye adaptation technique

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
