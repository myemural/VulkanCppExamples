# HDR Lift Gamma Gain Color Grading

**Code Name:** HdrLiftGammaGain

## Description

This example demonstrates scene-referred lift/gamma/gain applied to the linear HDR scene color before tone mapping. Users can select some default color presets (Teal & Orange, Golden Hour, Cold Bleak, Sticky Green) via number keys, and they can also adjust offsets and exposure values.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/ColorProcessingToneMapping/HdrLiftGammaGain.png?raw=true)

## Controls

| Input   | Action                            |
|---------|-----------------------------------|
| W/A/S/D | Move the camera                   |
| Mouse   | Look around with the camera       |
| Esc     | Close the window                  |
| Space   | Reset all values to default       |
| 0       | Apply color preset: Neutral       |
| 1       | Apply color preset: Teal & Orange |
| 2       | Apply color preset: Golden Hour   |
| 3       | Apply color preset: Cold Bleak    |
| 4       | Apply color preset: Sticky Green  |
| Q       | Increase exposure value           |
| E       | Decrease exposure value           |
| R       | Increase lift offset value        |
| F       | Decrease lift offset value        |
| T       | Increase gamma offset value       |
| G       | Decrease gamma offset value       |
| Y       | Increase gain offset value        |
| H       | Decrease gain offset value        |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Understanding how to implement scene-referred lift/gamma/gain applied to the linear HDR scene color

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
