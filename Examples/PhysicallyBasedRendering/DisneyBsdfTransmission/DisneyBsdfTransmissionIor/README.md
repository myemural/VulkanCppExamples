# Specular Transmission and IOR in Disney BSDF

**Code Name:** DisneyBsdfTransmissionIor

## Description

In this example, fundamental BSDF material system has been implemented and Disney BSDF’s transmission and IOR parameters have been integrated into this material system. In addition, transmittance color and volumetric absorption logic has been implemented and demonstrated.

## Output

![](/Docs/ExampleMedia/PhysicallyBasedRendering/DisneyBsdfTransmission/DisneyBsdfTransmissionIor.png?raw=true)

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

- Implementing specular transmission in Disney BSDF material model
- Implementing real IOR parameter in Disney BSDF material model

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
