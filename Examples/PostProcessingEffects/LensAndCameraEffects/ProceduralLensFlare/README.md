# Procedural Lens Flare Effect

**Code Name:** ProceduralLensFlare

## Description

In this example, a procedural lens flare effect (glow, starburst, anamorphic streak, ghosts and halo) has been applied for every point light, which is rendered as an emissive sphere, using a separate post-processing pass before tone mapping.The visibility of each light is calculated from the G-buffer position for smooth occlusion.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/LensAndCameraEffects/ProceduralLensFlare.png?raw=true)

## Controls

| Input   | Action                                          |
|---------|-------------------------------------------------|
| W/A/S/D | Move the camera                                 |
| Mouse   | Look around with the camera                     |
| Esc     | Close the window                                |
| 1       | Turn on/off glow element of the lens flare      |
| 2       | Turn on/off starburst element of the lens flare |
| 3       | Turn on/off streak element of the lens flare    |
| 4       | Turn on/off ghosts element of the lens flare    |
| 5       | Turn on/off halo element of the lens flare      |
| V       | Turn on/off lens flare effect                   |
| Q       | Increase exposure value                         |
| E       | Decrease exposure value                         |
| R       | Increase lens flare intensity value             |
| F       | Decrease lens flare intensity value             |

## Application Parameters

### Settings

| Parameter / Key                | Type              | Usage in Code                   | Description                         | Default Value |
|--------------------------------|-------------------|---------------------------------|-------------------------------------|---------------|
| AppSettings.ClearColor         | VkClearColorValue | AppSettings::ClearColor         | Background color of the screen      |               |
| AppSettings.MouseSensitivity   | float             | AppSettings::MouseSensitivity   | Mouse sensitivity value             |               |
| AppSettings.CameraSpeed        | float             | AppSettings::CameraSpeed        | Speed of the camera                 |               |

## Learning Objectives

- Understanding how to implement procedural lens flare post-processing effect
- Enabling or disabling glow, starburst, streak, ghosts and halo components of the lens flare effect
- Adjusting intensity value of the lens flare effect

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
