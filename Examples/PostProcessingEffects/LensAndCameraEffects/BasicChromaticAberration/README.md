# Basic Chromatic Aberration Effect

**Code Name:** BasicChromaticAberration

## Description

In this example, a basic lateral chromatic aberration effect has been applied to the scene before tone mapping using a separate post-processing pass. The read and blue channels are shifted radially from the screen center with three texture samples.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/LensAndCameraEffects/BasicChromaticAberration.jpg?raw=true)

## Controls

| Input   | Action                                  |
|---------|-----------------------------------------|
| W/A/S/D | Move the camera                         |
| Mouse   | Look around with the camera             |
| Esc     | Close the window                        |
| V       | Turn on/off chromatic aberration effect |
| Q       | Increase exposure value                 |
| E       | Decrease exposure value                 |
| R       | Increase intensity value                |
| F       | Decrease intensity value                |
| T       | Increase falloff value                  |
| G       | Decrease falloff value                  |

## Application Parameters

### Settings

| Parameter / Key                | Type              | Usage in Code                   | Description                         | Default Value |
|--------------------------------|-------------------|---------------------------------|-------------------------------------|---------------|
| AppSettings.ClearColor         | VkClearColorValue | AppSettings::ClearColor         | Background color of the screen      |               |
| AppSettings.MouseSensitivity   | float             | AppSettings::MouseSensitivity   | Mouse sensitivity value             |               |
| AppSettings.CameraSpeed        | float             | AppSettings::CameraSpeed        | Speed of the camera                 |               |

## Learning Objectives

- Understanding how to implement chromatic aberration post-processing effect
- Adjusting intensity and falloff values of the chromatic aberration effect

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
