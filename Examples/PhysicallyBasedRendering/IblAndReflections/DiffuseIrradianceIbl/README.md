# Diffuse Irradiance Image Based Lighting

**Code Name:** DiffuseIrradianceIbl

## Description

In this example, the cubemap used for the skybox is convoluted to obtain an irradiance map. This irradiance map is then used to calculate the diffuse part of Image Based Lighting (IBL) algorithm.

## Output

![](/Docs/ExampleMedia/PhysicallyBasedRendering/IblAndReflections/DiffuseIrradianceIbl.png?raw=true)

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

- Generating an irradiance map via convolution on a HDR skybox
- Using irradiance map to calculate diffuse part of the IBL

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
