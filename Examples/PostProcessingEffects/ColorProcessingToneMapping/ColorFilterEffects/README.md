# Color Filter Effects (Grayscale, Invert Colors, Sepia, Duotone, Channel Swap)

**Code Name:** ColorFilterEffects

## Description

This example demonstrates several color filter effects that can be applied as a post-processing step to the rendered image. It includes grayscale, inverted colors, sepia, duotone, and RGB channel swapping effects.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/ColorProcessingToneMapping/ColorFilterEffects.png?raw=true)

## Controls

| Input   | Action                             |
|---------|------------------------------------|
| W/A/S/D | Move the camera                    |
| Mouse   | Look around with the camera        |
| Esc     | Close the window                   |
| 0       | Color filter mode off              |
| 1       | Color filter mode is Grayscale     |
| 2       | Color filter mode is Invert Colors |
| 3       | Color filter mode is Sepia         |
| 4       | Color filter mode is Duotone       |
| 5       | Color filter mode is Channel Swap  |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Understanding how to implement grayscale post-processing color filter effect
- Understanding how to implement invert color post-processing color filter effect
- Understanding how to implement sepia post-processing color filter effect
- Understanding how to implement duotone post-processing color filter effect
- Understanding how to implement channel swap post-processing color filter effect

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
