# Clear Coat Layer in Disney BRDF

**Code Name:** DisneyBrdfClearCoat

## Description

In this example, Disney BRDF's clear coat layer is implemented along with IBL modifications. The clear coat and clear coat glossiness parameters of Disney's material model are tested.

## Output

![](/Docs/ExampleMedia/PhysicallyBasedRendering/DisneyPrincipledBrdf/DisneyBrdfClearCoat.png?raw=true)

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

- Adding clear coat layer to the PBR material system
- Implementing Disney BRDF's clear coat and clear coat glossiness parameters with IBL in Vulkan

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
