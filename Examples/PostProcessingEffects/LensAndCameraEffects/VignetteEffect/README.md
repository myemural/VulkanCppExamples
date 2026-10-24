# Vignette Effect

**Code Name:** VignetteEffect

## Description

In this example, a vignette effect has been applied to the exposed HDR scene before tone mapping phase using a separate post-processing pass. Some predefined presets (Classic Lens, Dramatic Spotlight, Horror, Low Health) can be selected by users.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/LensAndCameraEffects/VignetteEffect.png?raw=true)

## Controls

| Input   | Action                                  |
|---------|-----------------------------------------|
| W/A/S/D | Move the camera                         |
| Mouse   | Look around with the camera             |
| Esc     | Close the window                        |
| 1       | Apply Classic Lens vignette theme       |
| 2       | Apply Dramatic Spotlight vignette theme |
| 3       | Apply Horror vignette theme             |
| 4       | Apply Low Health vignette theme         |
| C       | Turn on/off rounded vignette            |
| V       | Turn on/off vignette effect             |
| Q       | Increase exposure value                 |
| E       | Decrease exposure value                 |
| R       | Increase vignette intensity value       |
| F       | Decrease vignette intensity value       |
| T       | Increase vignette smoothness value      |
| G       | Decrease vignette smoothness value      |
| Y       | Increase vignette roundness value       |
| H       | Decrease vignette roundness value       |

## Application Parameters

### Settings

| Parameter / Key                | Type              | Usage in Code                   | Description                         | Default Value |
|--------------------------------|-------------------|---------------------------------|-------------------------------------|---------------|
| AppSettings.ClearColor         | VkClearColorValue | AppSettings::ClearColor         | Background color of the screen      |               |
| AppSettings.MouseSensitivity   | float             | AppSettings::MouseSensitivity   | Mouse sensitivity value             |               |
| AppSettings.CameraSpeed        | float             | AppSettings::CameraSpeed        | Speed of the camera                 |               |

## Learning Objectives

- Understanding how to implement vignette post-processing effect
- Adjusting intensity, smoothness and roundness values of the vignette effect
- Applying different kind of vignette theme presets

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
