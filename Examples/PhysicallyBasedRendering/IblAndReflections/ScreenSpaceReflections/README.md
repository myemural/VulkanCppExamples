# Screen-Space Reflections (SSR)

**Code Name:** ScreenSpaceReflections

## Description

In this example, the PBR material system is first implemented using deferred shading logic, and then the Screen-Space Reflection (SSR) implementation is demonstrated in a separate pass.

## Output

![](/Docs/ExampleMedia/PhysicallyBasedRendering/IblAndReflections/ScreenSpaceReflections.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |
| 0       | SSR disabled                |
| 1       | SSR enabled                 |

## Application Parameters

### Settings

| Parameter / Key                  | Type              | Usage in Code                     | Description                                    | Default Value |
|----------------------------------|-------------------|-----------------------------------|------------------------------------------------|---------------|
| AppSettings.ClearColor           | VkClearColorValue | AppSettings::ClearColor           | Background color of the screen                 |               |
| AppSettings.MouseSensitivity     | float             | AppSettings::MouseSensitivity     | Mouse sensitivity value                        |               |
| AppSettings.CameraSpeed          | float             | AppSettings::CameraSpeed          | Speed of the camera                            |               |
| AppSettings.SsrMaxDistance       | float             | AppSettings::SsrMaxDistance       | Maximum ray distance in view-space for SSS     |               |
| AppSettings.SsrThickness         | float             | AppSettings::SsrThickness         | Depth tolerance for SSS                        |               |
| AppSettings.SsrMaxSteps          | std::int32_t      | AppSettings::SsrMaxSteps          | Ray march max step count for SSS               |               |
| AppSettings.SsrBinarySearchSteps | std::int32_t      | AppSettings::SsrBinarySearchSteps | Step count for intersection refinement for SSS |               |

## Learning Objectives

- Implementing PBR material system with deferred rendering technique
- Implementing Screen-Space Reflection (SSR) with ray marching

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
