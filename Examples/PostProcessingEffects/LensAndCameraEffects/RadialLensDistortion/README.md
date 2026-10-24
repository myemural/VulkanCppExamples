# Radial Lens Distortion Effects (Barrel, Pincushion, Fisheye)

**Code Name:** RadialLensDistortion

## Description

In this example, radial lens distortion effects (Barrel, Pincushion, Fisheye) have been applied to the scene before tone mapping using a separate post-processing pass, and they can be switched at runtime.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/LensAndCameraEffects/RadialLensDistortion.png?raw=true)

## Controls

| Input   | Action                                     |
|---------|--------------------------------------------|
| W/A/S/D | Move the camera                            |
| Mouse   | Look around with the camera                |
| Esc     | Close the window                           |
| 1       | Apply Barrel distortion effect             |
| 2       | Apply Pincushion distortion effect         |
| 3       | Apply Fisheye distortion effect            |
| C       | Turn on/off auto-fit feature (Barrel only) |
| V       | Turn on/off distortion effects             |
| Q       | Increase exposure value                    |
| E       | Decrease exposure value                    |
| R       | Increase distortion strength value         |
| F       | Decrease distortion strength value         |
| T       | Increase distortion zoom value             |
| G       | Decrease distortion zoom value             |

## Application Parameters

### Settings

| Parameter / Key                | Type              | Usage in Code                   | Description                         | Default Value |
|--------------------------------|-------------------|---------------------------------|-------------------------------------|---------------|
| AppSettings.ClearColor         | VkClearColorValue | AppSettings::ClearColor         | Background color of the screen      |               |
| AppSettings.MouseSensitivity   | float             | AppSettings::MouseSensitivity   | Mouse sensitivity value             |               |
| AppSettings.CameraSpeed        | float             | AppSettings::CameraSpeed        | Speed of the camera                 |               |

## Learning Objectives

- Understanding how to implement lens distortion (barrel, pincushion, fisheye) post-processing effects
- Adjusting strength and zoom values of the distortion effects
- Enabling or disabling auto-fit feature of the barrel distortion effect

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
