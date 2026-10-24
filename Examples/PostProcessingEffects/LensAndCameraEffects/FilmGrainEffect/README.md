# Film Grain Effect

**Code Name:** FilmGrainEffect

## Description

In this example, an animated film grain effect has been applied on top of the tone mapped scene using a separate post-processing pass. The grain noise is generated procedurally in the shader from the time value (without any noise texture).

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/LensAndCameraEffects/FilmGrainEffect.jpg?raw=true)

## Controls

| Input   | Action                                  |
|---------|-----------------------------------------|
| W/A/S/D | Move the camera                         |
| Mouse   | Look around with the camera             |
| Esc     | Close the window                        |
| C       | Turn on/off colored grain effect        |
| V       | Turn on/off film grain effect           |
| Q       | Increase exposure value                 |
| E       | Decrease exposure value                 |
| R       | Increase grain intensity value          |
| F       | Decrease grain intensity value          |
| T       | Increase grain size value               |
| G       | Decrease grain size value               |
| Y       | Increase grain luminance response value |
| H       | Decrease grain luminance response value |

## Application Parameters

### Settings

| Parameter / Key                | Type              | Usage in Code                   | Description                         | Default Value |
|--------------------------------|-------------------|---------------------------------|-------------------------------------|---------------|
| AppSettings.ClearColor         | VkClearColorValue | AppSettings::ClearColor         | Background color of the screen      |               |
| AppSettings.MouseSensitivity   | float             | AppSettings::MouseSensitivity   | Mouse sensitivity value             |               |
| AppSettings.CameraSpeed        | float             | AppSettings::CameraSpeed        | Speed of the camera                 |               |
| AppSettings.FilmGrainFrameRate | float             | AppSettings::FilmGrainFrameRate | Frame rate of the film grain effect |               |

## Learning Objectives

- Understanding how to implement film grain post-processing effect
- Adjusting intensity, size and luminance response values of the film grain effect
- Turning on/off colored film grain effect

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
