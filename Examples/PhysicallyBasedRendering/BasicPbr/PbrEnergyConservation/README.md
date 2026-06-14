# Principle of Conservation of Energy and Tone Mapping

**Code Name:** PbrEnergyConservation

## Description

In this example, the energy conservation principle, which forms the basis of the PBR material model, is implemented, and the total diffuse and specular light ratios are fixed. Blinn-Phong calculations are used again for the specular light. In addition, Reinhard Tone Mapping is applied to generate the output.

## Output

![](/Docs/ExampleMedia/PhysicallyBasedRendering/BasicPbr/PbrEnergyConservation.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                    | Default Value |
|------------------------------|-------------------|-------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value        |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera            |               |

## Learning Objectives

- Implementing Fresnel calculation and energy conversion principle in PBR
- Using Reinhard Tone Mapping to convert lighting calculations from HDR to LDR

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
