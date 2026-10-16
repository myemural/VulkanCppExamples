# HDR Tone Mapping Operations with Manual Exposure (Reinhard, ACES, Uchimura, AgX, Parametric Filmic)

**Code Name:** HdrToneMapping

## Description

In this example, tone mapping operations (Reinhard, ACES, Uchimura, AgX, Parametric Filmic) have been applied using a separate post-processing pass to the final scene.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/ColorProcessingToneMapping/HdrToneMapping.png?raw=true)

## Controls

| Input   | Action                         |
|---------|--------------------------------|
| W/A/S/D | Move the camera                |
| Mouse   | Look around with the camera    |
| Esc     | Close the window               |
| Space   | Reset all values to default    |
| Q       | Increase exposure value        |
| E       | Decrease exposure value        |
| 0       | Turn off tone mapping          |
| 1       | Reinhard tone mapping          |
| 2       | ACES tone mapping              |
| 3       | Uchimura tone mapping          |
| 4       | AgX tone mapping               |
| 5       | Parametric filmic tone mapping |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Understanding how to implement Reinhard tone mapping algorithm
- Understanding how to implement ACES tone mapping algorithm
- Understanding how to implement Uchimura tone mapping algorithm
- Understanding how to implement AgX tone mapping algorithm
- Understanding how to implement Parametric Filmic tone mapping algorithm

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
