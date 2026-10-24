# Subsurface Scattering in Disney BSDF

**Code Name:** DisneySubsurfaceScattering

## Description

In this example, a per-channel scatterDistance (RGB) parameter has been added to the Disney BSDF, and Burley’s normalized diffusion profile has been applied to the diffuse irradiance using screen-space importance sampling.

## Output

![](/Docs/ExampleMedia/PhysicallyBasedRendering/DisneyBsdfTransmission/DisneySubsurfaceScattering.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |
| 0       | IBL disabled                |
| 1       | IBL enabled                 |

## Application Parameters

### Settings

| Parameter / Key                  | Type              | Usage in Code                     | Description                                    | Default Value |
|----------------------------------|-------------------|-----------------------------------|------------------------------------------------|---------------|
| AppSettings.ClearColor           | VkClearColorValue | AppSettings::ClearColor           | Background color of the screen                 |               |
| AppSettings.MouseSensitivity     | float             | AppSettings::MouseSensitivity     | Mouse sensitivity value                        |               |
| AppSettings.CameraSpeed          | float             | AppSettings::CameraSpeed          | Speed of the camera                            |               |

## Learning Objectives

- Implementing subsurface scattering approach in Disney BSDF material model

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
