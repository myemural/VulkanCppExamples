# HDR White Balance and Color Temperature Adjustment

**Code Name:** HdrTemperatureAdjustment

## Description

This example demonstrates scene-referred chromatic adaptation applied to the linear HDR scene color before tone mapping, with adjustable color temperature green/magenta tint. Two inverse modes are provided: White Balance and Color Temperature.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/ColorProcessingToneMapping/HdrTemperatureAdjustment.png?raw=true)

## Controls

| Input   | Action                         |
|---------|--------------------------------|
| W/A/S/D | Move the camera                |
| Mouse   | Look around with the camera    |
| Esc     | Close the window               |
| Space   | Reset all values to default    |
| 0       | Turn of temperature mode       |
| 1       | White balance setting mode     |
| 2       | Color temperature setting mode |
| Q       | Increase exposure value        |
| E       | Decrease exposure value        |
| R       | Increase temperature value     |
| F       | Decrease temperature value     |
| T       | Increase tint value            |
| G       | Decrease tint value            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Understanding how to implement white balance and color temperature post-processing effect on HDR scene color

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
