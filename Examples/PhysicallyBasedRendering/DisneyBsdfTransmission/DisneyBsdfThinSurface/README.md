# Thin Surface Approximation in Disney BSDF

**Code Name:** DisneyBsdfThinSurface

## Description

This example demonstrates thin surface approximation in Disney BSDF, where the entry and exit scattering events of a sheet without modeled volume are evaluated at a single shading point. Static and firefly-like orbiting point lights are added, making it easy to compare reflected and transmitted light on each test row.

## Output

![](/Docs/ExampleMedia/PhysicallyBasedRendering/DisneyBsdfTransmission/DisneyBsdfThinSurface.png?raw=true)

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

- Implementing thin surface approximation approach in Disney BSDF material model
- Implementing flatness parameter in Disney BSDF material model

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
