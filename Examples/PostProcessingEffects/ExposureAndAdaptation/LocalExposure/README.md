# Local Exposure with Bilateral Grid and Blurred Luminance

**Code Name:** LocalExposure

## Description

In this example, local exposure has been applied by compressing the contrast of a base layer (a blend of a bilateral grid and a blurred luminance) around middle gray while preserving the per-pixel detail against it.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/ExposureAndAdaptation/LocalExposure.png?raw=true)

## Controls

| Input   | Action                                 |
|---------|----------------------------------------|
| W/A/S/D | Move the camera                        |
| Mouse   | Look around with the camera            |
| Esc     | Close the window                       |
| Space   | Reset eye adaptation                   |
| 1       | Reinhard tone mapping                  |
| 2       | ACES tone mapping                      |
| 3       | Uchimura tone mapping                  |
| 4       | AgX tone mapping                       |
| Z       | Decrease low percentile value          |
| X       | Increase low percentile value          |
| C       | Decrease high percentile value         |
| V       | Increase high percentile value         |
| F       | Decrease local contrast scale value    |
| G       | Increase local contrast scale value    |
| H       | Decrease blurred luminance blend value |
| J       | Increase blurred luminance blend value |
| L       | Turn on/off lights of the bright room  |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Applying local exposure technique using a blend of a bilateral grid and a blurred luminance
- Integrating eye adaptation on local exposure

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
