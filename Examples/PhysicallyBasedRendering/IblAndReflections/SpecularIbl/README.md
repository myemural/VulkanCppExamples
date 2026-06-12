# Specular Image Based Lighting

**Code Name:** SpecularIbl

## Description

In this example, the specular part of Image Based Lighting (IBL) is implemented using a pre-calculated BRDF Lookup Table and a prefiltered cubemap image. As a result, complete reflection is achieved in the PBR according to the roughness value of the surface.

## Output

![](/Docs/ExampleMedia/PhysicallyBasedRendering/IblAndReflections/SpecularIbl.png?raw=true)

## Controls

| Input        | Action                     |
|--------------|----------------------------|
| Mouse Move   | Rotataing the orbit camera |
| Mouse Scroll | Zooming the orbit camera   |
| Esc          | Close the window           |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraZoomSpeed  | float             | AppSettings::CameraZoomSpeed  | Zoom speed of the camera         |               |

## Learning Objectives

- Generating pre-calculated BRDF Lookup Table (LUT) via compute shaders
- Generating prefiltered cubemap image via compute shaders
- Using prefiltered cubemap image to calculate specular part of the IBL

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
