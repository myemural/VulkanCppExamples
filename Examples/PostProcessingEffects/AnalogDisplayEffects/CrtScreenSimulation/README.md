# CRT Screen Simulation (Curvature, Halation, Interlacing)

**Code Name:** CrtScreenSimulation

## Description

In this example, a complete CRT screen has been simulated. The tone mapped scene resampled into a persistent low-res virtual image. In interlaced mode (480i), every frame updates only the odd or the even lines, the lines of the other field fade out like decaying phosphors, which gives the typical line flicker and combing on moving objects. Finally, a blurred copy of the virtual image is added as halation.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/AnalogDisplayEffects/CrtScreenSimulation.png?raw=true)

## Controls

| Input   | Action                                     |
|---------|--------------------------------------------|
| W/A/S/D | Move the camera                            |
| Mouse   | Look around with the camera                |
| Esc     | Close the window                           |
| Space   | Turn on/off CRT effect                     |
| I       | Switch video mode (240p / 480i interlaced) |
| Q       | Increase exposure value                    |
| E       | Decrease exposure value                    |
| R       | Increase screen curvature                  |
| F       | Decrease screen curvature                  |
| T       | Increase halation strength                 |
| G       | Decrease halation strength                 |

## Application Parameters

### Settings

| Parameter / Key                | Type              | Usage in Code                   | Description                         | Default Value |
|--------------------------------|-------------------|---------------------------------|-------------------------------------|---------------|
| AppSettings.ClearColor         | VkClearColorValue | AppSettings::ClearColor         | Background color of the screen      |               |
| AppSettings.MouseSensitivity   | float             | AppSettings::MouseSensitivity   | Mouse sensitivity value             |               |
| AppSettings.CameraSpeed        | float             | AppSettings::CameraSpeed        | Speed of the camera                 |               |
| AppSettings.FilmGrainFrameRate | float             | AppSettings::FilmGrainFrameRate | Frame rate of the film grain effect |               |

## Learning Objectives

- Understanding how to simulate the curved tube of a CRT with rounded corners
- Implementing halation with a separate gaussian blur of the low-res virtual image
- Implementing interlacing with a persistent render target, blending and phosphor persistence

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
