# CRT Scanlines and Phosphor Masks

**Code Name:** CrtScanlinesPhosphorMask

## Description

In this example, a CRT display has been simulated on top of the tone mapped scene usinf a separate post-processing pass. The scene is resampled into a low-res virtual image (240 lines), then every virtual line is drawn as a gaussian electron beam whose width depends on the brightness of the color, which gives the typical scanlines. Finally, a phosphor mask (aperture grille, slot mask or shadow mask) is applied in screen pixel space.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/AnalogDisplayEffects/CrtScanlinesPhosphorMask.png?raw=true)

## Controls

| Input   | Action                                  |
|---------|-----------------------------------------|
| W/A/S/D | Move the camera                         |
| Mouse   | Look around with the camera             |
| Esc     | Close the window                        |
| Space   | Turn on/off CRT effect                  |
| 0       | Turn off phosphor mask                  |
| 1       | Switch phosphor mask to aperture grille |
| 2       | Switch phosphor mask to slot mask       |
| 3       | Switch phosphor mask to shadow mask     |
| Q       | Increase exposure value                 |
| E       | Decrease exposure value                 |
| R       | Increase scanline strength              |
| F       | Decrease scanline strength              |

## Application Parameters

### Settings

| Parameter / Key                | Type              | Usage in Code                   | Description                         | Default Value |
|--------------------------------|-------------------|---------------------------------|-------------------------------------|---------------|
| AppSettings.ClearColor         | VkClearColorValue | AppSettings::ClearColor         | Background color of the screen      |               |
| AppSettings.MouseSensitivity   | float             | AppSettings::MouseSensitivity   | Mouse sensitivity value             |               |
| AppSettings.CameraSpeed        | float             | AppSettings::CameraSpeed        | Speed of the camera                 |               |
| AppSettings.FilmGrainFrameRate | float             | AppSettings::FilmGrainFrameRate | Frame rate of the film grain effect |               |

## Learning Objectives

- Understanding how to simulate a CRT display with a low-resolution virtual image
- Drawing scanlines with a brightness dependent gaussian beam profile without losing the average brightness
- Implementing aperture grille, slot mask and shadow mask phosphor patterns

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
