# Primary Color Corrections on LDR (Brightness, Contrast, Saturation, Hue Shift)

**Code Name:** PrimaryColorCorrections

## Description

In this example, real-time primary color adjustments (Brightness, Contrast, Saturation, Hue Shift) have been applied using separate post-processing pass to the final LDR scene color.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/ColorProcessingToneMapping/PrimaryColorCorrections.png?raw=true)

## Controls

| Input   | Action                                |
|---------|---------------------------------------|
| W/A/S/D | Move the camera                       |
| Mouse   | Look around with the camera           |
| Esc     | Close the window                      |
| Space   | Reset all values to default           |
| R       | Increase brightness value             |
| F       | Decrease brightness value             |
| T       | Increase contrast value               |
| G       | Decrease contrast value               |
| Y       | Increase saturation value             |
| H       | Decrease saturation value             |
| U       | Increase hue shift value              |
| J       | Decrease hue shift value              |
| Q       | Toggle display encoding sRGB / Linear |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Understanding how to implement brightness post-processing color correction effect
- Understanding how to implement contrast color post-processing color correction effect
- Understanding how to implement saturation post-processing color correction effect
- Understanding how to implement hue shift post-processing color correction effect

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
